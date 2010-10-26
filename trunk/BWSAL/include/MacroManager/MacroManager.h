#pragma once
#include <BWAPI.h>
#include <Arbitrator.h>
#include <ResourceTimeline.h>
class TaskStream;
class MacroManager
{
  public:
    MacroManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    void update();
    std::list< TaskStream* > taskStreams;
    std::set< TaskStream* > killSet;
    TaskStream* getTaskStream(BWAPI::Unit* unit);
    Resources getGatherRate() const;
    ResourceTimeline rtl;
    std::map< BWAPI::Unit*, TaskStream* > unitToTaskStream;
  private:
    void computeGatherRate();
    Resources gatherRateA;
    Resources gatherRateB;
    Resources gatherRateC;
    Resources gatherRate;
    int gatherRateStepSize;
    std::list<Resources> resourceHistory;
};
extern MacroManager* TheMacroManager;
extern Arbitrator::Arbitrator<BWAPI::Unit*,double>* TheArbitrator;