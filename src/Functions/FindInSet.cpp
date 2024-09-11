#include <string>
#include <Columns/ColumnString.h>
#include <DataTypes/DataTypeString.h>
#include <Functions/FunctionFactory.h>
#include <Functions/FunctionHelpers.h>
#include <Functions/IFunctionMySql.h>
#include <Functions/FunctionsConversion.h>

namespace DB
{
namespace ErrorCodes
{
    extern const int ILLEGAL_TYPE_OF_ARGUMENT;
    extern const int NUMBER_OF_ARGUMENTS_DOESNT_MATCH;
    extern const int ILLEGAL_COLUMN;
}

class FunctionFindInSet : public IFunction
{
public:
    static constexpr auto name = "find_in_set";
    static FunctionPtr create(ContextPtr context)
    {
        if (context && context->getSettingsRef().enable_implicit_arg_type_convert)
            return std::make_shared<IFunctionMySql>(std::make_unique<FunctionFindInSet>());
        return std::make_shared<FunctionFindInSet>();
    }

    ArgType getArgumentsType() const override { return ArgType::STRINGS; }

    String getName() const override { return name; }

    size_t getNumberOfArguments() const override { return 2; }

    bool useDefaultImplementationForConstants() const override { return true; }

    DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override
    {
        if (arguments.size() != 2)
            throw Exception(
                "Number of arguments for function " + getName() + " doesn't match: passed " + toString(arguments.size())
                    + ", should be 2.",
                ErrorCodes::NUMBER_OF_ARGUMENTS_DOESNT_MATCH);

        return std::make_shared<DataTypeUInt64>();
    }

    ColumnPtr executeImpl(const ColumnsWithTypeAndName & arguments, const DataTypePtr &/*result_type*/, size_t cnt) const override
    {
        ColumnPtr needle_column = arguments[0].column;
        if (!isString(arguments[0].type))
        {
            ColumnsWithTypeAndName args_src {arguments[0]};
            needle_column = ConvertImplGenericToString<ColumnString>::execute(args_src, std::make_shared<DataTypeString>(), cnt);
        }

        ColumnPtr haystack_column = arguments[1].column;
        if (!isString(arguments[1].type))
        {
            ColumnsWithTypeAndName args_src {arguments[1]};
            haystack_column = ConvertImplGenericToString<ColumnString>::execute(args_src, std::make_shared<DataTypeString>(), cnt);
        }

        auto col_res = ColumnUInt64::create();
        ColumnUInt64::Container & vec_res = col_res->getData();
        vec_res.resize_fill(needle_column->size());

        for (size_t j = 0; j < vec_res.size(); ++j)
        {
            std::string needle = needle_column->getDataAt(j).toString();
            std::string haystack = haystack_column->getDataAt(j).toString();
            size_t last = 0;
            size_t next = 0;
            size_t pos = 0;
            while ((next = haystack.find(',', last)) != std::string::npos)
            {
                ++pos;
                if (haystack.substr(last, next-last) == needle)
                {
                    vec_res[j] = pos;
                    break;
                }

                last = next + 1;
            }

            if (next == std::string::npos && haystack.substr(last) == needle)
                vec_res[j] = ++pos;
        }

        return col_res;
    }
};

REGISTER_FUNCTION(FindInSet)
{
    factory.registerFunction<FunctionFindInSet>(FunctionFactory::CaseInsensitive);
}

}
