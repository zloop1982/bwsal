#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
class MorphManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
    MorphManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    virtual void onOffer(std::set<BWAPI::Unit*> units);
    virtual void onRevoke(BWAPI::Unit* unit, double bid);
    virtual void update();
    virtual std::string getName() const;
    void onRemoveUnit(BWAPI::Unit* unit);
    bool morph(BWAPI::UnitType type);
    Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
    std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> > morphQueues;
    std::map<BWAPI::Unit*,BWAPI::UnitType> morphingUnits;
};