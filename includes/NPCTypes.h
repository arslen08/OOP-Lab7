#pragma once
#include "NPC.h"

class Bear : public NPC {
public:
    Bear(const std::string &n, double x, double y) : NPC(n, x, y) {}
    std::string type() const override { return "Bear"; }
    bool accept(FightVisitor &v, NPC &defender) override;
    
    std::shared_ptr<NPC> cloneWithPosition(double x, double y) const override {
        return std::make_shared<Bear>(name_, x, y);
    }
};

class Bittern : public NPC {
public:
    Bittern(const std::string &n, double x, double y) : NPC(n, x, y) {}
    std::string type() const override { return "Bittern"; }
    bool accept(FightVisitor &v, NPC &defender) override;
    
    std::shared_ptr<NPC> cloneWithPosition(double x, double y) const override {
        return std::make_shared<Bittern>(name_, x, y);
    }
};

class Desman : public NPC {
public:
    Desman(const std::string &n, double x, double y) : NPC(n, x, y) {}
    std::string type() const override { return "Desman"; }
    bool accept(FightVisitor &v, NPC &defender) override;
    
    std::shared_ptr<NPC> cloneWithPosition(double x, double y) const override {
        return std::make_shared<Desman>(name_, x, y);
    }
};