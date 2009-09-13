#include <BWTA.h>
#include <ScoutManager.h>

ScoutManager::ScoutManager(Arbitrator::Arbitrator<BWAPI::Unit*,double> *arbitrator)
{
  this->arbitrator = arbitrator;
  desiredScoutCount = 0;

  myStartLocation = BWTA::getStartLocation(BWAPI::Broodwar->self());
  std::set<BWTA::BaseLocation *> locations = BWTA::getStartLocations();
  for(std::set<BWTA::BaseLocation *>::iterator l = locations.begin(); l != locations.end(); l++)
  {
    if (myStartLocation->getGroundDistance(*l) > 0)
    {
      positionsToScout.insert((*l)->getPosition());
    }
  }
}

void ScoutManager::onOffer(std::set<BWAPI::Unit*> units)
{
  for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
  {
    if ((*u)->getType().isWorker() && needMoreScouts())
    {
      arbitrator->accept(this, *u);
      addScout(*u);
    }
    else
      arbitrator->decline(this, *u, 0);
  }
}

void ScoutManager::onRevoke(BWAPI::Unit *unit, double bid)
{
  scouts.erase(unit);
}

void ScoutManager::update()
{
  if (needMoreScouts())
  {
    requestScout(/* bid = */ 12); // Bid 12.
  }
  else
  {
    while (scouts.size()>desiredScoutCount)
    {
      arbitrator->setBid(this, scouts.begin()->first,0);
      scouts.erase(scouts.begin());
    }
  }
  updateScoutAssignments();
}

std::string ScoutManager::getName() const
{
  return "Scout Manager";
}

void ScoutManager::onRemoveUnit(BWAPI::Unit* unit)
{
  scouts.erase(unit);
}

void ScoutManager::setScoutCount(int count)
{
  this->desiredScoutCount=count;
}

bool ScoutManager::isScouting() const
{
  return scouts.size() >= 1;
}

bool ScoutManager::needMoreScouts() const
{
  return scouts.size() < desiredScoutCount;
}

void ScoutManager::requestScout(double bid)
{
  // Bid on all completed workers.
  std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
  for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); u++)
  {
    if ((*u)->isCompleted() && (*u)->getType().isWorker())
    {
      arbitrator->setBid(this, *u, bid);
    }
  }
}

void ScoutManager::addScout(BWAPI::Unit* u)
{
  ScoutData temp;
  scouts.insert(std::make_pair(u,temp));
}

void ScoutManager::updateScoutAssignments()
{
  // Remove scout positions if the enemy is not there.
  std::map<BWAPI::Unit*, ScoutData>::iterator u;
  for(u = scouts.begin(); u != scouts.end(); u++)
  {
    if ( (*u).second.mode == ScoutData::Searching
      && (*u).first->getPosition().getDistance((*u).second.target) < BWAPI::TILE_SIZE*(*u).first->getType().sightRange())
    {
      positionsToScout.erase((*u).second.target);
      (*u).second.mode = ScoutData::Idle;
    }
  }

  // Set scouts to scout.
  if (positionsToScout.size() > 0) // are there still positions to scout?
  {
    std::set<const BWAPI::Position>::iterator p;
    for( u = scouts.begin(), p = positionsToScout.begin()
         ;
         u != scouts.end() && p != positionsToScout.end()
         ;
         u++) 
    { // for
      if ((*u).second.mode == ScoutData::Idle)
      {
        
        (*u).second.mode = ScoutData::Searching;
        (*u).first->rightClick(*p);
        (*u).second.target = *p;
        p++;
      }
    } // for
  }

}