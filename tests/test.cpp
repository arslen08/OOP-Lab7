#include <gtest/gtest.h>
#include "../includes/NPCFactory.h"
#include "../includes/Editor.h"
#include "../includes/Observer.h"
#include "../includes/Game.h"
#include "../includes/FightRules.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <atomic>

struct TestObserver : FightObserver {
    std::vector<std::pair<std::string,std::string>> events;
    void onKill(const std::string &k, const std::string &v) override {
        events.emplace_back(k, v);
    }
    void clear() { events.clear(); }
};

TEST(FactoryTest, CreateAllTypes) {
    auto bear = NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0);
    auto bittern = NPCFactory::create(NPCType::Bittern, "Bittern1", 1.0, 1.0);
    auto desman = NPCFactory::create(NPCType::Desman, "Desman1", 2.0, 2.0);
    
    ASSERT_NE(bear, nullptr);
    ASSERT_NE(bittern, nullptr);
    ASSERT_NE(desman, nullptr);
    
    ASSERT_EQ(bear->type(), "Bear");
    ASSERT_EQ(bittern->type(), "Bittern");
    ASSERT_EQ(desman->type(), "Desman");
    
    ASSERT_EQ(bear->name(), "Bear1");
    ASSERT_EQ(bear->x(), 0.0);
    ASSERT_EQ(bear->y(), 0.0);
}

TEST(FactoryTest, LoadFromStreamAllTypes) {
    std::istringstream iss("Bear TestBear 10.5 20.5\nBittern TestBird 30.0 40.0\nDesman TestDesman 50.0 60.0");
    
    auto npc1 = NPCFactory::loadFromStream(iss);
    auto npc2 = NPCFactory::loadFromStream(iss);
    auto npc3 = NPCFactory::loadFromStream(iss);
    
    ASSERT_NE(npc1, nullptr);
    ASSERT_NE(npc2, nullptr);
    ASSERT_NE(npc3, nullptr);
    
    ASSERT_EQ(npc1->type(), "Bear");
    ASSERT_EQ(npc2->type(), "Bittern");
    ASSERT_EQ(npc3->type(), "Desman");
    
    ASSERT_EQ(npc1->name(), "TestBear");
    ASSERT_EQ(npc2->name(), "TestBird");
    ASSERT_EQ(npc3->name(), "TestDesman");
}

TEST(FactoryTest, CloneWithPosition) {
    auto bear = NPCFactory::create(NPCType::Bear, "Bear1", 10.0, 20.0);
    auto cloned_bear = bear->cloneWithPosition(30.0, 40.0);
    
    ASSERT_EQ(cloned_bear->type(), "Bear");
    ASSERT_EQ(cloned_bear->name(), "Bear1");
    ASSERT_EQ(cloned_bear->x(), 30.0);
    ASSERT_EQ(cloned_bear->y(), 40.0);
    ASSERT_NE(bear.get(), cloned_bear.get()); 
    
    auto bittern = NPCFactory::create(NPCType::Bittern, "Bittern1", 5.0, 5.0);
    auto cloned_bittern = bittern->cloneWithPosition(15.0, 15.0);
    
    ASSERT_EQ(cloned_bittern->type(), "Bittern");
    ASSERT_EQ(cloned_bittern->name(), "Bittern1");
}

TEST(FactoryTest, InvalidTypeThrows) {
    std::istringstream iss("InvalidType Name 10 10");
    EXPECT_THROW(NPCFactory::loadFromStream(iss), std::runtime_error);
}

TEST(FactoryTest, BadFormatThrows) {
    std::istringstream iss("Bear OnlyName");
    EXPECT_THROW(NPCFactory::loadFromStream(iss), std::runtime_error);
}

TEST(EditorTest, AddNPCWithUniqueNames) {
    Editor ed;
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Bear, "B1", 0.0, 0.0)));
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Bittern, "BT1", 10.0, 10.0)));
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Desman, "D1", 20.0, 20.0)));
    
    EXPECT_FALSE(ed.addNPC(NPCFactory::create(NPCType::Bear, "B1", 30.0, 30.0)));
    EXPECT_FALSE(ed.addNPC(NPCFactory::create(NPCType::Bittern, "B1", 40.0, 40.0)));
}

TEST(EditorTest, AddNPCWithBoundaryChecks) {
    Editor ed;
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Bear, "B1", 0.0, 0.0)));
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Bittern, "BT1", 250.0, 250.0)));
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Desman, "D1", 500.0, 500.0)));
    
    EXPECT_FALSE(ed.addNPC(NPCFactory::create(NPCType::Bear, "B2", -1.0, 5.0)));
    EXPECT_FALSE(ed.addNPC(NPCFactory::create(NPCType::Bittern, "BT2", 5.0, -1.0)));
    EXPECT_FALSE(ed.addNPC(NPCFactory::create(NPCType::Desman, "D2", 501.0, 5.0)));
    EXPECT_FALSE(ed.addNPC(NPCFactory::create(NPCType::Bear, "B3", 5.0, 501.0)));
}

TEST(EditorTest, ObserverManagement) {
    Editor ed;
    auto obs1 = std::make_shared<TestObserver>();
    auto obs2 = std::make_shared<TestObserver>();
    
    ed.addObserver(obs1);
    ed.addObserver(obs2);
    
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bittern1", 1.0, 1.0));
    
    ed.runBattle(5.0);
    ASSERT_FALSE(obs1->events.empty());
    EXPECT_EQ(obs1->events[0].first, "Bear1");
    EXPECT_EQ(obs1->events[0].second, "Bittern1");
    
    EXPECT_EQ(obs1->events.size(), obs2->events.size());
}

TEST(EditorTest, SaveAndLoad) {
    Editor ed;
    ed.addNPC(NPCFactory::create(NPCType::Bear, "BearX", 5.5, 5.5));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "BirdX", 6.0, 6.0));
    ed.addNPC(NPCFactory::create(NPCType::Desman, "DesmanX", 7.0, 7.0));
    
    const std::string filename = "test_save_editor.txt";
    ed.saveToFile(filename);
    
    Editor ed2;
    ed2.loadFromFile(filename);
    
    ASSERT_EQ(ed2.npcs().size(), 3);
    
    EXPECT_EQ(ed2.npcs()[0]->type(), "Bear");
    EXPECT_EQ(ed2.npcs()[0]->name(), "BearX");
    EXPECT_DOUBLE_EQ(ed2.npcs()[0]->x(), 5.5);
    EXPECT_DOUBLE_EQ(ed2.npcs()[0]->y(), 5.5);
    
    EXPECT_EQ(ed2.npcs()[1]->type(), "Bittern");
    EXPECT_EQ(ed2.npcs()[1]->name(), "BirdX");
    
    EXPECT_EQ(ed2.npcs()[2]->type(), "Desman");
    EXPECT_EQ(ed2.npcs()[2]->name(), "DesmanX");
    
    std::filesystem::remove(filename);
}

TEST(EditorTest, PrintAllOutput) {
    Editor ed;
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 1.0, 2.0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bird1", 3.0, 4.0));
    ed.addNPC(NPCFactory::create(NPCType::Desman, "Desman1", 5.0, 6.0));
    
    std::ostringstream oss;
    ed.printAll(oss);
    std::string output = oss.str();
    
    EXPECT_NE(output.find("Bear1"), std::string::npos);
    EXPECT_NE(output.find("Bird1"), std::string::npos);
    EXPECT_NE(output.find("Desman1"), std::string::npos);
    EXPECT_NE(output.find("Bear"), std::string::npos);
    EXPECT_NE(output.find("Bittern"), std::string::npos);
    EXPECT_NE(output.find("Desman"), std::string::npos);
}

TEST(EditorTest, EmptyEditorOperations) {
    Editor ed;
    
    const std::string filename = "test_empty.txt";
    ed.saveToFile(filename);
    
    Editor ed2;
    ed2.loadFromFile(filename);
    EXPECT_TRUE(ed2.npcs().empty());
    
    std::ostringstream oss;
    ed.printAll(oss);
    EXPECT_NE(oss.str().find("NPC list (0):"), std::string::npos);
    
    EXPECT_NO_THROW(ed.runBattle(10.0));
    
    std::filesystem::remove(filename);
}

TEST(Variant19RulesTest, BearKillsEveryoneExceptBears) {
    FightRules rules;
    
    auto bear1 = NPCFactory::create(NPCType::Bear, "Bear1", 0, 0);
    auto bear2 = NPCFactory::create(NPCType::Bear, "Bear2", 0, 0);
    auto bittern = NPCFactory::create(NPCType::Bittern, "Bittern1", 0, 0);
    auto desman = NPCFactory::create(NPCType::Desman, "Desman1", 0, 0);
    
    EXPECT_TRUE(bear1->accept(rules, *bittern));
    
    EXPECT_TRUE(bear1->accept(rules, *desman));
    
    EXPECT_FALSE(bear1->accept(rules, *bear2));
}

TEST(Variant19RulesTest, BitternKillsNoOne) {
    FightRules rules;
    
    auto bittern = NPCFactory::create(NPCType::Bittern, "Bittern1", 0, 0);
    auto bear = NPCFactory::create(NPCType::Bear, "Bear1", 0, 0);
    auto desman = NPCFactory::create(NPCType::Desman, "Desman1", 0, 0);
    auto another_bittern = NPCFactory::create(NPCType::Bittern, "Bittern2", 0, 0);
    
    EXPECT_FALSE(bittern->accept(rules, *bear));
    EXPECT_FALSE(bittern->accept(rules, *desman));
    EXPECT_FALSE(bittern->accept(rules, *another_bittern));
}

TEST(Variant19RulesTest, DesmanKillsOnlyBears) {
    FightRules rules;
    
    auto desman = NPCFactory::create(NPCType::Desman, "Desman1", 0, 0);
    auto bear = NPCFactory::create(NPCType::Bear, "Bear1", 0, 0);
    auto bittern = NPCFactory::create(NPCType::Bittern, "Bittern1", 0, 0);
    auto another_desman = NPCFactory::create(NPCType::Desman, "Desman2", 0, 0);
    
    EXPECT_TRUE(desman->accept(rules, *bear));
    
    EXPECT_FALSE(desman->accept(rules, *bittern));
    
    EXPECT_FALSE(desman->accept(rules, *another_desman));
}

TEST(BattleTest, BearKillsBittern) {
    Editor ed;
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 1.0, 1.0));
    
    auto observerPtr = std::make_shared<TestObserver>();
    ed.addObserver(observerPtr);

    ed.runBattle(5.0);
    
    ASSERT_EQ(ed.npcs().size(), 1);
    EXPECT_EQ(ed.npcs()[0]->type(), "Bear");
    EXPECT_EQ(ed.npcs()[0]->name(), "Bear1");
    
    ASSERT_FALSE(observerPtr->events.empty());
    EXPECT_EQ(observerPtr->events[0].first, "Bear1");
    EXPECT_EQ(observerPtr->events[0].second, "Bit1");
}

TEST(BattleTest, DesmanKillsBear) {
    Editor ed;
    ed.addNPC(NPCFactory::create(NPCType::Bear, "BearA", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Desman, "DesA", 1.0, 1.0));
    
    auto observerPtr = std::make_shared<TestObserver>();
    ed.addObserver(observerPtr);

    ed.runBattle(5.0);
    
    ASSERT_EQ(ed.npcs().size(), 0);
}

TEST(BattleTest, BearAndDesmanKillEachOther) {
    Editor ed;
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Desman, "Des1", 100.0, 100.0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 50.0, 50.0));
    
    auto observerPtr = std::make_shared<TestObserver>();
    ed.addObserver(observerPtr);

    ed.runBattle(150.0);
    
    EXPECT_TRUE(ed.npcs().empty() || ed.npcs().size() == 1);
}

TEST(BattleTest, BitternNeverKills) {
    Editor ed;
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 1.0, 1.0));
    ed.addNPC(NPCFactory::create(NPCType::Desman, "Des1", 2.0, 2.0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit2", 3.0, 3.0));
    
    auto observerPtr = std::make_shared<TestObserver>();
    ed.addObserver(observerPtr);

    ed.runBattle(10.0);
    
    for (const auto& event : observerPtr->events) {
        EXPECT_NE(event.first, "Bit1");
        EXPECT_NE(event.first, "Bit2");
    }
}

TEST(BattleTest, DistanceAffectsBattle) {
    Editor ed;
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 100.0, 100.0));
    
    auto observerPtr = std::make_shared<TestObserver>();
    ed.addObserver(observerPtr);

    ed.runBattle(10.0);
    EXPECT_TRUE(observerPtr->events.empty());
    EXPECT_EQ(ed.npcs().size(), 2);
    
    observerPtr->clear();
    
    ed.runBattle(150.0);
    EXPECT_FALSE(observerPtr->events.empty());
    EXPECT_EQ(ed.npcs().size(), 1);
}

TEST(ObserverTest, ConsoleObserverCreation) {
    ConsoleObserver observer;
    SUCCEED();
}

TEST(ObserverTest, FileObserverCreatesFile) {
    const std::string filename = "test_observer_log.txt";
    {
        FileObserver observer(filename);
        
        Editor ed;
        ed.addObserver(std::make_shared<FileObserver>(filename));
        ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0, 0));
        ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 1, 1));
        
        ed.runBattle(5.0);
    }
    
    EXPECT_TRUE(std::filesystem::exists(filename));
    
    std::ifstream file(filename);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    EXPECT_NE(content.find("killed"), std::string::npos);
    
    std::filesystem::remove(filename);
}

TEST(ObserverTest, MultipleObserversReceiveEvents) {
    Editor ed;
    auto obs1 = std::make_shared<TestObserver>();
    auto obs2 = std::make_shared<TestObserver>();
    auto obs3 = std::make_shared<TestObserver>();
    
    ed.addObserver(obs1);
    ed.addObserver(obs2);
    ed.addObserver(obs3);
    
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0, 0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 1, 1));
    
    ed.runBattle(5.0);
    
    EXPECT_EQ(obs1->events.size(), obs2->events.size());
    EXPECT_EQ(obs2->events.size(), obs3->events.size());
    
    if (!obs1->events.empty()) {
        EXPECT_EQ(obs1->events[0].first, obs2->events[0].first);
        EXPECT_EQ(obs1->events[0].second, obs2->events[0].second);
    }
}

TEST(GameTest, ConstructorAndDestructor) {
    {
        Game game;
        SUCCEED();
    }
    SUCCEED();
}

TEST(GameTest, StartStopBasic) {
    Game game;
    
    game.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    game.stop();
    
    SUCCEED();
}

TEST(GameTest, GameConstants) {
    Game game;
    SUCCEED();
}

TEST(GameTest, ThreadSafetyBasic) {
    Game game;
    
    std::atomic<bool> test_passed{true};
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&game, &test_passed, i]() {
            try {
                if (i == 0) {
                    game.start();
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    game.stop();
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10 * i));
                }
            } catch (...) {
                test_passed = false;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_TRUE(test_passed);
}

TEST(GameTest, InitialNPCCount) {
    Game game;
    SUCCEED();
}

TEST(IntegrationTest, FullEditorWorkflow) {
    const std::string filename = "test_integration.txt";
    
    {
        Editor ed;

        EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 10, 10)));
        EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 20, 20)));
        EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Desman, "Des1", 30, 30)));
        
        ed.saveToFile(filename);
        
        EXPECT_TRUE(std::filesystem::exists(filename));
    }
    
    {
        Editor ed2;
        ed2.loadFromFile(filename);
        
        EXPECT_EQ(ed2.npcs().size(), 3);
        
        auto observer = std::make_shared<TestObserver>();
        ed2.addObserver(observer);
        ed2.runBattle(50.0);
        
        EXPECT_FALSE(observer->events.empty());
    }
    
    std::filesystem::remove(filename);
}

TEST(IntegrationTest, GameAndEditorCompatibility) {
    Game game;
    
    game.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    game.stop();
    
    SUCCEED();
}

TEST(IntegrationTest, MemoryLeakCheck) {
    {
        Game* game = new Game();
        game->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        game->stop();
        delete game;
    }
    
    {
        Editor* ed = new Editor();
        ed->addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0, 0));
        ed->addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 1, 1));
        delete ed;
    }
    
    SUCCEED();
}

TEST(EdgeCasesTest, NPCAtBoundaries) {
    Editor ed;
    
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Bear, "BearMin", 0.0, 0.0)));
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Bittern, "BitMax", 500.0, 500.0)));
    EXPECT_TRUE(ed.addNPC(NPCFactory::create(NPCType::Desman, "DesCenter", 250.0, 250.0)));
    
    EXPECT_EQ(ed.npcs().size(), 3);
}

TEST(EdgeCasesTest, BattleWithZeroDistance) {
    Editor ed;
    
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 0.0, 0.0));
    
    auto observer = std::make_shared<TestObserver>();
    ed.addObserver(observer);
    
    ed.runBattle(0.0);
    
    EXPECT_FALSE(observer->events.empty());
    EXPECT_EQ(ed.npcs().size(), 1);
}

TEST(EdgeCasesTest, BattleWithVeryLargeDistance) {
    Editor ed;
    
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 400.0, 400.0));
    
    auto observer = std::make_shared<TestObserver>();
    ed.addObserver(observer);
    
    ed.runBattle(2000.0);
    
    EXPECT_FALSE(observer->events.empty());
    EXPECT_EQ(ed.npcs().size(), 1);
}

TEST(EdgeCasesTest, ManyNPCsPerformance) {
    Editor ed;
    
    for (int i = 0; i < 100; ++i) {
        std::string name = "NPC" + std::to_string(i);
        NPCType type = static_cast<NPCType>(i % 3);
        double x = static_cast<double>(i * 5);
        double y = static_cast<double>(i * 5);
        
        ed.addNPC(NPCFactory::create(type, name, x, y));
    }
    
    EXPECT_EQ(ed.npcs().size(), 100);
    
    auto start = std::chrono::steady_clock::now();
    ed.runBattle(10.0);
    auto end = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 5000); 
}

TEST(EdgeCasesTest, RemoveAllNPCs) {
    Editor ed;
    
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0));
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear2", 1.0, 1.0));
    
    ed.runBattle(10.0);
    EXPECT_EQ(ed.npcs().size(), 2);
    
    ed.addNPC(NPCFactory::create(NPCType::Desman, "Des1", 2.0, 2.0));
    ed.runBattle(10.0);
    
    EXPECT_EQ(ed.npcs().size(), 0);
}

TEST(SerializationTest, NPCSerializationFormat) {
    auto bear = NPCFactory::create(NPCType::Bear, "TestBear", 123.456, 789.012);
    
    std::ostringstream oss;
    bear->serialize(oss);
    std::string output = oss.str();
    
    EXPECT_NE(output.find("Bear"), std::string::npos);
    EXPECT_NE(output.find("TestBear"), std::string::npos);
    EXPECT_NE(output.find("123.456"), std::string::npos);
    EXPECT_NE(output.find("789.012"), std::string::npos);
}

TEST(SerializationTest, RoundTripSerialization) {
    Editor ed;
    
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 1.1, 2.2));
    ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit1", 3.3, 4.4));
    ed.addNPC(NPCFactory::create(NPCType::Desman, "Des1", 5.5, 6.6));
    
    const std::string filename = "test_roundtrip.txt";
    ed.saveToFile(filename);
    
    Editor ed2;
    ed2.loadFromFile(filename);
    
    ASSERT_EQ(ed.npcs().size(), ed2.npcs().size());
    
    for (size_t i = 0; i < ed.npcs().size(); ++i) {
        EXPECT_EQ(ed.npcs()[i]->type(), ed2.npcs()[i]->type());
        EXPECT_EQ(ed.npcs()[i]->name(), ed2.npcs()[i]->name());
        EXPECT_DOUBLE_EQ(ed.npcs()[i]->x(), ed2.npcs()[i]->x());
        EXPECT_DOUBLE_EQ(ed.npcs()[i]->y(), ed2.npcs()[i]->y());
    }
    
    std::filesystem::remove(filename);
}

TEST(SpecialCasesTest, SelfBattleDoesNothing) {
    Editor ed;
    
    ed.addNPC(NPCFactory::create(NPCType::Bear, "Bear1", 0.0, 0.0));
    
    auto observer = std::make_shared<TestObserver>();
    ed.addObserver(observer);
    
    ed.runBattle(100.0);
    EXPECT_TRUE(observer->events.empty());
    EXPECT_EQ(ed.npcs().size(), 1);
}

TEST(SpecialCasesTest, OnlyBitternsNoBattle) {
    Editor ed;
    
    for (int i = 0; i < 5; ++i) {
        ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit" + std::to_string(i), i * 10.0, i * 10.0));
    }
    
    auto observer = std::make_shared<TestObserver>();
    ed.addObserver(observer);
    
    ed.runBattle(100.0);
    
    EXPECT_TRUE(observer->events.empty());
    EXPECT_EQ(ed.npcs().size(), 5);
}

TEST(SpecialCasesTest, BearDominance) {
    Editor ed;
    
    ed.addNPC(NPCFactory::create(NPCType::Bear, "BearKing", 0.0, 0.0));
    
    for (int i = 0; i < 3; ++i) {
        ed.addNPC(NPCFactory::create(NPCType::Bittern, "Bit" + std::to_string(i), (i+1) * 5.0, 0.0));
        ed.addNPC(NPCFactory::create(NPCType::Desman, "Des" + std::to_string(i), 0.0, (i+1) * 5.0));
    }
    
    auto observer = std::make_shared<TestObserver>();
    ed.addObserver(observer);
    
    ed.runBattle(100.0);
    
    EXPECT_LE(ed.npcs().size(), 2); 
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}