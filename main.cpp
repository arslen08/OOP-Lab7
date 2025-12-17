#include "Game.h"
#include "Editor.h"
#include "NPCFactory.h"
#include "Observer.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}

void runEditor() {
    Editor ed;
    ed.addObserver(std::make_shared<ConsoleObserver>());
    ed.addObserver(std::make_shared<FileObserver>("log.txt"));

    while (true) {
        std::cout << "\n=== Dungeon Editor (Lab 6) ===\n";
        std::cout << "NPC Types: Bear, Bittern (Выпь), Desman (Выхухоль)\n";
        std::cout << "Rules:\n";
        std::cout << "  - Bear kills everyone except Bears\n";
        std::cout << "  - Bittern kills no one\n";
        std::cout << "  - Desman kills Bears\n";
        std::cout << "===============================\n";
        std::cout << "1) Add NPC\n";
        std::cout << "2) Save / Load\n";
        std::cout << "3) Print NPC list\n";
        std::cout << "4) Run battle\n";
        std::cout << "0) Back to main menu\n";
        std::cout << "> ";

        int cmd;
        if (!(std::cin >> cmd)) break;
        std::cin.ignore(1000, '\n');

        if (cmd == 0) break;
        else if (cmd == 1) {
            std::cout << "Enter NPC: Type Name x y\n";
            std::cout << "Types: bear, bittern, desman\n> ";
            std::string line;
            std::getline(std::cin, line);
            line = trim(line);

            std::string t, name;
            double x, y;

            std::istringstream iss(line);
            if (!(iss >> t >> name >> x >> y)) {
                std::cout << "Bad input\n";
                continue;
            }

            std::string low = t;
            std::transform(low.begin(), low.end(), low.begin(), ::tolower);

            NPCType nt;
            if (low == "bear") nt = NPCType::Bear;
            else if (low == "bittern") nt = NPCType::Bittern;
            else if (low == "desman") nt = NPCType::Desman;
            else {
                std::cout << "Unknown NPC type. Available: bear, bittern, desman\n";
                continue;
            }

            auto npc = NPCFactory::create(nt, name, x, y);

            if (!ed.addNPC(npc))
                std::cout << "Failed: name must be unique and coordinates 0..500\n";
            else
                std::cout << "NPC added\n";
        }
        else if (cmd == 2) {
            std::cout << "1) Save\n2) Load\n0) Back\n> ";
            int s;
            std::cin >> s;
            std::cin.ignore(1000, '\n');

            if (s == 1) {
                std::string f;
                std::cout << "Filename: ";
                std::getline(std::cin, f);
                try {
                    ed.saveToFile(f);
                    std::cout << "Saved.\n";
                } catch (...) {
                    std::cout << "Save failed\n";
                }
            }
            else if (s == 2) {
                std::string f;
                std::cout << "Filename: ";
                std::getline(std::cin, f);
                try {
                    ed.loadFromFile(f);
                    std::cout << "Loaded.\n";
                } catch (...) {
                    std::cout << "Load failed\n";
                }
            }
        }
        else if (cmd == 3) {
            ed.printAll(std::cout);
        }
        else if (cmd == 4) {
            double r;
            std::cout << "Battle distance: ";
            std::cin >> r;
            std::cin.ignore(1000,'\n');

            ed.runBattle(r);
            std::cout << "Battle finished.\n";
        }
        else {
            std::cout << "Unknown command\n";
        }
    }
}

void runMultiThreadedGame() {
    std::cout << "\n=== Multi-threaded NPC Game (Lab Work #7) ===" << std::endl;
    std::cout << "Based on Lab 6 Variant 19: Bear, Bittern (Выпь), Desman (Выхухоль)" << std::endl;
    std::cout << "Using Desman (Выхухоль) parameters: Move=5, Kill=20" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    try {
        Game game;
        
        std::cout << "\nGame will run for 30 seconds with 3 threads:" << std::endl;
        std::cout << "1. Movement thread (moves NPCs randomly)" << std::endl;
        std::cout << "2. Battle thread (handles fights with dice rolls)" << std::endl;
        std::cout << "3. Main thread (prints map every second)" << std::endl;
        std::cout << "\nPress Enter to start the game..." << std::endl;
        std::cin.ignore(1000, '\n');
        
        game.start();
        game.waitForFinish();
        
        std::cout << "\nGame finished!" << std::endl;
        std::cout << "Press Enter to continue..." << std::endl;
        std::cin.get();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << "Press Enter to continue..." << std::endl;
        std::cin.get();
    }
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    while (true) {
        std::cout << "\n=== Main Menu ===" << std::endl;
        std::cout << "1) Run Multi-threaded Game (Lab Work #7)" << std::endl;
        std::cout << "2) Run Editor (Lab Work #6 - Variant 19)" << std::endl;
        std::cout << "0) Exit" << std::endl;
        std::cout << "> ";
        
        int choice;
        if (!(std::cin >> choice)) break;
        std::cin.ignore(1000, '\n');
        
        if (choice == 0) {
            break;
        } else if (choice == 1) {
            runMultiThreadedGame();
        } else if (choice == 2) {
            runEditor();
        } else {
            std::cout << "Invalid choice!" << std::endl;
        }
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}