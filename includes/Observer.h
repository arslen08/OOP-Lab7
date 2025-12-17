#pragma once
#include <string>
#include <memory>
#include <vector>

class FightObserver {
public:
    virtual void onKill(const std::string &killer, const std::string &victim) = 0;
    virtual ~FightObserver() = default;
};

using ObsPtr = std::shared_ptr<FightObserver>;

class ConsoleObserver : public FightObserver {
public:
    void onKill(const std::string &killer, const std::string &victim) override;
};

class FileObserver : public FightObserver {
    std::string filename_;
public:
    explicit FileObserver(const std::string &fname = "log.txt");
    void onKill(const std::string &killer, const std::string &victim) override;
};