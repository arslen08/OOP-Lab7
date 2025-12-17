#include "Observer.h"
#include <iostream>
#include <fstream>

void ConsoleObserver::onKill(const std::string &killer, const std::string &victim) {
    std::cout << "[EVENT] " << killer << " killed " << victim << "\n";
}

FileObserver::FileObserver(const std::string &fname) : filename_(fname) {}

void FileObserver::onKill(const std::string &killer, const std::string &victim) {
    std::ofstream out(filename_, std::ios::app);
    out << killer << " killed " << victim << "\n";
}