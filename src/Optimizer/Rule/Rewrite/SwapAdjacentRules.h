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

#pragma once
#include <Optimizer/Rule/Patterns.h>
#include <Optimizer/Rule/Rule.h>

namespace DB
{

class SwapAdjacentWindows : public Rule
{
public:
    RuleType getType() const override { return RuleType::SWAP_WINDOWS; }
    String getName() const override { return "SWAP_ADJACENT_WINDOWS"; }
    bool isEnabled(ContextPtr context) const override {return context->getSettingsRef().enable_windows_reorder; }
    ConstRefPatternPtr getPattern() const override { static auto pattern = Patterns::window().result(); return pattern; }

    TransformResult transformImpl(PlanNodePtr node, const Captures & captures, RuleContext & context) override;
};

}
