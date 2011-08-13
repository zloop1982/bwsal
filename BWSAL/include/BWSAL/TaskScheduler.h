#pragma once
#include <map>
#include <BWSAL/BuildEvent.h>
#include <BWSAL/Types.h>
namespace BWSAL
{
  class BuildEventTimeline;
  class Task;
  class BuildUnit;
  class BuildUnitManager;
  class TaskScheduler : public BWAPI::AIModule
  {
    public:
      static TaskScheduler* create( BuildEventTimeline* timeline, BuildUnitManager* buildUnitManager );
      static TaskScheduler* getInstance();
      static void destroy();
      void scheduleTask( Task* t );
    private:
      TaskScheduler();
      ~TaskScheduler();
      void scheduleLarvaUsingTask( Task* t );
      void scheduleTask( Task* t, BuildUnit* builder, int startTime );
      bool canCompleteWithUnitBeforeNextEvent( int validBuildTypeSince,
                                               BuildUnit* unit,
                                               const Task* t,
                                               const std::list< std::pair< int, BuildEvent > >::const_iterator nextEvent );
      // Planning data associated with a particular HLH ( Hatchery, Lair, or Hive )
      class HLHPlanData
      {
        public:
          HLHPlanData();
          int candidateNextLarvaSpawnTime;
          int candidateLarvaCount;
          int candidateMorphTime;
          bool candidateMorphed;
      };
      void TaskScheduler::resetCandidates( std::map< BuildUnit*, HLHPlanData > *hlhPlans, BuildState* state );
      void TaskScheduler::initializeHLHPlanData( std::map< BuildUnit*, HLHPlanData > *hlhPlans );
      void TaskScheduler::continueToTimeWithLarvaSpawns( BuildState* state, std::map< BuildUnit*, HLHPlanData > *hlhPlans, int time );
      void TaskScheduler::findCandidateMorphTimes( std::map< BuildUnit*, HLHPlanData > *hlhPlans, int validBuildTimeSince );
      BuildEventTimeline* m_timeline;
      BuildUnitManager* m_buildUnitManager;
      static TaskScheduler* s_taskScheduler;
  };
}