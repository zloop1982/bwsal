#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <BuildManager.h>
class SupplyManager
{
  public:
    SupplyManager(BuildManager* builder);
    void update();
    std::string getName() const;
    void onRemoveUnit(BWAPI::Unit* unit);
    BuildManager* builder;
    int plannedSupply;
    int lastFrameCheck;
};