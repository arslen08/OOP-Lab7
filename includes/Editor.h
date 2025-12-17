#pragma once
#include "NPC.h"
#include "Observer.h"
#include <vector>
#include <string>

class Editor {
    std::vector<NPCPtr> npcs_;
    std::vector<ObsPtr> observers_;
public:
    void addObserver(ObsPtr obs);
    void removeObserver(ObsPtr obs);

    bool addNPC(NPCPtr npc);

    void saveToFile(const std::string &filename) const;
    void loadFromFile(const std::string &filename);

    void printAll(std::ostream &os) const;

    void runBattle(double distance);

    const std::vector<NPCPtr>& npcs() const { return npcs_; }
};