#pragma once
#include <list>
#include <BWAPI.h>
#include <BWSAL/Task.h>
namespace BWSAL
{
  class Task;
  class BuildOrderManager;
  class MacroTask
  {
    friend class BuildOrderManager;
    public:
      const std::list< Task* >& getTasks() const { return m_tasks; }
      BuildType getType() const { return m_type; }
      bool isAdditional() const { return m_isAdditional; }
      bool isCompleted() const { return m_isCompleted; }
      int getCount() const { return m_count; }
      int getRemainingCount() const { return m_remainingCount; }
      BWAPI::TilePosition getSeedLocation() const { return m_seedLocation; }
      int getPriority() const { return m_priority; }
    private:
      MacroTask(BuildType type, int priority, bool isAdditional, int count, BWAPI::TilePosition seedLocation = BWAPI::TilePositions::None )
        : m_type( type ),
          m_priority( priority ),
          m_isAdditional( isAdditional ),
          m_count( count ),
          m_isCompleted( false ),
          m_seedLocation( seedLocation ),
          m_remainingCount( count )
      {
        if ( isAdditional )
        {
          for( int i = 0; i < count; i++ )
          {
            Task* t = new Task( type );
            t->setSeedLocation( seedLocation );
            m_tasks.push_back( t );
          }
        }
      }
      std::list< Task* > m_tasks;
      BuildType m_type;
      bool m_isAdditional;
      bool m_isCompleted;
      int m_count;
      int m_remainingCount;
      int m_priority;
      BWAPI::TilePosition m_seedLocation;
  };
}