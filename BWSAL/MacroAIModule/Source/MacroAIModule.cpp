#include "MacroAIModule.h"
#include <BWSAL/Util.h>
#include <Util/Foreach.h>
#include <algorithm>
using namespace BWSAL;
using namespace BWAPI;
using namespace std;
MacroAIModule::MacroAIModule()
{
}

MacroAIModule::~MacroAIModule()
{
}

void MacroAIModule::onStart()
{
  Broodwar->enableFlag( Flag::UserInput );
  BWTA::readMap();
  BWTA::analyze();
  BWSAL::resetLog();

  m_informationManager = InformationManager::create();
  m_borderManager = BorderManager::create( m_informationManager );
  m_baseManager = BaseManager::create( m_borderManager );
  m_buildingPlacer = new BFSBuildingPlacer();
  m_reservedMap = ReservedMap::create();
  m_unitArbitrator = new UnitArbitrator();
  m_unitGroupManager = UnitGroupManager::create();
  m_buildUnitManager = BuildUnitManager::create();
  m_buildEventTimeline = BuildEventTimeline::create( m_buildUnitManager );
  m_taskScheduler = TaskScheduler::create( m_buildEventTimeline, m_buildUnitManager );
  m_taskExecutor = TaskExecutor::create( m_unitArbitrator, m_buildEventTimeline, m_reservedMap, m_buildingPlacer );
  m_workerManager = WorkerManager::create( m_unitArbitrator, m_baseManager );
  m_scoutManager = ScoutManager::create( m_unitArbitrator, m_informationManager );
  m_defenseManager = DefenseManager::create( m_unitArbitrator, m_borderManager );
  m_buildOrderManager = BuildOrderManager::create( m_taskScheduler, m_taskExecutor, m_buildUnitManager );
  m_enhancedUI = new EnhancedUI();

  m_buildEventTimeline->initialize();
  m_scoutManager->initialize();

  m_modules.push_back( m_informationManager );
  m_modules.push_back( m_borderManager );
  m_modules.push_back( m_baseManager );
  m_modules.push_back( m_buildEventTimeline );
  m_modules.push_back( m_reservedMap );
  m_modules.push_back( m_unitGroupManager );
  m_modules.push_back( m_buildUnitManager );
  m_modules.push_back( m_taskScheduler );
  m_modules.push_back( m_taskExecutor );
  m_modules.push_back( m_workerManager );
  m_modules.push_back( m_scoutManager );
  m_modules.push_back( m_defenseManager );

  BWAPI::Race race = Broodwar->self()->getRace();
  BWAPI::Race enemyRace = Broodwar->enemy()->getRace();
  double minDist;
  BWTA::BaseLocation* natural = NULL;
  BWTA::BaseLocation* home = BWTA::getStartLocation( Broodwar->self() );
  foreach( BWTA::BaseLocation* bl, BWTA::getBaseLocations() )
  {
    if ( bl != home )
    {
      double dist = home->getGroundDistance( bl );
      if ( dist > 0 )
      {
        if ( natural == NULL || dist < minDist)
        {
          minDist = dist;
          natural = bl;
        }
      }
    }
  }

  if ( race == Races::Zerg )
  {
    // Send an overlord out if Zerg
    m_scoutManager->setScoutCount( 1 );
  }
  else if ( race == Races::Terran )
  {
    m_buildOrderManager->build( 20, UnitTypes::Terran_SCV, 80 );
    m_buildOrderManager->buildAdditional( 1, UnitTypes::Terran_Barracks, 60 );
    m_buildOrderManager->buildAdditional( 9, UnitTypes::Terran_Marine, 45 );
    m_buildOrderManager->buildAdditional( 1, UnitTypes::Terran_Refinery, 42 );
    m_buildOrderManager->buildAdditional( 1, UnitTypes::Terran_Barracks, 40 );
    m_buildOrderManager->buildAdditional( 1, UnitTypes::Terran_Academy, 39 );
    m_buildOrderManager->buildAdditional( 9, UnitTypes::Terran_Medic, 38 );
    m_buildOrderManager->build( 3, UnitTypes::Terran_Supply_Depot, 30 );
  }
  m_drawTasks = true;
  m_drawAssignments = true;
  m_drawResources = true;
  m_drawLarva = Broodwar->self()->getRace() == Races::Zerg;
}

void MacroAIModule::onEnd( bool isWinner )
{
}

void MacroAIModule::onFrame()
{
  if ( Broodwar->isPaused() )
  {
    return;
  }
  m_unitArbitrator->update();
  m_buildEventTimeline->reset();

  foreach( BWAPI::AIModule* m, m_modules )
  {
    m->onFrame();
  }
  m_enhancedUI->update();
  m_borderManager->draw();
  if ( Broodwar->getFrameCount() > 2 * 24 * 60 )
  {
    m_scoutManager->setScoutCount( 1 );
  }

  m_buildEventTimeline->m_initialState.createUnclaimedBuildUnits();

  m_buildOrderManager->onFrame();

  if ( m_drawResources )
  {
    m_buildEventTimeline->draw();
  }
  if ( m_drawLarva )
  {
    m_buildEventTimeline->drawLarvaCounts();
  }
  int y = -16;
  Broodwar->drawTextScreen( 0, y += 16, "Time: %d, Minerals: %f, Gas: %f", m_buildEventTimeline->m_initialState.getTime(), m_buildEventTimeline->m_initialState.getMinerals(), m_buildEventTimeline->m_initialState.getGas() );
  y = 50 - 16;
  /*
  if ( m_drawTasks )
  {
    foreach( Task* t, tasks )
    {
      if ( t->getCompletionTime() >= Broodwar->getFrameCount() - 24 * 10 )
      {
        Broodwar->drawTextScreen( 0, y += 16, "Task: %s, S = %s, RT = %d",
        t->getBuildType().getName().c_str(),
        t->getState().getName().c_str(),
        t->getRunTime() );
      }
    }
  }*/

  std::set< Unit* > units = Broodwar->self()->getUnits();
  if ( m_drawAssignments )
  {
    foreach( Unit* i, units )
    {
      if ( m_unitArbitrator->hasBid( i ) )
      {
        int x = i->getPosition().x();
        int y = i->getPosition().y();
        std::list< std::pair< UnitController*, double > > bids = m_unitArbitrator->getAllBidders( i );
        int y_off = 0;
        bool first = false;
        const char activeColor = '\x07', inactiveColor = '\x16';
        char color = activeColor;
        for ( std::list< std::pair< UnitController*, double > >::iterator j = bids.begin(); j != bids.end(); j++ )
        {
          Broodwar->drawTextMap( x, y + y_off, "%c%s: %d", color, j->first->getName().c_str(), (int)j->second );
          y_off += 15;
          color = inactiveColor;
        }
      }
    }
  }
}

void MacroAIModule::onSendText( std::string text )
{
  foreach( BWAPI::AIModule* m, m_modules )
  {
    m->onSendText( text );
  }
  if ( text == "t")
  {
    m_drawTasks = !m_drawTasks;
  }
  else if ( text == "a")
  {
    m_drawAssignments = !m_drawAssignments;
  }
  else if ( text == "r")
  {
    m_drawResources = !m_drawResources;
  }
  else if ( text == "l")
  {
    m_drawLarva = !m_drawLarva;
  }
}

void MacroAIModule::onUnitDiscover( BWAPI::Unit* unit )
{
  foreach( BWAPI::AIModule* m, m_modules )
  {
    m->onUnitDiscover( unit );
  }
}

void MacroAIModule::onUnitEvade( BWAPI::Unit* unit )
{
  foreach( BWAPI::AIModule* m, m_modules )
  {
    m->onUnitEvade( unit );
  }
}

void MacroAIModule::onUnitDestroy( BWAPI::Unit* unit )
{
  m_unitArbitrator->onRemoveObject( unit );
  foreach( BWAPI::AIModule* m, m_modules )
  {
    m->onUnitDestroy( unit );
  }
}

void MacroAIModule::onUnitMorph( BWAPI::Unit* unit )
{
  foreach( BWAPI::AIModule* m, m_modules )
  {
    m->onUnitMorph( unit );
  }
}

void MacroAIModule::onUnitRenegade( BWAPI::Unit* unit )
{
  foreach( BWAPI::AIModule* m, m_modules )
  {
    m->onUnitRenegade( unit );
  }
}

void MacroAIModule::onUnitComplete( BWAPI::Unit* unit )
{
  foreach( BWAPI::AIModule* m, m_modules )
  {
    m->onUnitComplete( unit );
  }
}
