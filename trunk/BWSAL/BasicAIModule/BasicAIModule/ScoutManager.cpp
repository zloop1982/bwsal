#include <BWTA.h>
#include <ScoutManager.h>

ScoutManager::ScoutManager(Arbitrator::Arbitrator<BWAPI::Unit*,double> *arbitrator)
{
  this->arbitrator = arbitrator;
  desiredScoutCount = 1;
  scoutingStartFrame = 50;

  myStartLocation = BWTA::getStartLocation(BWAPI::Broodwar->self());
  std::set<BWTA::BaseLocation *> locations = BWTA::getBaseLocations();
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
  if (bid < 40 /* maxBid */)
  {
    // Try to increase the bid if cannot receive a scout.
    requestScout(bid+1);
  }
}

void ScoutManager::update()
{
  if (isScoutTime())
  {
    if (needMoreScouts())
    {
      requestScout(/* bid = */ 10); // Start with initial bid of 10.
    }

    updateScoutAssignments();
  }
}

std::string ScoutManager::getName() const
{
  return "Scout Manager";
}

void ScoutManager::onRemoveUnit(BWAPI::Unit* unit)
{
  scouts.erase(unit);
}

bool ScoutManager::isScoutTime() const
{
  return BWAPI::Broodwar->getFrameCount() >= scoutingStartFrame;
  // TODO: Check also if we have enough workers done to be sure that
  // no scouting is done without good resource production first.
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
      && (*u).first->getPosition().getDistance((*u).second.target) < 2 /* some delta */)
    {
      positionsToScout.erase((*u).second.target);
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