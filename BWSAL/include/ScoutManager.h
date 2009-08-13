#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>

class ScoutManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
  public:
    class ScoutData
    {
      public:
        enum ScoutMode
        {
          Idle,
          Searching,
          Roaming,
          Harassing,
          Fleeing
        };
        ScoutData(){ mode = Idle; target = NULL; }
        BWAPI::Position* target;
        ScoutMode mode;
    };
    ScoutManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    virtual void onOffer(std::set<BWAPI::Unit*> units);
    virtual void onRevoke(BWAPI::Unit* unit, double bid);
    virtual void update();

    virtual std::string getName() const;
    void onRemoveUnit(BWAPI::Unit* unit);

    std::map<BWAPI::Unit*, ScoutData> scouts;
    Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
};