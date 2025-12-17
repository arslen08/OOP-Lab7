#pragma once
#include "FightVisitor.h"

class FightRules : public FightVisitor {
public:
    bool visit(Bear &attacker, NPC &defender) override;
    bool visit(Bittern &attacker, NPC &defender) override;
    bool visit(Desman &attacker, NPC &defender) override;
};