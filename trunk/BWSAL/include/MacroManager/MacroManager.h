#pragma once
#include <BWAPI.h>
#include <Arbitrator.h>
#include <ResourceTimeline.h>
#include <UnitCountTimeline.h>
class TaskStream;
class MacroManager
{
  public:
    static MacroManager* create(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    MacroManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    ~MacroManager();
    void update();
    TaskStream* getTaskStream(BWAPI::Unit* unit) const;
    std::list< TaskStream* > taskStreams;
    std::set< TaskStream* > killSet;
    ResourceTimeline rtl;
    UnitCountTimeline uctl;
    Resources spentResources;
    std::map< BWAPI::Unit*, TaskStream* > unitToTaskStream;
};
extern MacroManager* TheMacroManager;
extern Arbitrator::Arbitrator<BWAPI::Unit*,double>* TheArbitrator;