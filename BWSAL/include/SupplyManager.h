#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <BuildManager.h>
class SupplyManager
{
  public:
    SupplyManager(BuildManager* buildManager);
    void update();
    std::string getName() const;
    int getPlannedSupply() const;
    BuildManager* buildManager;
    int lastFrameCheck;
};