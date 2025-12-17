#include "FightRules.h"
#include "NPCTypes.h"

bool FightRules::visit(Bear &attacker, NPC &defender) {
    (void)attacker;
    return defender.type() != "Bear";
}

bool FightRules::visit(Bittern &attacker, NPC &defender) {
    (void)attacker;
    (void)defender;
    return false;
}

bool FightRules::visit(Desman &attacker, NPC &defender) {
    (void)attacker;
    return defender.type() == "Bear";
}