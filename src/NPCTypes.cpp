#include "NPCTypes.h"
#include "FightVisitor.h"

bool Bear::accept(FightVisitor &v, NPC &defender) {
    return v.visit(*this, defender);
}
bool Bittern::accept(FightVisitor &v, NPC &defender) {
    return v.visit(*this, defender);
}
bool Desman::accept(FightVisitor &v, NPC &defender) {
    return v.visit(*this, defender);
}