#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
class ProductionManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
    ProductionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    virtual void onOffer(std::set<BWAPI::Unit*> units);
    virtual void onRevoke(BWAPI::Unit* unit, double bid);
    virtual void update();
    virtual std::string getName();
    void onRemoveUnit(BWAPI::Unit* unit);
    bool train(BWAPI::UnitType type);
    Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
    std::map<BWAPI::UnitType,std::list<BWAPI::Unit*> > factories;
    std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> > factoriesQueues;
};