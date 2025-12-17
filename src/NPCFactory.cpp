#include "NPCFactory.h"
#include "NPCTypes.h"
#include <algorithm>
#include <stdexcept>

NPCPtr NPCFactory::create(NPCType type, const std::string &name, double x, double y) {
    switch (type) {
        case NPCType::Bear:   return std::make_shared<Bear>(name, x, y);
        case NPCType::Bittern:return std::make_shared<Bittern>(name, x, y);
        case NPCType::Desman: return std::make_shared<Desman>(name, x, y);
    }
    throw std::runtime_error("Unknown NPCType");
}

NPCPtr NPCFactory::loadFromStream(std::istream &in) {
    std::string type, name;
    double x, y;

    if (!(in >> type)) return nullptr;
    if (!(in >> name >> x >> y))
        throw std::runtime_error("Bad NPC format");

    std::string low = type;
    std::transform(low.begin(), low.end(), low.begin(), ::tolower);

    if (low == "bear")    return create(NPCType::Bear, name, x, y);
    if (low == "bittern") return create(NPCType::Bittern, name, x, y);
    if (low == "desman")  return create(NPCType::Desman, name, x, y);

    throw std::runtime_error("Unknown NPC type: " + type);
}