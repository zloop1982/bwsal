#pragma once
#include <BWAPI.h>
#include <Arbitrator.h>
#include <ResourceTimeline.h>
#include <UnitCountTimeline.h>
#include <TechTimeline.h>
#include <UpgradeTimeline.h>
#include <Task.h>
class TaskStream;
class MacroManager
{
  public:
    static MacroManager* create(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    MacroManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
    ~MacroManager();
    void update();

    /** Inserts the given task stream right above the given one */
    bool insertTaskStreamAbove(TaskStream* newTS, TaskStream* existingTS);

    /** Inserts the given task stream right below the given one */
    bool insertTaskStreamBelow(TaskStream* newTS, TaskStream* existingTS);

    /** Swaps the position of the two given task streams */
    bool swapTaskStreams(TaskStream* a, TaskStream* b);

    /** Gets the task stream of which this unit is a worker */
    TaskStream* getTaskStream(BWAPI::Unit* unit) const;

    std::list< TaskStream* > taskStreams;
    std::set< TaskStream* > killSet;
    ResourceTimeline rtl;
    UnitCountTimeline uctl;
    TechTimeline ttl;
    UpgradeTimeline utl;
    Resources spentResources;
    std::map< BWAPI::Unit*, TaskStream* > unitToTaskStream;
    std::map< int, std::list< std::pair<TaskStream*, Task > > > plan;
    bool taskstream_list_visible;
};
extern MacroManager* TheMacroManager;
extern Arbitrator::Arbitrator<BWAPI::Unit*,double>* TheArbitrator;