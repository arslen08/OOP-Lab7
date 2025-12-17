#include "Editor.h"
#include "NPCFactory.h"
#include "FightRules.h"
#include <fstream>
#include <algorithm>
#include <cmath>

void Editor::addObserver(ObsPtr obs) {
    observers_.push_back(obs);
}
void Editor::removeObserver(ObsPtr obs) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), obs), observers_.end());
}

bool Editor::addNPC(NPCPtr npc) {
    if (!npc) return false;

    if (npc->x() < 0 || npc->x() > 500 || npc->y() < 0 || npc->y() > 500)
        return false;

    for (auto &e : npcs_) {
        if (e->name() == npc->name())
            return false;
    }

    npcs_.push_back(npc);
    return true;
}

void Editor::saveToFile(const std::string &filename) const {
    std::ofstream out(filename);
    for (auto &n : npcs_)
        n->serialize(out);
}

void Editor::loadFromFile(const std::string &filename) {
    std::ifstream in(filename);
    npcs_.clear();

    while (true) {
        auto npc = NPCFactory::loadFromStream(in);
        if (!npc) break;

        if (!addNPC(npc))
            throw std::runtime_error("Invalid or duplicate NPC in file");
    }
}

void Editor::printAll(std::ostream &os) const {
    os << "NPC list (" << npcs_.size() << "):\n";
    for (auto &n : npcs_) {
        os << n->type() << " " << n->name()
           << " " << n->x() << " " << n->y() << "\n";
    }
}

static double dist2(const NPCPtr &a, const NPCPtr &b) {
    double dx = a->x() - b->x();
    double dy = a->y() - b->y();
    return dx*dx + dy*dy;
}

void Editor::runBattle(double distance) {
    FightRules rules;
    double d2 = distance * distance;

    bool killed = true;

    while (killed) {
        killed = false;
        std::vector<std::string> dead;

        for (size_t i = 0; i < npcs_.size(); ++i) {
            for (size_t j = i+1; j < npcs_.size(); ++j) {
                auto &A = npcs_[i];
                auto &B = npcs_[j];

                if (dist2(A,B) > d2) continue;

                bool A_kills_B = A->accept(rules, *B);
                bool B_kills_A = B->accept(rules, *A);

                if (A_kills_B) {
                    dead.push_back(B->name());
                    for (auto &o : observers_) o->onKill(A->name(), B->name());
                }
                if (B_kills_A) {
                    dead.push_back(A->name());
                    for (auto &o : observers_) o->onKill(B->name(), A->name());
                }
            }
        }

        if (!dead.empty()) {
            killed = true;

            std::sort(dead.begin(), dead.end());
            dead.erase(std::unique(dead.begin(), dead.end()), dead.end());

            npcs_.erase(std::remove_if(npcs_.begin(), npcs_.end(),
                [&](auto &p){ return std::binary_search(dead.begin(), dead.end(), p->name()); }),
                npcs_.end());
        }
    }
}