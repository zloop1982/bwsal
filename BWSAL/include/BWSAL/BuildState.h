#pragma once
#include <BWSAL/BuildEvent.h>
#include <BWSAL/Types.h>
#include <map>
namespace BWSAL
{
  class WorkerManager;
  class BuildUnit;
  class TaskExecutor;
  class TaskScheduler;
  class BuildState
  {
    friend class TaskExecutor;
    friend class TaskScheduler;
    public:
      BuildState();
      int getNextTimeWithMinimumResources( int minerals, int gas );
      bool hasEnoughSupplyAndRequiredBuildTypes( BuildType buildType );

      void continueToTime( int time );
      void doEvent( BuildEvent& e );
      void updateWithCurrentGameState();
      void createUnclaimedBuildUnits();
      int getTime() const;
      double getMinerals() const;
      double getGas() const;
      int getSupply() const;
      int getMineralWorkers() const;
      int getGasWorkers() const;
      int reservedMinerals;
      int reservedGas;
    private:
      unsigned int m_completedBuildTypes;
      int m_time;
      double m_minerals;
      double m_gas;
      int m_supply;
      int m_mineralWorkers;
      int m_gasWorkers;
  };
}