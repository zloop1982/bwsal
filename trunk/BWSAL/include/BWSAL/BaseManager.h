#pragma once
#include <map>
#include <set>
#include <BWTA.h>
#include <BWSAL/Arbitrator.h>
namespace BWSAL
{
  class Base;
  class BorderManager;
  class BaseManager : public BWAPI::AIModule
  {
    public:
      static BaseManager* create( BorderManager* borderManager );
      static BaseManager* getInstance();
      static void destroy();
      virtual void onFrame();
      virtual void onUnitDestroy( BWAPI::Unit* unit );
      Base* getBase( BWTA::BaseLocation* location );
      const std::set< Base* >& getActiveBases() const;
      const std::set< Base* >& getReadyBases() const;
      const std::set< Base* >& getAllBases() const;
      const std::set< Base* >& getDestroyedBases() const;
      std::string getName() const;

    private:
      BaseManager();
      ~BaseManager();
      std::map< BWTA::BaseLocation*, Base* > m_location2base;
      std::set< Base* > m_activeBases;
      std::set< Base* > m_readyBases;
      std::set< Base* > m_allBases;
      std::set< Base* > m_destroyedBases;
      BorderManager* m_borderManager;
      static BaseManager* s_baseManager;
  };
}