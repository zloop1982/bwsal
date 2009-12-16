#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include "UnitItem.h"
#include "TechItem.h"
class BuildManager;
class TechManager;
class UpgradeManager;
class WorkerManager;
class BuildOrderManager
{
  public:
    class PriorityLevel
    {
      public:
        std::list<TechItem> techs;
        std::map<BWAPI::UnitType, std::map<BWAPI::UnitType, UnitItem > > units;
    };
    class Resources
    {
      public:
        Resources() : minerals(0),gas(0) {}
        Resources(int m, int g) : minerals(m),gas(g) {}
        int minerals;
        int gas;
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
    std::pair<int, Resources> reserveResources(BWAPI::Unit* builder, BWAPI::UnitType unitType);
    void unreserveResources(std::pair<int, BuildOrderManager::Resources> res);
    bool updateUnits();
    std::set<BWAPI::UnitType> unitsCanMake(BWAPI::Unit* builder, int time);
    BuildManager* buildManager;
    TechManager* techManager;
    UpgradeManager* upgradeManager;
    WorkerManager* workerManager;
    std::map<int, PriorityLevel > items;
    int usedMinerals;
    int usedGas;
    std::map<int, Resources> reservedResources;
    std::set<BWAPI::Unit*> reservedUnits;
};