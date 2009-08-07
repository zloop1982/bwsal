#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
class BuildingPlacer;
class ConstructionManager;
class ProductionManager;
class MorphManager;
class BuildManager
{
  public:
    BuildManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    ~BuildManager();
    void update();
    std::string getName() const;
    void onRemoveUnit(BWAPI::Unit* unit);
    bool build(BWAPI::UnitType type);

  private:
    Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
    BuildingPlacer* buildingPlacer;
    ConstructionManager* constructionManager;
    ProductionManager* productionManager;
    MorphManager* morphManager;
};