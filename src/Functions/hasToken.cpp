#include <Functions/FunctionFactory.h>
#include <Functions/FunctionsStringSearch.h>
#include <Functions/HasTokenImpl.h>

#include <Common/Volnitsky.h>

namespace DB
{

struct NameHasToken
{
    static constexpr auto name = "hasToken";
};

struct NameHasTokenOrNull
{
    static constexpr auto name = "hasTokenOrNull";
};

using FunctionHasToken
    = FunctionsStringSearch<HasTokenImpl<NameHasToken, Volnitsky, false>>;
using FunctionHasTokenOrNull
    = FunctionsStringSearch<HasTokenImpl<NameHasTokenOrNull, Volnitsky, false>, ExecutionErrorPolicy::Null>;

REGISTER_FUNCTION(HasToken)
{
    factory.registerFunction<FunctionHasToken>(FunctionFactory::CaseSensitive);

    factory.registerFunction<FunctionHasTokenOrNull>(FunctionFactory::CaseSensitive);
}

}
