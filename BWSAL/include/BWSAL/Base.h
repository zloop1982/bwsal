#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <BWSAL/BaseManager.h>
#include <set>
namespace BWSAL
{
  class Base
  {
    friend BaseManager;
    public:
      BWTA::BaseLocation* getBaseLocation() const;
      BWAPI::Unit* getResourceDepot() const;
      BWAPI::Unit* getRefinery() const;
      const std::set< BWAPI::Unit* >& getMinerals() const;
      const std::set< BWAPI::Unit* >& getGeysers() const;
      void setPaused( bool paused );
      bool isPaused() const;
      bool isReady() const;
      bool isActive() const;
      void update();
      void onUnitDestroy( BWAPI::Unit* u );

    protected:
      Base( BWTA::BaseLocation* b, BWAPI::Unit* resourceDepot = NULL );
      BWTA::BaseLocation* m_baseLocation;
      BWAPI::Unit* m_resourceDepot;
      BWAPI::Unit* m_refinery;
      bool m_ready;
      bool m_paused;
  };
}