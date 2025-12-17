#pragma once
#include <string>
#include <memory>
#include <ostream>

class FightVisitor;

class NPC {
protected:
    std::string name_;
    double x_, y_;
public:
    NPC(const std::string &name, double x, double y)
        : name_(name), x_(x), y_(y) {}

    virtual ~NPC() = default;

    const std::string& name() const { return name_; }
    double x() const { return x_; }
    double y() const { return y_; }

    virtual std::string type() const = 0;

    virtual bool accept(FightVisitor &visitor, NPC &defender) = 0;

    virtual void serialize(std::ostream &os) const {
        os << type() << " " << name_ << " " << x_ << " " << y_ << "\n";
    }

    virtual std::shared_ptr<NPC> cloneWithPosition(double x, double y) const = 0;
};

using NPCPtr = std::shared_ptr<NPC>;