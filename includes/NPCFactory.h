#pragma once
#include "NPC.h"
#include <istream>

enum class NPCType { Bear, Bittern, Desman };

class NPCFactory {
public:
    static NPCPtr create(NPCType type, const std::string &name, double x, double y);
    static NPCPtr loadFromStream(std::istream &in);
};