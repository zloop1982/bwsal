#include <BWSAL/TaskScheduler.h>
#include <BWSAL/Task.h>
#include <BWSAL/BuildState.h>
#include <BWSAL/BuildUnit.h>
#include <BWSAL/BuildUnitManager.h>
#include <BWSAL/MetaUnitVariable.h>
#include <BWSAL/BuildEventTimeline.h>
#include <BWSAL/Util.h>
#include <Util/Foreach.h>
#include <list>
#include <BWAPI.h>
namespace BWSAL
{
  TaskScheduler* TaskScheduler::s_taskScheduler = NULL;
  TaskScheduler* TaskScheduler::create( BuildEventTimeline* timeline, BuildUnitManager* buildUnitManager )
  {
    if ( s_taskScheduler )
    {
      return s_taskScheduler;
    }
    s_taskScheduler = new TaskScheduler();
    s_taskScheduler->m_timeline = timeline;
    s_taskScheduler->m_buildUnitManager = buildUnitManager;
    return s_taskScheduler;
  }
  TaskScheduler* TaskScheduler::getInstance()
  {
    return s_taskScheduler;
  }
  void TaskScheduler::destroy()
  {
    if ( s_taskScheduler )
    {
      delete s_taskScheduler;
    }
  }
  TaskScheduler::TaskScheduler()
  {
  }
  TaskScheduler::~TaskScheduler()
  {
    s_taskScheduler = NULL;
  }
  void TaskScheduler::scheduleTask( Task* t, BuildUnit* builder, int runTime )
  {
    t->m_builder->assign( ( MetaUnit* )builder );
    t->setRunTime( runTime );
    t->setScheduledThisFrame();
    t->setState( TaskStates::Tentatively_Scheduled );

    int executeTime = runTime + t->getBuildType().prepTime();
    int builderReleaseTime = executeTime + t->getBuildType().builderTime();
    int completeTime = executeTime + t->getBuildType().buildUnitTime();
    std::list< std::pair< int, BuildEvent > >::iterator timelineIter = m_timeline->begin();
    if ( t->getBuildType().requiresLarva() )
    {
      BuildEvent e( t->getBuildType() );
      e.useLarva( builder );
      timelineIter = m_timeline->addEvent( runTime, e, timelineIter );
    }
    else
    {
      timelineIter = m_timeline->addEvent( runTime, t->getReserveBuilderEvent(), timelineIter );
    }
    timelineIter = m_timeline->addEvent( executeTime, t->getReserveResourcesEvent(), timelineIter );
    if ( t->getBuildType().requiresLarva() == false )
    {
      timelineIter = m_timeline->addEvent( builderReleaseTime, t->getReleaseBuilderEvent(), timelineIter );
    }
    timelineIter = m_timeline->addEvent( completeTime, t->getCompleteBuildTypeEvent(), timelineIter );
  }

  inline bool TaskScheduler::canCompleteWithUnitBeforeNextEvent( int validBuildTypeSince,
                                                                 BuildUnit* unit,
                                                                 const Task* t,
                                                                 const std::list< std::pair< int, BuildEvent > >::const_iterator nextEvent )
  {
    bool found = t->m_useAnyBuilder;
    if ( !found )
    {
      foreach( MetaUnit* mu, t->m_possibleBuilders )
      {
        if ( mu->getBuildUnit() == unit )
        {
          found = true;
          break;
        }
      }
    }
    if ( !found )
    {
      // Can't use this build unit
      return false;
    }
    int availableSince = unit->m_planningData.m_availableSince;
    if ( availableSince != NEVER )
    {
      int buildTime = max( t->getBuildType().builderTime(), t->getBuildType().buildUnitTime() );
      int canBuildSince = max( availableSince, validBuildTypeSince - t->getBuildType().prepTime() );
      if ( nextEvent == m_timeline->end() || canBuildSince + buildTime < nextEvent->first )
      {
        return true;
      }
    }
    return false;
  }
  TaskScheduler::HLHPlanData::HLHPlanData()
  {
    candidateNextLarvaSpawnTime = NEVER;
    candidateLarvaCount = 0;
    candidateMorphTime = NEVER;
    candidateMorphed = false;
  }
  void TaskScheduler::resetCandidates( std::map< BuildUnit*, HLHPlanData > *hlhPlans, BuildState* state )
  {
    for ( std::map< BuildUnit*, HLHPlanData >::iterator h = hlhPlans->begin(); h != hlhPlans->end(); h++ )
    {
      h->second.candidateNextLarvaSpawnTime = h->first->m_planningData.m_nextLarvaSpawnTime;
      h->second.candidateLarvaCount = h->first->m_planningData.m_larvaCount;
      h->second.candidateMorphTime = NEVER;
      h->second.candidateMorphed = false;
    }
  }
  void TaskScheduler::initializeHLHPlanData(std::map< BuildUnit*, HLHPlanData > *hlhPlans )
  {
    // Initialize HLH data
    foreach( BuildUnit* bu, m_buildUnitManager->getUnits() )
    {
      if ( bu->getType().getUnitType().producesLarva() )
      {
        ( *hlhPlans )[bu].candidateNextLarvaSpawnTime = bu->m_currentState.m_nextLarvaSpawnTime;
        ( *hlhPlans )[bu].candidateLarvaCount = bu->m_currentState.m_larvaCount;
      }
    }
  }
  void TaskScheduler::continueToTimeWithLarvaSpawns( BuildState* state, std::map< BuildUnit*, HLHPlanData > *hlhPlans, int time )
  {
    // Continue build state to current time
    state->continueToTime( time );

    // Update HLHPlanData for each HLH
    for ( std::map< BuildUnit*, HLHPlanData >::iterator h = hlhPlans->begin(); h != hlhPlans->end(); h++ )
    {
      // if its time to test the candidate morph
      if ( h->second.candidateMorphTime < time && h->second.candidateMorphed == false )
      {
        while ( h->second.candidateNextLarvaSpawnTime <= h->second.candidateMorphTime )
        {
          h->second.candidateLarvaCount++;

          // If we have three larva, next larva NEVER spawns
          if ( h->second.candidateLarvaCount == 3 )
          {
            h->second.candidateNextLarvaSpawnTime = NEVER;
            break;
          }
          else
          {
            h->second.candidateNextLarvaSpawnTime += LARVA_SPAWN_TIME;
          }
        }

        // try our candidate morph time
        h->second.candidateLarvaCount--;
        h->second.candidateMorphed = true;

        // detect invalid time line
        if ( h->second.candidateLarvaCount < 0 )
        {
          // revert to existing timeline
          h->second.candidateLarvaCount = h->first->m_planningData.m_larvaCount;
          h->second.candidateNextLarvaSpawnTime = h->first->m_planningData.m_nextLarvaSpawnTime;

          // assume we can morph with the next larva
          h->second.candidateMorphTime = h->first->m_planningData.m_nextLarvaSpawnTime;
          h->second.candidateMorphed = false;
        }
      }

      // Continue candidate timeline to current time
      while ( h->second.candidateNextLarvaSpawnTime <= state->getTime() )
      {
        h->second.candidateLarvaCount++;

        // We have three larva, next larva NEVER spawns
        if ( h->second.candidateLarvaCount == 3 )
        {
          h->second.candidateNextLarvaSpawnTime = NEVER;
          break;
        }
        else
        {
          h->second.candidateNextLarvaSpawnTime += LARVA_SPAWN_TIME;
        }
      }
    }
  }
  void TaskScheduler::findCandidateMorphTimes( std::map< BuildUnit*, HLHPlanData > *hlhPlans, int validBuildTimeSince )
  {  
    for ( std::map< BuildUnit*, HLHPlanData >::iterator h = hlhPlans->begin(); h != hlhPlans->end(); h++ )
    {
      // if we don't yet have a candidate morph time
      if ( h->second.candidateMorphTime == NEVER )
      {
        if ( h->first->m_planningData.m_larvaCount > 0 )
        {
          h->second.candidateMorphTime = validBuildTimeSince;
          h->second.candidateLarvaCount = h->first->m_planningData.m_larvaCount;
          h->second.candidateNextLarvaSpawnTime = h->first->m_planningData.m_nextLarvaSpawnTime;
          if ( h->first->m_planningData.m_larvaCount == 3 ) // Decreasing from 3 larva - start making the next larva
          {
            h->second.candidateNextLarvaSpawnTime = h->second.candidateMorphTime + LARVA_SPAWN_TIME;
          }
          h->second.candidateLarvaCount--;
          h->second.candidateMorphed = true;
        }
        else
        {
          h->second.candidateMorphTime = max( validBuildTimeSince, h->first->m_planningData.m_nextLarvaSpawnTime );
          h->second.candidateLarvaCount = h->first->m_planningData.m_larvaCount;
          h->second.candidateNextLarvaSpawnTime = h->first->m_planningData.m_nextLarvaSpawnTime;
          h->second.candidateMorphed = false;
        }
      }
    }
  }

  void TaskScheduler::scheduleLarvaUsingTask( Task* t )
  {
    // Sanity check
    if ( t == NULL || t->isScheduledThisFrame() )
    {
      return;
    }

    // State/Planning information is stored in BuildState and in the BuildUnits themselves
    BuildState state = m_timeline->m_initialState;
    m_buildUnitManager->resetPlanningData();

    BuildType buildType = t->getBuildType();
    // reset assignments and run time for this task
    t->getBuilder()->assign( NULL );
    if ( t->getSecondBuilder() != NULL )
    {
      t->getSecondBuilder()->assign( NULL );
    }
    t->setRunTime( NEVER );
    std::map< BuildUnit*, HLHPlanData > hlhPlans;

    initializeHLHPlanData( &hlhPlans );

    int validBuildTypeSince = NEVER;
    std::list< std::pair< int, BuildEvent > >::iterator nextEvent = m_timeline->begin();

    if ( state.hasEnoughSupplyAndRequiredBuildTypes( buildType ) )
    {
      if ( state.getMinerals() >= buildType.mineralPrice() && state.getGas() >= buildType.gasPrice() )
      {
        validBuildTypeSince = state.getTime();
      }
      else
      {
        int nextSatisfiedTime = state.getNextTimeWithMinimumResources( buildType. mineralPrice(), buildType.gasPrice() );
        if ( nextEvent == m_timeline->end() || nextSatisfiedTime < nextEvent->first )
        {
          state.continueToTime( nextSatisfiedTime );
          validBuildTypeSince = state.getTime();
        }
      }
    }
    if ( validBuildTypeSince != NEVER )
    {
      findCandidateMorphTimes( &hlhPlans, validBuildTypeSince );
    }
    // Iterate over the timeline of events
    while ( nextEvent != m_timeline->end() )
    {
      continueToTimeWithLarvaSpawns( &state, &hlhPlans, nextEvent->first );
      state.doEvent( nextEvent->second );

      // Handle use larva events
      if ( nextEvent->second.getUseLarva() != NULL )
      {
        BuildUnit* h = nextEvent->second.getUseLarva();
        if ( hlhPlans[h].candidateLarvaCount == 3 )
        {
          hlhPlans[h].candidateNextLarvaSpawnTime = state.getTime() + LARVA_SPAWN_TIME;
        }
        hlhPlans[h].candidateLarvaCount--;
        if ( hlhPlans[h].candidateLarvaCount < 0 )
        {
          hlhPlans[h].candidateMorphTime = h->m_planningData.m_larvaCount;
          hlhPlans[h].candidateNextLarvaSpawnTime = h->m_planningData.m_nextLarvaSpawnTime;
          hlhPlans[h].candidateLarvaCount = h->m_planningData.m_larvaCount;
          hlhPlans[h].candidateMorphed = false;
        }
      }
      nextEvent++;
      if ( !state.hasEnoughSupplyAndRequiredBuildTypes( buildType ) )
      {
        validBuildTypeSince = NEVER;
        // Throw away our candidate solutions
        resetCandidates( &hlhPlans, &state );
        continue;
      }
      // If we don't have enough minerals or gas on this event
      if ( state.getMinerals() < buildType.mineralPrice() || state.getGas() < buildType.gasPrice() )
      {
        validBuildTypeSince = NEVER;
        // Throw away our candidate solutions
        resetCandidates( &hlhPlans, &state );

        // See if we will have enough minerals and gas before the next event
        int nextSatisfiedTime = state.getNextTimeWithMinimumResources( buildType.mineralPrice(), buildType.gasPrice() );
        if ( nextEvent == m_timeline->end() || nextSatisfiedTime < nextEvent->first )
        {
          if ( nextSatisfiedTime == NEVER )
          {
            // Right now it doesn't look like we'll ever have enough resources
            // But continue to the next event. Who knows, maybe we've planned a worker or refinery
            continue;
          }
          // If so, continue to that point and time and update validBuildTypeSince
          continueToTimeWithLarvaSpawns( &state, &hlhPlans, nextSatisfiedTime );
        }
        else
        {
          // Otherwise, we will need to continue processing events until we find a valid time
          continue;
        }
      }
      // If we get to here, we have a valid build time, so set it if its not set
      if ( validBuildTypeSince == NEVER )
      {
        validBuildTypeSince = state.getTime();
      }
      findCandidateMorphTimes( &hlhPlans, validBuildTypeSince );
    }

    // Pick the candidate with the earliest morph time to schedule with
    BuildUnit* candidateUnit = NULL;
    int candidateTime = NEVER;
    for ( std::map< BuildUnit*, HLHPlanData >::iterator h = hlhPlans.begin(); h != hlhPlans.end(); h++ )
    {
      if ( h->second.candidateMorphTime < candidateTime )
      {
        candidateTime = h->second.candidateMorphTime;
        candidateUnit = h->first;
      }
    }
    if ( candidateUnit != NULL )
    {
      scheduleTask( t, candidateUnit, candidateTime );
    }
  }
  void TaskScheduler::scheduleTask( Task* t )
  {
    // Sanity check
    if ( t == NULL || t->isScheduledThisFrame() )
    {
      return;
    }

    // Tasks that use larva are very different from other tasks
    if ( t->getBuildType().requiresLarva() )
    {
      scheduleLarvaUsingTask( t );
      return;
    }

    // State/Planning information is stored in BuildState and in the BuildUnits themselves
    BuildState state = m_timeline->m_initialState;
    m_buildUnitManager->resetPlanningData();

    BuildType buildType = t->getBuildType();
    BuildType builderType = buildType.whatBuilds().first;


    int buildTime = buildType.buildUnitTime();

    // Reset assignments and run time for this task
    t->getBuilder()->assign( NULL );
    if ( t->getSecondBuilder() != NULL )
    {
      t->getSecondBuilder()->assign( NULL );
    }
    t->setRunTime( NEVER );

    int validBuildTypeSince = NEVER;
    std::list< std::pair< int, BuildEvent > >::iterator nextEvent = m_timeline->begin();

    // The set of units that can execute this task
    // ( except in the case of add - ons, but we take care of that later )

    BuildUnit* candidateUnit = NULL;
    int candidateTime = NEVER;


    if ( state.hasEnoughSupplyAndRequiredBuildTypes( buildType ) )
    {
      if ( state.getMinerals() >= buildType.mineralPrice() && state.getGas() >= buildType.gasPrice() )
      {
        validBuildTypeSince = state.getTime();
      }
      else
      {
        int nextSatisfiedTime = state.getNextTimeWithMinimumResources( buildType.mineralPrice(), buildType.gasPrice() );
        if ( nextEvent == m_timeline->end() || nextSatisfiedTime < nextEvent->first )
        {
          state.continueToTime( nextSatisfiedTime );
          validBuildTypeSince = state.getTime();
        }
      }
    }
    if ( validBuildTypeSince != NEVER )
    {
      foreach ( BuildUnit* bu, m_buildUnitManager->getUnits() )
      {
        if ( bu->m_planningData.m_type == t->getBuildType().whatBuilds().first &&
             canCompleteWithUnitBeforeNextEvent( validBuildTypeSince, bu, t, nextEvent ) )
        {
          int availableSince = bu->m_planningData.m_availableSince;
          int canBuildSince = max( availableSince, validBuildTypeSince - t->getBuildType().prepTime() );
          if ( canBuildSince < candidateTime && canBuildSince < NEVER )
          {
            candidateUnit = bu;
            candidateTime = canBuildSince;
          }
        }
      }
    }
    // Iterate over the timeline of events
    while ( nextEvent != m_timeline->end() )
    {
      state.continueToTime( nextEvent->first );
      state.doEvent( nextEvent->second );
      nextEvent++;
      if ( !state.hasEnoughSupplyAndRequiredBuildTypes( buildType ) )
      {
        validBuildTypeSince = NEVER;
        candidateUnit = NULL;
        candidateTime = NEVER;
        continue;
      }
      // Take into accout the fact that pulling a worker off minerals to build something will
      // reduce the amount of minerals we're going to have because that worker is not mining
      double candidateMinerals = state.getMinerals();
      if ( candidateTime != NEVER )
      {
        if ( state.getTime() > candidateTime + buildType.prepTime() + buildType.builderTime() && buildType.getRace() != BWAPI::Races::Zerg )
        {
          candidateMinerals -= ( buildType.prepTime() + buildType.builderTime() ) * MINERALS_PER_WORKER_PER_FRAME;
        }
        else
        {
          candidateMinerals -= ( state.getTime() - candidateTime ) * MINERALS_PER_WORKER_PER_FRAME;
        }
      }
      // If we don't have enough minerals or gas on this event
      if ( candidateMinerals < buildType.mineralPrice() || state.getGas() < buildType.gasPrice() )
      {
        // Throw away our candidate unit
        candidateUnit = NULL;
        candidateTime = NEVER;
        validBuildTypeSince = NEVER;

        // See if we will have enough minerals and gas before the next event
        int nextSatisfiedTime = state.getNextTimeWithMinimumResources( buildType.mineralPrice(), buildType.gasPrice() );
        if ( nextEvent == m_timeline->end() || nextSatisfiedTime < nextEvent->first )
        {
          if ( nextSatisfiedTime == NEVER )
          {
            // Right now it doesn't look like we'll ever have enough resources
            // But continue to the next event. Who knows, maybe we've planned a worker or refinery
            continue;
          }
          // if so, continue to that point and time and update validBuildTypeSince
          state.continueToTime( nextSatisfiedTime );
        }
        else
        {
          // Otherwise, we will need to continue processing events until we find a valid time
          continue;
        }
      }

      // If we get to here, we have a valid build time, so set it if its not set
      if ( validBuildTypeSince == NEVER )
      {
        validBuildTypeSince = state.getTime();
      }
      if ( candidateUnit == NULL )
      {
        // We need to find a candidate unit
        foreach ( BuildUnit* bu, m_buildUnitManager->getUnits() )
        {
          if ( bu->m_planningData.m_type == t->getBuildType().whatBuilds().first )
          {
            // Filter by addon if needed
            if ( buildType.requiredAddon() == BWAPI::UnitTypes::None || buildType.requiredAddon() == bu->m_planningData.m_addon )
            {
              if ( canCompleteWithUnitBeforeNextEvent( validBuildTypeSince, bu, t, nextEvent ) )
              {
                int availableSince = bu->m_planningData.m_availableSince;
                int canBuildSince = max( availableSince, validBuildTypeSince - t->getBuildType().prepTime() );
                if ( canBuildSince < candidateTime && canBuildSince < NEVER )
                {
                  candidateUnit = bu;
                  candidateTime = canBuildSince;
                }
              }
            }
          }
        }
      }
    }
    // Now that we've traversed the entire timeline we can safely schedule our task if we still have a candidate
    if ( candidateUnit != NULL )
    {
      scheduleTask( t, candidateUnit, candidateTime );
    }
  }
}