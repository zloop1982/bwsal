#pragma once
#include <BWAPI.h>
#include <map>
#include <list>
namespace BWSAL
{
  class Task;
  class TaskScheduler;
  class TaskExecutor;
  class BuildUnitManager;
  class MacroTask;
  class BuildOrderManager : public BWAPI::AIModule
  {
    public:
      static BuildOrderManager* create( TaskScheduler* taskScheduler, TaskExecutor* taskExecutor, BuildUnitManager* buildUnitManager );
      static BuildOrderManager* getInstance();
      static void destroy();
      void onFrame();
      MacroTask* build( int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedLocation = BWAPI::TilePositions::None );
      MacroTask* buildAdditional( int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedLocation = BWAPI::TilePositions::None );
      MacroTask* research( BWAPI::TechType t, int priority );
      MacroTask* upgrade( int level, BWAPI::UpgradeType t, int priority );
      void deleteMacroTask( MacroTask* mt );

    private:
      class PriorityLevel
      {
        public:
          std::list< MacroTask* > techAndUpgradeMacroTasks;
          std::list< MacroTask* > unitMacroTasks;
      };
      BuildOrderManager();
      ~BuildOrderManager();
      typedef std::map< int, PriorityLevel, std::greater<int> > PMTMap;
      PMTMap m_prioritizedMacroTasks;
      TaskScheduler* m_taskScheduler;
      TaskExecutor* m_taskExecutor;
      BuildUnitManager* m_buildUnitManager;
      static BuildOrderManager* s_buildOrderManager;
  };
}