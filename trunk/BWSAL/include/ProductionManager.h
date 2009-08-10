#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <BuildingPlacer.h>
class ProductionManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
    class Unit
    {
      public:
        BWAPI::UnitType type;
        int lastAttemptFrame;
        BWAPI::Unit* unit;
    };
    ProductionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer);
    virtual void onOffer(std::set<BWAPI::Unit*> units);
    virtual void onRevoke(BWAPI::Unit* unit, double bid);
    virtual void update();
    virtual std::string getName() const;
    void onRemoveUnit(BWAPI::Unit* unit);
    bool train(BWAPI::UnitType type);
    Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
    std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> > productionQueues;
    std::map<BWAPI::Unit*,Unit> producingUnits;
    BuildingPlacer* placer;
};