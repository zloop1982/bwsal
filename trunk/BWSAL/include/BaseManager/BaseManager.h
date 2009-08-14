#pragma once
#include <map>
#include <set>
#include <BWAPI.h>
#include <Arbitrator.h>
#include <BWTA.h>
#include <BuildManager.h>
#include "Base.h"
class BaseManager
{
  public:
  BaseManager(BuildManager* builder);
  void update();
  void addBase(BWTA::BaseLocation* location);
  std::set<Base*> getActiveBases() const;
  std::set<Base*> getAllBases() const;

  std::string getName();
  void onRemoveUnit(BWAPI::Unit* unit);

  BuildManager* builder;
  std::map<BWTA::BaseLocation*,Base*> location2base;
  std::set<Base*> allBases;
};