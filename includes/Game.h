#pragma once
#include "Editor.h"
#include "NPCFactory.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <random>
#include <vector>
#include <chrono>

class Game {
private:
    Editor editor_;
    mutable std::shared_mutex npc_mutex_;
    mutable std::mutex cout_mutex_;
    
    std::atomic<bool> running_{false};
    std::atomic<int> alive_count_{0};
    
    std::thread movement_thread_;
    std::thread battle_thread_;
    std::thread main_thread_;
    
    static constexpr double MOVE_DISTANCE = 5.0;   
    static constexpr double KILL_DISTANCE = 20.0;   
    
    static constexpr double MAP_WIDTH = 100.0;
    static constexpr double MAP_HEIGHT = 100.0;
    
    std::random_device rd_;
    mutable std::mt19937 gen_;
    std::uniform_real_distribution<> pos_dist_;
    std::uniform_int_distribution<> type_dist_;
    
    void generateInitialNPCs();
    double calculateDistance(double x1, double y1, double x2, double y2) const;
    void movementWorker();
    void battleWorker();
    void mainWorker();
    
public:
    Game();
    ~Game();
    
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    
    void start();
    void stop();
    void waitForFinish();
    
    int getAliveCount() const { return alive_count_; }
    const Editor& getEditor() const { return editor_; }
};