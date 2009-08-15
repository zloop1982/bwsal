#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <BuildManager.h>
#include <BuildOrderManager.h>
class SupplyManager
{
  public:
    SupplyManager(BuildManager* buildManager, BuildOrderManager* buildOrderManager);
    void update();
    std::string getName() const;
    int getPlannedSupply() const;
    BuildManager* buildManager;
    BuildOrderManager* buildOrderManager;
    int lastFrameCheck;
};