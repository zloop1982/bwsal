#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
class BuildManager;
class TechManager;
class UpgradeManager;
class WorkerManager;
class BuildOrderManager
{
  public:
    class BuildItem
    {
      public:
        BWAPI::UnitType unitType;
        BWAPI::TechType techType;
        BWAPI::UpgradeType upgradeType;
        BWAPI::Position seedPosition;
        bool isAdditional;
        int count;
    };
    class PriorityLevel
    {
      public:
        std::list<BuildItem> techs;
        std::list<BuildItem> upgrades;
        std::list<BuildItem> buildings;
        std::list<BuildItem> units;
    };
    BuildOrderManager(BuildManager* buildManager, TechManager* techManager, UpgradeManager* upgradeManager, WorkerManager* workerManager);
    void update();
    std::string getName() const;
    void build(int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedPosition=BWAPI::TilePositions::None);
    void buildAdditional(int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedPosition=BWAPI::TilePositions::None);
    void research(BWAPI::TechType t, int priority);
    void upgrade(int level, BWAPI::UpgradeType t, int priority);
    bool hasResources(BWAPI::UnitType t);
    bool hasResources(BWAPI::TechType t);
    bool hasResources(BWAPI::UpgradeType t);
    void spendResources(BWAPI::UnitType t);
    void spendResources(BWAPI::TechType t);
    void spendResources(BWAPI::UpgradeType t);
    int getPlannedCount(BWAPI::UnitType t);

  private:
    void reserveResources(BWAPI::Unit* builder, BWAPI::UnitType unitType);
    void unreserveResources(BWAPI::Unit* builder, BWAPI::UnitType unitType);
    BuildManager* buildManager;
    TechManager* techManager;
    UpgradeManager* upgradeManager;
    WorkerManager* workerManager;
    std::map<int, PriorityLevel > items;
    int usedMinerals;
    int usedGas;
    std::map<int, int> reservedMinerals;
    std::map<int, int> reservedGas;
};