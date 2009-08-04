#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <BuildingPlacer.h>
class ConstructionManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
  class Building
  {
    public:
    BWAPI::TilePosition tilePosition;
    BWAPI::Position position;
    BWAPI::UnitType type;
    BWAPI::Unit* buildingUnit;
    BWAPI::Unit* builderUnit;
  };
  ConstructionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer);
  virtual void onOffer(std::set<BWAPI::Unit*> units);
  virtual void onRevoke(BWAPI::Unit* unit, double bid);
  virtual void update();
  virtual std::string getName();
  void onRemoveUnit(BWAPI::Unit* unit);
  bool build(BWAPI::UnitType type);
  Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
  BuildingPlacer* placer;
  std::map<BWAPI::Unit*,Building*> builders;
  std::vector<Building*> incompleteBuildings;
  std::set<Building*> buildingsNeedingBuilders;
};