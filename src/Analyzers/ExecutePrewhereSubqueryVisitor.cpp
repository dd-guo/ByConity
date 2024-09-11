/*
 * Copyright (2022) Bytedance Ltd. and/or its affiliates
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Analyzers/ExecutePrewhereSubqueryVisitor.h>

#include <Analyzers/function_utils.h>
#include <DataStreams/BlockIO.h>
#include <DataTypes/DataTypeNullable.h>
#include <DataTypes/DataTypeTuple.h>
#include <Interpreters/InterpreterFactory.h>
#include <Parsers/ASTExpressionList.h>
#include <Parsers/ASTFunction.h>
#include <Parsers/ASTLiteral.h>
#include <Parsers/ASTSubquery.h>

#include <memory>

namespace DB
{

namespace ErrorCodes
{
    extern const int INCORRECT_RESULT_OF_SCALAR_SUBQUERY;
    extern const int TOO_MANY_ROWS;
}

static ASTPtr addTypeConversion(std::unique_ptr<ASTLiteral> && ast, const String & type_name)
{
    auto func = std::make_shared<ASTFunction>();
    ASTPtr res = func;
    func->alias = ast->alias;
    func->prefer_alias_to_column_name = ast->prefer_alias_to_column_name;
    ast->alias.clear();
    func->name = "CAST";
    auto exp_list = std::make_shared<ASTExpressionList>();
    func->arguments = exp_list;
    func->children.push_back(func->arguments);
    exp_list->children.emplace_back(ast.release());
    exp_list->children.emplace_back(std::make_shared<ASTLiteral>(type_name));
    return res;
}


void ExecutePrewhereSubquery::visit(ASTSubquery & subquery, ASTPtr & ast) const
{
    rewriteSubqueryToScalarLiteral(subquery, ast);
}

void ExecutePrewhereSubquery::visit(ASTFunction & function, ASTPtr & ast) const
{
    auto type = getFunctionType(function, context);
    if (type == FunctionType::IN_SUBQUERY)
    {
        auto & subquery = function.arguments->children[1];
        rewriteSubqueryToSet(subquery->as<ASTSubquery &>(), subquery);
    }
    else if (type == FunctionType::EXISTS_SUBQUERY)
    {
        bool exists;
        if (function.name == "not")
        {
            auto & subquery = function.arguments->children[0]->as<ASTFunction &>().arguments->children[0];
            exists = !rewriteSubqueryToSet(subquery->as<ASTSubquery &>(), subquery);
        }
        else
        {
            auto & subquery = function.arguments->children[0];
            exists = rewriteSubqueryToSet(subquery->as<ASTSubquery &>(), subquery);
        }

        ast = exists ? std::make_shared<ASTLiteral>(true) : std::make_shared<ASTLiteral>(false);
    }
}

void ExecutePrewhereSubquery::rewriteSubqueryToScalarLiteral(ASTSubquery & subquery, ASTPtr & ast) const
{
    ContextMutablePtr subquery_context = Context::createCopy(context);
    Settings subquery_settings = context->getSettings();
    subquery_settings.max_result_rows = 1;
    subquery_settings.extremes = false;
    // internal SQL doesn't work well in optimizer mode, mainly due to PlanSegmentExecutor
    subquery_settings.enable_optimizer = false;
    subquery_context->setSettings(subquery_settings);

    ASTPtr subquery_select = subquery.children.at(0);
    auto interpreter = InterpreterFactory::get(subquery_select, subquery_context,
                                               SelectQueryOptions(QueryProcessingStage::Complete).setInternal(true));
    auto stream = interpreter->execute().getInputStream();

    Block block;
    try
    {
        do
        {
            block = stream->read();
        } while (block && block.rows() == 0);

        if (!block)
        {
            auto types = stream->getHeader().getDataTypes();
            if (types.size() != 1)
                types = {std::make_shared<DataTypeTuple>(types)};

            auto & type = types[0];
            if (!type->isNullable())
            {
                if (!type->canBeInsideNullable())
                    throw Exception(
                        ErrorCodes::INCORRECT_RESULT_OF_SCALAR_SUBQUERY,
                        "Scalar subquery returned empty result of type {} which cannot be Nullable",
                        type->getName());

                type = makeNullable(type);
            }

            /// Interpret subquery with empty result as Null literal
            auto ast_null = std::make_unique<ASTLiteral>(Null());
            auto ast_new = addTypeConversion(std::move(ast_null), type->getName());
            ast_new->setAlias(ast->tryGetAlias());
            ast = std::move(ast_new);
            return;
        }

        if (block.rows() > 1)
            throw Exception(
                "Scalar subquery expected 1 row, got " + std::to_string(block.rows()) + " rows",
                ErrorCodes::INCORRECT_RESULT_OF_SCALAR_SUBQUERY);
        while (Block rest = stream->read())
            if (rest.rows() > 0)
                throw Exception(
                    "Scalar subquery returned more than one non-empty block", ErrorCodes::INCORRECT_RESULT_OF_SCALAR_SUBQUERY);
    }
    catch (const Exception & e)
    {
        if (e.code() == ErrorCodes::TOO_MANY_ROWS)
            throw Exception("Scalar subquery returned too many rows", ErrorCodes::INCORRECT_RESULT_OF_SCALAR_SUBQUERY);
        else
            throw;
    }

    size_t columns = block.columns();
    if (columns == 1)
    {
        auto lit = std::make_unique<ASTLiteral>((*block.safeGetByPosition(0).column)[0]);
        lit->alias = subquery.alias;
        lit->prefer_alias_to_column_name = subquery.prefer_alias_to_column_name;
        ast = addTypeConversion(std::move(lit), block.safeGetByPosition(0).type->getName());
    }
    else
    {
        auto tuple = std::make_shared<ASTFunction>();
        tuple->alias = subquery.alias;
        ast = tuple;
        tuple->name = "tuple";
        auto exp_list = std::make_shared<ASTExpressionList>();
        tuple->arguments = exp_list;
        tuple->children.push_back(tuple->arguments);

        exp_list->children.resize(columns);
        for (size_t i = 0; i < columns; ++i)
        {
            exp_list->children[i] = addTypeConversion(
                std::make_unique<ASTLiteral>((*block.safeGetByPosition(i).column)[0]), block.safeGetByPosition(i).type->getName());
        }
    }
}

bool ExecutePrewhereSubquery::rewriteSubqueryToSet(ASTSubquery & subquery, ASTPtr & ast) const
{
    ContextMutablePtr subquery_context = Context::createCopy(context);
    Settings subquery_settings = context->getSettings();
    subquery_settings.extremes = false;
    // internal SQL doesn't work well in optimizer mode, mainly due to PlanSegmentExecutor
    subquery_settings.enable_optimizer = false;
    subquery_context->setSettings(subquery_settings);

    ASTPtr subquery_select = subquery.children.at(0);
    auto interpreter = InterpreterFactory::get(subquery_select, subquery_context,
                                               SelectQueryOptions(QueryProcessingStage::Complete).setInternal(true));
    auto stream = interpreter->execute().getInputStream();
    size_t columns = stream->getHeader().columns();

    auto array = std::make_shared<ASTFunction>();
    array->name = "array";
    array->alias = subquery.alias;
    array->arguments = std::make_shared<ASTExpressionList>();
    array->children.push_back(array->arguments);

    Block block;
    while (true)
    {
        block = stream->read();
        if (!block || !block.rows())
            break;
        if (columns == 1)
        {
            for (size_t position = 0; position < block.rows(); position++)
            {
                auto literal = std::make_unique<ASTLiteral>((*block.safeGetByPosition(0).column)[position]);
                literal->alias = subquery.alias;
                literal->prefer_alias_to_column_name = subquery.prefer_alias_to_column_name;
                auto cast = addTypeConversion(std::move(literal), block.safeGetByPosition(0).type->getName());
                array->arguments->children.emplace_back(cast);
            }
        }
        else
        {
            for (size_t position = 0; position < block.rows(); position++)
            {
                auto tuple = std::make_shared<ASTFunction>();
                tuple->alias = subquery.alias;
                ast = tuple;
                tuple->name = "tuple";
                auto exp_list = std::make_shared<ASTExpressionList>();
                tuple->arguments = exp_list;
                tuple->children.push_back(tuple->arguments);

                exp_list->children.resize(columns);
                for (size_t i = 0; i < columns; ++i)
                {
                    exp_list->children[i] = addTypeConversion(
                        std::make_unique<ASTLiteral>((*block.safeGetByPosition(i).column)[position]), block.safeGetByPosition(i).type->getName());
                }
                array->arguments->children.emplace_back(tuple);
            }
        }
    }

    if (array->arguments->children.empty())
    {
        /// Interpret subquery with empty result as Null literal
        if (columns == 1)
        {
            auto ast_new = std::make_unique<ASTLiteral>(Null());
            ast_new->setAlias(ast->tryGetAlias());
            ast = std::move(ast_new);
            return false;
        }
        else
        {
            auto tuple = std::make_shared<ASTFunction>();
            tuple->alias = subquery.alias;
            ast = tuple;
            tuple->name = "tuple";
            auto exp_list = std::make_shared<ASTExpressionList>();
            tuple->arguments = exp_list;
            tuple->children.push_back(tuple->arguments);

            exp_list->children.resize(columns);
            for (size_t i = 0; i < columns; ++i)
                exp_list->children[i] = std::make_unique<ASTLiteral>(Null());
            array->arguments->children.emplace_back(tuple);

            ast = std::move(array);
            return false;
        }
    }
    else
    {
        ast = std::move(array);
        return true;
    }
}

}
