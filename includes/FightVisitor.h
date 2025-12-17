#pragma once
class Bear;
class Bittern;
class Desman;
class NPC;

class FightVisitor {
public:
    virtual bool visit(Bear &attacker, NPC &defender) = 0;
    virtual bool visit(Bittern &attacker, NPC &defender) = 0;
    virtual bool visit(Desman &attacker, NPC &defender) = 0;
    virtual ~FightVisitor() = default;
};