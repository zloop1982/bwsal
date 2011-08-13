#include <BWSAL/BuildOrderManager.h>
#include <BWSAL/BuildUnitManager.h>
#include <BWSAL/Task.h>
#include <BWSAL/TaskScheduler.h>
#include <BWSAL/TaskExecutor.h>
#include <BWSAL/BuildState.h>
#include <BWSAL/MacroTask.h>
#include <BWSAL/BuildUnit.h>
#include <Util/Foreach.h>
#include <BWSAL/Util.h>
namespace BWSAL
{
  BuildOrderManager* BuildOrderManager::s_buildOrderManager = NULL;
  BuildOrderManager* BuildOrderManager::create( TaskScheduler* taskScheduler, TaskExecutor* taskExecutor, BuildUnitManager* buildUnitManager )
  {
    if ( s_buildOrderManager )
    {
      return s_buildOrderManager;
    }
    s_buildOrderManager = new BuildOrderManager();
    s_buildOrderManager->m_taskScheduler = taskScheduler;
    s_buildOrderManager->m_taskExecutor = taskExecutor;
    s_buildOrderManager->m_buildUnitManager = buildUnitManager;
    return s_buildOrderManager;
  }

  BuildOrderManager* BuildOrderManager::getInstance()
  {
    return s_buildOrderManager;
  }

  void BuildOrderManager::destroy()
  {
    if ( s_buildOrderManager )
    {
      delete s_buildOrderManager;
    }
  }

  BuildOrderManager::BuildOrderManager()
  {
  }

  BuildOrderManager::~BuildOrderManager()
  {
    s_buildOrderManager = NULL;
  }

  void BuildOrderManager::onFrame()
  {
    std::map< BuildType, int > totalScheduledUnitCount;
    for each( BuildType t in BuildTypes::allBuildTypes( BWAPI::Broodwar->self()->getRace() ) )
    {
      if ( t.isUnitType() )
      {
        totalScheduledUnitCount[t] = BWAPI::Broodwar->self()->completedUnitCount( t.getUnitType() ) + m_taskExecutor->getRunningCount( t );
      }
    }

    Heap< MacroTask*, int> macroTaskHeap;
    bool supplyIncrease = true;
    while ( supplyIncrease )
    {
      supplyIncrease = false;
      for ( PMTMap::iterator pmt = m_prioritizedMacroTasks.begin(); pmt != m_prioritizedMacroTasks.end() && supplyIncrease == false; pmt++ )
      {
        foreach( MacroTask* mt, pmt->second.techAndUpgradeMacroTasks )
        {
          foreach( Task* t, mt->m_tasks)
          {
            if ( t->isWaiting() && t->isScheduledThisFrame() == false )
            {
              m_taskScheduler->scheduleTask( t );
              if ( t->getRunTime() <= BWAPI::Broodwar->getFrameCount() )
              {
                m_taskExecutor->run( t );
              }
            }
          }
        }
        macroTaskHeap.clear();
        foreach( MacroTask* mt, pmt->second.unitMacroTasks )
        {
          int unscheduledTaskCount = 0;
          foreach( Task* t, mt->m_tasks )
          {
            if ( t->isWaiting() && !t->isScheduledThisFrame() )
            {
              unscheduledTaskCount++;
            }
          }
          if ( mt->isAdditional() )
          {
            mt->m_remainingCount = unscheduledTaskCount;
          }
          else
          {
            mt->m_remainingCount = mt->getCount() - totalScheduledUnitCount[mt->getType()];
            if ( unscheduledTaskCount > mt->m_remainingCount )
            {
              std::list< Task* >::iterator i = mt->m_tasks.begin();
              std::list< Task* >::iterator i2 = i;
              for( ; i != mt->m_tasks.end(); i = i2 )
              {
                i2++;
                if ( !(*i)->isScheduledThisFrame() && (*i)->isWaiting() )
                {
                  mt->m_tasks.erase( i );
                  unscheduledTaskCount--;
                  if ( unscheduledTaskCount <= mt->m_remainingCount )
                  {
                    break;
                  }
                }
              }
            }
            else
            {
              while ( unscheduledTaskCount < mt->m_remainingCount )
              {
                Task* t = new Task( mt->getType() );
                t->setSeedLocation( mt->getSeedLocation() );
                mt->m_tasks.push_back( t );
                unscheduledTaskCount++;
              }
            }
          }
          if ( mt->m_remainingCount > 0 )
          {
            macroTaskHeap.set( mt, mt->m_remainingCount );
          }
        }
        if ( macroTaskHeap.empty() == false )
        {
          while ( macroTaskHeap.top().second > 0 )
          {
            MacroTask* mt = macroTaskHeap.top().first;
            Task* st = NULL;
            foreach( Task* t, mt->m_tasks)
            {
              if ( t->isWaiting() && t->isScheduledThisFrame() == false )
              {
                st = t;
                break;
              }
            }
            if ( st != NULL )
            {
              m_taskScheduler->scheduleTask( st );
              if ( st->isScheduledThisFrame() )
              {
                totalScheduledUnitCount[ st->getBuildType() ]++;
              }
              int newCount = macroTaskHeap.top().second - 1;
              macroTaskHeap.set( mt, newCount );
              if ( st->getRunTime() <= BWAPI::Broodwar->getFrameCount() )
              {
                m_taskExecutor->run( st );
              }
              if ( st->getRunTime() != NEVER && st->getBuildType().supplyProvided() > 0 )
              {
                supplyIncrease = true;
                break;
              }
            }
            else
            {
              int newCount = 0;
              macroTaskHeap.set( mt, newCount );
            }
          }
        }
      }
    }
  }

  MacroTask* BuildOrderManager::build( int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedLocation )
  {
    if ( seedLocation == BWAPI::TilePositions::None )
    {
      seedLocation = BWAPI::Broodwar->self()->getStartLocation();
    }
    MacroTask* mt = new MacroTask( BuildType( t ), priority, false, count, seedLocation );
    m_prioritizedMacroTasks[priority].unitMacroTasks.push_back( mt );
    return mt;
  }

  MacroTask* BuildOrderManager::buildAdditional( int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedLocation )
  {
    if ( seedLocation == BWAPI::TilePositions::None )
    {
      seedLocation = BWAPI::Broodwar->self()->getStartLocation();
    }
    MacroTask* mt = new MacroTask( BuildType( t ), priority, true, count, seedLocation );
    m_prioritizedMacroTasks[priority].unitMacroTasks.push_back( mt );
    return mt;
  }

  MacroTask* BuildOrderManager::research( BWAPI::TechType t, int priority )
  {
    MacroTask* mt = new MacroTask( BuildType( t ), priority, true, 1 );
    m_prioritizedMacroTasks[priority].techAndUpgradeMacroTasks.push_back( mt );
    return mt;
  }

  MacroTask* BuildOrderManager::upgrade( int level, BWAPI::UpgradeType t, int priority )
  {
    MacroTask* mt = new MacroTask( BuildType( t, level ), priority, true, 1 );
    m_prioritizedMacroTasks[priority].techAndUpgradeMacroTasks.push_back( mt );
    return mt;
  }

  void BuildOrderManager::deleteMacroTask( MacroTask* mt )
  {
    // Sanity check
    if ( mt == NULL )
    {
      return;
    }
    // TODO: Implement
  }

  void BuildOrderManager::draw( int x, int y )
  {
    y -= 16;
    for ( PMTMap::iterator pmt = m_prioritizedMacroTasks.begin(); pmt != m_prioritizedMacroTasks.end(); pmt++ )
    {
      foreach( MacroTask* mt, pmt->second.techAndUpgradeMacroTasks )
      {
        foreach( Task* t, mt->m_tasks)
        {
          if ( t->getCompletionTime() > BWAPI::Broodwar->getFrameCount() - 4 * 24 )
          {
            BWAPI::Broodwar->drawTextScreen( x, y += 16, "[ %d ] Task: %s, S = %s, RT = %d",
            pmt->first,
            t->getBuildType().getName().c_str(),
            t->getState().getName().c_str(),
            t->getRunTime() );
          }
        }
      }
      foreach( MacroTask* mt, pmt->second.unitMacroTasks )
      {
        foreach( Task* t, mt->m_tasks)
        {
          if ( t->getCompletionTime() > BWAPI::Broodwar->getFrameCount() - 4 * 24 )
          {
            BWAPI::Broodwar->drawTextScreen( x, y += 16, "[ %d ] Task: %s, S = %s, RT = %d",
            pmt->first,
            t->getBuildType().getName().c_str(),
            t->getState().getName().c_str(),
            t->getRunTime() );
          }
        }
      }
    }
  }
}