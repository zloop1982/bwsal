#include <BWSAL/BaseManager.h>
#include <BWSAL/Base.h>
#include <BWSAL/BorderManager.h>
#include <Util/Foreach.h>
#include <BWAPI.h>
namespace BWSAL
{
  BaseManager* BaseManager::s_baseManager = NULL;

  BaseManager* BaseManager::create( BorderManager* borderManager )
  {
    if ( s_baseManager )
    {
      return s_baseManager;
    }
    s_baseManager = new BaseManager();
    s_baseManager->m_borderManager = borderManager;
    return s_baseManager;
  }

  BaseManager* BaseManager::getInstance()
  {
    return s_baseManager;
  }

  void BaseManager::destroy()
  {
    if ( s_baseManager )
    {
      delete s_baseManager;
    }
  }

  BaseManager::BaseManager()
  {
  }

  BaseManager::~BaseManager()
  {
    s_baseManager = NULL;
  }

  void BaseManager::onFrame()
  {
    // check to see if any new base locations need to be added
    foreach( BWTA::BaseLocation* location, BWTA::getBaseLocations() )
    {
      if ( m_location2base.find( location ) == m_location2base.end() )
      {
        // There is no base currently associated with this base location
        // so look for a resource depot on it
        BWAPI::TilePosition tile = location->getTilePosition();
        foreach( BWAPI::Unit* u, BWAPI::Broodwar->getUnitsOnTile( tile.x(), tile.y() ) )
        {
          if ( u->getPlayer() == BWAPI::Broodwar->self() && u->getType().isResourceDepot() )
          {
            // Found a resource depot, create a base location
            Base* mb = new Base( location, u );
            m_location2base[location] = mb;
            m_allBases.insert( mb );
            m_borderManager->addMyBase( location );
            break;
          }
        }
      }
    }
    // Update each base
    foreach( Base* mb, m_allBases )
    {
      mb->update();
      if ( mb->isActive() )
      {
        m_activeBases.insert( mb );
      }
      else
      {
        m_activeBases.erase( mb );
      }
      if ( mb->isReady() )
      {
        m_readyBases.insert( mb );
      }
      else
      {
        m_readyBases.erase( mb );
      }
    }
  }

  Base* BaseManager::getBase( BWTA::BaseLocation* location )
  {
    std::map< BWTA::BaseLocation*, Base* >::iterator i = m_location2base.find( location );
    if ( i == m_location2base.end() )
    {
      return NULL;
    }
    return i->second;
  }

  const std::set< Base* >& BaseManager::getActiveBases() const
  {
    return m_activeBases;
  }

  const std::set< Base* >& BaseManager::getReadyBases() const
  {
    return m_readyBases;
  }

  const std::set< Base* >& BaseManager::getAllBases() const
  {
    return m_allBases;
  }

  const std::set< Base* >& BaseManager::getDestroyedBases() const
  {
    return m_destroyedBases;
  }

  std::string BaseManager::getName() const
  {
    return "BaseManager";
  }

  void BaseManager::onUnitDestroy( BWAPI::Unit* unit )
  {
    // Sanity check
    if ( unit == NULL )
    {
      return;
    }

    // Tell all our bases that this unit has been destroyed
    foreach( Base* b, m_allBases )
    {
      b->onUnitDestroy( unit );
    }
  }
}