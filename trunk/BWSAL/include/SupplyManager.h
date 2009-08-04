#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <ConstructionManager.h>
class SupplyManager
{
  public:
  SupplyManager(ConstructionManager* builder);
  void update();
  virtual std::string getName();
  void onRemoveUnit(BWAPI::Unit* unit);
  ConstructionManager* builder;
  int plannedSupply;
  int lastFrameCheck;
};