#pragma once

#include <Interpreters/IInterpreter.h>
#include <Parsers/IAST_fwd.h>
#include "Parsers/ASTShowTablesQuery.h"


namespace DB
{

class Context;
class ASTShowTablesQuery;

/** Return a list of tables or databases meets specified conditions.
  * Interprets a query through replacing it to SELECT query from system.tables or system.databases.
  */
class InterpreterShowTablesQuery : public IInterpreter, WithMutableContext
{
public:
    InterpreterShowTablesQuery(const ASTPtr & query_ptr_, ContextMutablePtr context_);

    BlockIO execute() override;

    /// Ignore quota and limits here because execute() produces a SELECT query which checks quotas/limits by itself.
    bool ignoreQuota() const override { return true; }
    bool ignoreLimits() const override { return true; }

private:
    ASTPtr query_ptr;

    String getRewrittenQuery();
    String getRewrittenQueryImpl();
    String getRewrittenQueryForExternalCatalogImpl();
};


}
