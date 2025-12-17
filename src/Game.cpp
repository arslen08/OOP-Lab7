#include "Game.h"
#include "Observer.h"
#include "FightRules.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>

using namespace std::chrono_literals;

Game::Game() 
    : gen_(rd_()), 
      pos_dist_(0.0, MAP_WIDTH),
      type_dist_(0, 2) { 
    
    editor_.addObserver(std::make_shared<ConsoleObserver>());
    editor_.addObserver(std::make_shared<FileObserver>("game_log.txt"));
}

Game::~Game() {
    stop();
}

void Game::generateInitialNPCs() {
    std::unique_lock<std::shared_mutex> lock(npc_mutex_);
    
    std::vector<std::string> bear_names = {"Ursa", "Grizzly", "Brown", "Black", "Polar", "Honey", "Teddy", "Growler", "Fuzzy", "Bruno"};
    std::vector<std::string> bittern_names = {"Wader", "Heron", "Egret", "Stork", "Crane", "Ibis", "Spoonbill", "Flamingo", "Pelican", "Grebe"};
    std::vector<std::string> desman_names = {"Mole", "Shrew", "Vole", "Muskrat", "Beaver", "Otter", "Mink", "Weasel", "Ferret", "Badger"};
    
    int count = 0;
    while (count < 50) {
        NPCType type = static_cast<NPCType>(type_dist_(gen_));
        std::string name;
        double x = pos_dist_(gen_);
        double y = pos_dist_(gen_);
        
        switch (type) {
            case NPCType::Bear:
                name = bear_names[count % bear_names.size()] + std::to_string(count);
                break;
            case NPCType::Bittern:
                name = bittern_names[count % bittern_names.size()] + std::to_string(count);
                break;
            case NPCType::Desman:
                name = desman_names[count % desman_names.size()] + std::to_string(count);
                break;
        }
        
        auto npc = NPCFactory::create(type, name, x, y);
        if (editor_.addNPC(npc)) {
            count++;
        }
    }
    
    alive_count_ = 50;
    
    {
        std::lock_guard<std::mutex> cout_lock(cout_mutex_);
        std::cout << "[GAME] Generated 50 NPCs:" << std::endl;
        
        int bears = 0, bitterns = 0, desmans = 0;
        for (const auto& npc : editor_.npcs()) {
            if (npc->type() == "Bear") bears++;
            else if (npc->type() == "Bittern") bitterns++;
            else if (npc->type() == "Desman") desmans++;
        }
        
        std::cout << "[GAME] Bears: " << bears << ", Bitterns: " << bitterns << ", Desmans: " << desmans << std::endl;
    }
}

double Game::calculateDistance(double x1, double y1, double x2, double y2) const {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return std::sqrt(dx*dx + dy*dy);
}

void Game::movementWorker() {
    std::uniform_int_distribution<> sleep_dist(50, 200);
    std::uniform_real_distribution<> move_dist(-MOVE_DISTANCE, MOVE_DISTANCE);
    
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(gen_)));
        
        std::shared_lock<std::shared_mutex> read_lock(npc_mutex_);
        auto npcs = editor_.npcs();
        
        if (npcs.empty()) {
            read_lock.unlock();
            continue;
        }
        
        std::uniform_int_distribution<> idx_dist(0, static_cast<int>(npcs.size()) - 1);
        int idx = idx_dist(gen_);
        auto npc_to_move = npcs[idx];
        read_lock.unlock();
        
        double new_x = npc_to_move->x() + move_dist(gen_);
        double new_y = npc_to_move->y() + move_dist(gen_);
        
        new_x = std::clamp(new_x, 0.0, MAP_WIDTH);
        new_y = std::clamp(new_y, 0.0, MAP_HEIGHT);
        
        auto new_npc = npc_to_move->cloneWithPosition(new_x, new_y);
        
        std::unique_lock<std::shared_mutex> write_lock(npc_mutex_);
        auto& all_npcs = const_cast<std::vector<NPCPtr>&>(editor_.npcs());
        for (auto& existing_npc : all_npcs) {
            if (existing_npc->name() == npc_to_move->name()) {
                existing_npc = new_npc;
                break;
            }
        }
    }
}

void Game::battleWorker() {
    std::uniform_int_distribution<> sleep_dist(100, 300);
    std::uniform_int_distribution<> dice_dist(1, 6);
    FightRules rules;
    
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(gen_)));
        
        std::shared_lock<std::shared_mutex> read_lock(npc_mutex_);
        auto npcs = editor_.npcs();
        read_lock.unlock();
        
        if (npcs.size() < 2) continue;
        
        std::vector<std::string> killed_npcs;
        std::vector<std::pair<std::string, std::string>> kill_events;
        
        for (size_t i = 0; i < npcs.size(); ++i) {
            for (size_t j = i + 1; j < npcs.size(); ++j) {
                if (std::find(killed_npcs.begin(), killed_npcs.end(), npcs[i]->name()) != killed_npcs.end() ||
                    std::find(killed_npcs.begin(), killed_npcs.end(), npcs[j]->name()) != killed_npcs.end()) {
                    continue;
                }
                
                double dist = calculateDistance(
                    npcs[i]->x(), npcs[i]->y(),
                    npcs[j]->x(), npcs[j]->y()
                );
                
                if (dist <= KILL_DISTANCE) {
                    int attack_power_i = dice_dist(gen_);
                    int defense_power_j = dice_dist(gen_);
                    
                    int attack_power_j = dice_dist(gen_);
                    int defense_power_i = dice_dist(gen_);
                    
                    bool i_can_kill_j = false;
                    bool j_can_kill_i = false;
                    
                    if (attack_power_i > defense_power_j) {
                        i_can_kill_j = npcs[i]->accept(rules, *npcs[j]);
                    }
                    
                    if (attack_power_j > defense_power_i) {
                        j_can_kill_i = npcs[j]->accept(rules, *npcs[i]);
                    }
                    
                    if (i_can_kill_j && !j_can_kill_i) {
                        killed_npcs.push_back(npcs[j]->name());
                        kill_events.emplace_back(npcs[i]->name(), npcs[j]->name());
                    } else if (j_can_kill_i && !i_can_kill_j) {
                        killed_npcs.push_back(npcs[i]->name());
                        kill_events.emplace_back(npcs[j]->name(), npcs[i]->name());
                    } else if (i_can_kill_j && j_can_kill_i) {
                        killed_npcs.push_back(npcs[i]->name());
                        killed_npcs.push_back(npcs[j]->name());
                        kill_events.emplace_back(npcs[i]->name(), npcs[j]->name());
                        kill_events.emplace_back(npcs[j]->name(), npcs[i]->name());
                    }
                }
            }
        }
        
        if (!killed_npcs.empty()) {
            std::unique_lock<std::shared_mutex> write_lock(npc_mutex_);
            
            auto& all_npcs = const_cast<std::vector<NPCPtr>&>(editor_.npcs());
            auto new_end = std::remove_if(all_npcs.begin(), all_npcs.end(),
                [&](const NPCPtr& npc) {
                    return std::find(killed_npcs.begin(), killed_npcs.end(), npc->name()) != killed_npcs.end();
                });
            
            if (new_end != all_npcs.end()) {
                all_npcs.erase(new_end, all_npcs.end());
                
                for (const auto& kill : kill_events) {
                    std::string killer_type = "Unknown";
                    for (const auto& npc : all_npcs) {
                        if (npc->name() == kill.first) {
                            killer_type = npc->type();
                            break;
                        }
                    }
                    
                    std::lock_guard<std::mutex> cout_lock(cout_mutex_);
                    std::cout << "[BATTLE] " << kill.first << " (" << killer_type 
                              << ") killed " << kill.second << std::endl;
                }
                
                alive_count_ = static_cast<int>(all_npcs.size());
                
                std::ofstream log_file("game_log.txt", std::ios::app);
                if (log_file) {
                    for (const auto& kill : kill_events) {
                        log_file << "[BATTLE] " << kill.first << " killed " << kill.second << std::endl;
                    }
                }
            }
        }
    }
}

void Game::mainWorker() {
    auto start_time = std::chrono::steady_clock::now();
    auto duration = 30s;
    
    int map_updates = 0;
    
    while (running_) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = current_time - start_time;
        
        if (elapsed >= duration) {
            running_ = false;
            break;
        }
        
        std::this_thread::sleep_for(1s);
        
        std::shared_lock<std::shared_mutex> read_lock(npc_mutex_);
        auto npcs = editor_.npcs();
        read_lock.unlock();
        
        {
            std::lock_guard<std::mutex> cout_lock(cout_mutex_);
            
            map_updates++;
            std::cout << "\n=== Game Map Update #" << map_updates 
                      << " (Alive: " << alive_count_ 
                      << ", Time: " << std::chrono::duration_cast<std::chrono::seconds>(elapsed).count()
                      << "s) ===" << std::endl;
            
            const int grid_size = 10;
            std::vector<std::vector<char>> grid(grid_size, std::vector<char>(grid_size, '.'));
            std::vector<std::vector<int>> count_grid(grid_size, std::vector<int>(grid_size, 0));
            
            for (const auto& npc : npcs) {
                int grid_x = static_cast<int>((npc->x() / MAP_WIDTH) * grid_size);
                int grid_y = static_cast<int>((npc->y() / MAP_HEIGHT) * grid_size);
                
                grid_x = std::clamp(grid_x, 0, grid_size - 1);
                grid_y = std::clamp(grid_y, 0, grid_size - 1);
                
                count_grid[grid_y][grid_x]++;
                
                char symbol = '?';
                std::string type = npc->type();
                if (type == "Bear") symbol = 'B';
                else if (type == "Bittern") symbol = 'I';
                else if (type == "Desman") symbol = 'D';
                
                if (grid[grid_y][grid_x] == '.') {
                    grid[grid_y][grid_x] = symbol;
                } 
                else if (grid[grid_y][grid_x] != symbol && grid[grid_y][grid_x] != 'X') {
                    grid[grid_y][grid_x] = 'X';
                }
            }
            
            int bears = 0, bitterns = 0, desmans = 0;
            for (const auto& npc : npcs) {
                std::string type = npc->type();
                if (type == "Bear") bears++;
                else if (type == "Bittern") bitterns++;
                else if (type == "Desman") desmans++;
            }
            
            std::cout << "Stats: B=" << bears << " I=" << bitterns << " D=" << desmans << std::endl;
            
            std::cout << "    ";
            for (int x = 0; x < grid_size; ++x) {
                std::cout << std::setw(2) << x << " ";
            }
            std::cout << std::endl;
            
            for (int y = 0; y < grid_size; ++y) {
                std::cout << std::setw(2) << y << "  ";
                for (int x = 0; x < grid_size; ++x) {
                    std::cout << grid[y][x];
                    if (count_grid[y][x] > 1) {
                        std::cout << std::to_string(count_grid[y][x]);
                    } else {
                        std::cout << " ";
                    }
                    std::cout << " ";
                }
                std::cout << std::endl;
            }
            
            std::cout << "Legend: B=Bear, I=Bittern, D=Desman, X=Mixed, .=Empty, Number=Count" << std::endl;
        }
    }
    
    std::shared_lock<std::shared_mutex> read_lock(npc_mutex_);
    auto npcs = editor_.npcs();
    read_lock.unlock();
    
    {
        std::lock_guard<std::mutex> cout_lock(cout_mutex_);
        
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "=== GAME OVER ===" << std::endl;
        std::cout << "Total time: 30 seconds" << std::endl;
        std::cout << "Survivors (" << alive_count_ << "):" << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        if (npcs.empty()) {
            std::cout << "No survivors! All NPCs were killed." << std::endl;
        } else {
            std::vector<NPCPtr> bears, bitterns, desmans;
            for (const auto& npc : npcs) {
                std::string type = npc->type();
                if (type == "Bear") bears.push_back(npc);
                else if (type == "Bittern") bitterns.push_back(npc);
                else if (type == "Desman") desmans.push_back(npc);
            }
            
            if (!bears.empty()) {
                std::cout << "\nBears (" << bears.size() << "):" << std::endl;
                for (const auto& bear : bears) {
                    std::cout << "  " << std::left << std::setw(15) << bear->name()
                              << " at (" << std::setw(6) << std::fixed << std::setprecision(1) << bear->x()
                              << ", " << std::setw(6) << bear->y() << ")" << std::endl;
                }
            }
            
            if (!bitterns.empty()) {
                std::cout << "\nBitterns (" << bitterns.size() << "):" << std::endl;
                for (const auto& bittern : bitterns) {
                    std::cout << "  " << std::left << std::setw(15) << bittern->name()
                              << " at (" << std::setw(6) << std::fixed << std::setprecision(1) << bittern->x()
                              << ", " << std::setw(6) << bittern->y() << ")" << std::endl;
                }
            }
            
            if (!desmans.empty()) {
                std::cout << "\nDesmans (" << desmans.size() << "):" << std::endl;
                for (const auto& desman : desmans) {
                    std::cout << "  " << std::left << std::setw(15) << desman->name()
                              << " at (" << std::setw(6) << std::fixed << std::setprecision(1) << desman->x()
                              << ", " << std::setw(6) << desman->y() << ")" << std::endl;
                }
            }
        }
        
        std::cout << std::string(50, '=') << std::endl;
        
        std::cout << "\nFinal Statistics:" << std::endl;
        std::cout << "Initial NPCs: 50" << std::endl;
        std::cout << "Survivors: " << alive_count_ << std::endl;
        std::cout << "Killed: " << (50 - alive_count_) << std::endl;
        std::cout << "Survival rate: " << std::fixed << std::setprecision(1) 
                  << (alive_count_ * 100.0 / 50.0) << "%" << std::endl;
        
        std::ofstream final_file("final_state.txt");
        if (final_file) {
            final_file << "=== Final Game State ===" << std::endl;
            final_file << "Survivors: " << alive_count_ << std::endl;
            for (const auto& npc : npcs) {
                final_file << npc->type() << " " << npc->name() << " "
                          << npc->x() << " " << npc->y() << std::endl;
            }
            std::cout << "\nFinal state saved to 'final_state.txt'" << std::endl;
        }
    }
}

void Game::start() {
    if (running_) return;
    
    running_ = true;
    
    generateInitialNPCs();
    
    movement_thread_ = std::thread(&Game::movementWorker, this);
    battle_thread_ = std::thread(&Game::battleWorker, this);
    main_thread_ = std::thread(&Game::mainWorker, this);
    
    {
        std::lock_guard<std::mutex> cout_lock(cout_mutex_);
        std::cout << std::string(60, '=') << std::endl;
        std::cout << "=== MULTI-THREADED NPC GAME STARTED ===" << std::endl;
        std::cout << "Based on Lab 6 Variant 19: Bear, Bittern (Выпь), Desman (Выхухоль)" << std::endl;
        std::cout << std::string(60, '-') << std::endl;
        std::cout << "Initial NPCs: 50 (randomly generated)" << std::endl;
        std::cout << "Movement distance: " << MOVE_DISTANCE << " units (Desman/Выхухоль)" << std::endl;
        std::cout << "Kill distance: " << KILL_DISTANCE << " units (Desman/Выхухоль)" << std::endl;
        std::cout << "Map size: " << MAP_WIDTH << "x" << MAP_HEIGHT << " units" << std::endl;
        std::cout << "Game duration: 30 seconds" << std::endl;
        std::cout << "Threads: Movement, Battle, Main (map display)" << std::endl;
        std::cout << "Battle rules (Lab 6):" << std::endl;
        std::cout << "  - Bear kills everyone except Bears" << std::endl;
        std::cout << "  - Bittern kills no one" << std::endl;
        std::cout << "  - Desman kills Bears" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
}

void Game::stop() {
    running_ = false;
    
    if (movement_thread_.joinable()) movement_thread_.join();
    if (battle_thread_.joinable()) battle_thread_.join();
    if (main_thread_.joinable()) main_thread_.join();
}

void Game::waitForFinish() {
    if (main_thread_.joinable()) {
        main_thread_.join();
    }
}