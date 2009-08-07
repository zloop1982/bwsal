#include <ConstructionManager.h>

ConstructionManager::ConstructionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer)
{
  this->arbitrator = arbitrator;
  this->placer     = placer;
}

void ConstructionManager::onOffer(std::set<BWAPI::Unit*> units)
{
  std::set<Building*> buildingsWithNewBuilders;
  for(std::map<BWAPI::UnitType,std::set<Building*> >::iterator i=this->buildingsNeedingBuilders.begin();i!=this->buildingsNeedingBuilders.end();i++)
  {
    for(std::set<Building*>::iterator b = i->second.begin(); b != i->second.end(); b++)
    {
      double min_dist = 1000000;
      BWAPI::Unit* builder = NULL;
      for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
      {
        double dist = (*u)->getPosition().getDistance((*b)->position);
        if (dist < min_dist)
        {
          min_dist = dist;
          builder  = *u;
        }
      }
      if (builder != NULL)
      {
        arbitrator->accept(this, builder);
        arbitrator->setBid(this, builder, 100.0);
        builders.insert(std::make_pair(builder, *b));
        (*b)->builderUnit = builder;

        if ((*b)->type.isAddon())
        {
          (*b)->tilePosition = builder->getTilePosition();
          if (builder->isLifted())
          {
            if (!this->placer->canBuildHereWithSpace((*b)->tilePosition,(*b)->type))
              (*b)->tilePosition=placer->getBuildLocationNear((*b)->tilePosition,*(*b)->type.whatBuilds().first);
          }
          else
          {
            bool buildable=true;
            for(int x=(*b)->tilePosition.x()+4;x<(*b)->tilePosition.x()+6;x++)
              for(int y=(*b)->tilePosition.y()+1;y<(*b)->tilePosition.y()+3;y++)
                if (!this->placer->buildable(x,y))
                  buildable=false;
            if (!buildable)
            {
              this->placer->freeTiles((*b)->tilePosition, 4,3);
              (*b)->tilePosition=placer->getBuildLocationNear((*b)->tilePosition,*(*b)->type.whatBuilds().first);
            }
          }
          this->placer->reserveTiles((*b)->tilePosition, 4,3);
          this->placer->reserveTiles((*b)->tilePosition+BWAPI::TilePosition(4,1), 2,2);
        }
        units.erase(builder);
        buildingsWithNewBuilders.insert(*b);
      }
    }
  }
  for(std::set<Building*>::iterator b = buildingsWithNewBuilders.begin(); b != buildingsWithNewBuilders.end(); b++)
  {    
    buildingsNeedingBuilders[*(*b)->type.whatBuilds().first].erase(*b);
  }
  for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); u++)
    arbitrator->decline(this, *u, 0);
}

void ConstructionManager::onRevoke(BWAPI::Unit* unit, double bid)
{
  this->onRemoveUnit(unit);
}

void ConstructionManager::update()
{
  std::set<BWAPI::Unit*> myPlayerUnits = BWAPI::Broodwar->self()->getUnits();
  for(std::map<BWAPI::UnitType,std::set<Building*> >::iterator i=this->buildingsNeedingBuilders.begin();i!=this->buildingsNeedingBuilders.end();i++)
  {
    if (!i->second.empty())
    {
      for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); u++)
        if ((*u)->isCompleted() && (*u)->getType()==i->first && (*u)->getAddon()==NULL && this->builders.find(*u)==this->builders.end())
        {
          double min_dist=1000000;
          for(std::set<Building*>::iterator b = i->second.begin(); b != i->second.end(); b++)
          {
            double dist = (*u)->getPosition().getDistance((*b)->position);
            if (dist < min_dist)
              min_dist = dist;
          }
          if (min_dist < 10)
            min_dist = 10;
          if (min_dist > 256*32 + 10)
            min_dist = 256*32 + 10;
          double bid = 80 - (min_dist - 10)/(256*32)*60;
          arbitrator->setBid(this, *u, bid);
        }
    }
  }
  std::list<Building>::iterator i_next;
  int index=0;
  for(std::list<Building>::iterator i=this->incompleteBuildings.begin();i!=this->incompleteBuildings.end();i=i_next)
  {
    index++;
    i_next=i;
    i_next++;
    Building* b = &(*i);
    if (b->type.isAddon())
    {
      if (b->builderUnit!=NULL)
        b->buildingUnit=b->builderUnit->getAddon();

      BWAPI::Unit* u = b->builderUnit;
      BWAPI::Unit* s = b->buildingUnit;
      if (s!=NULL && s->isCompleted())
      {
        //If the building is complete, we can forget about it.
        this->incompleteBuildings.erase(i);
        if (u != NULL)
        {
          this->builders.erase(u);
          arbitrator->setBid(this,u,0);
        }
        this->placer->freeTiles(b->tilePosition, 4,3);
        this->placer->freeTiles(b->tilePosition+BWAPI::TilePosition(4,1), 2,2);
      }
      else
      {
        if (BWAPI::Broodwar->canMake(NULL,b->type))
        {
          if (u == NULL)
            buildingsNeedingBuilders[*b->type.whatBuilds().first].insert(b);
          else
          {
            if (u->getAddon() == NULL)
            {
              if (u->isLifted())
              {
                if (BWAPI::Broodwar->getFrameCount()>b->lastOrderFrame+BWAPI::Broodwar->getLatency()*2)
                {
                  if (!this->placer->canBuildHereWithSpace(b->tilePosition,b->type))
                  {
                    this->placer->freeTiles(b->tilePosition, 4,3);
                    this->placer->freeTiles(b->tilePosition+BWAPI::TilePosition(4,1), 2,2);
                    b->tilePosition=placer->getBuildLocationNear(b->tilePosition,*b->type.whatBuilds().first);
                    this->placer->reserveTiles(b->tilePosition, 4,3);
                    this->placer->reserveTiles(b->tilePosition+BWAPI::TilePosition(4,1), 2,2);
                  }
                  u->land(b->tilePosition);
                  b->lastOrderFrame = BWAPI::Broodwar->getFrameCount();
                }
              }
              else
              {
                if (u->isTraining())
                {
                  u->cancelTrain();
                }
                else
                {
                  if (u->getTilePosition()!=b->tilePosition)
                  {
                    u->lift();
                    b->lastOrderFrame = BWAPI::Broodwar->getFrameCount();
                  }
                  else
                  {
                    bool buildable=true;
                    for(int x=b->tilePosition.x()+4;x<b->tilePosition.x()+6;x++)
                      for(int y=b->tilePosition.y()+1;y<b->tilePosition.y()+3;y++)
                        if (!this->placer->buildable(x,y) || BWAPI::Broodwar->hasCreep(x,y))
                          buildable=false;
                    if (buildable)
                    {
                      u->buildAddon(b->type);
                    }
                    else
                    {
                      u->lift();
                      b->lastOrderFrame = BWAPI::Broodwar->getFrameCount();
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    else
    {
      if (b->tilePosition==BWAPI::TilePositions::None)
      {
        if ((BWAPI::Broodwar->getFrameCount()+index)%25==0 && BWAPI::Broodwar->canMake(NULL,b->type))
        {
          b->tilePosition = this->placer->getBuildLocationNear(BWAPI::Broodwar->self()->getStartLocation(), b->type);
          if (b->tilePosition!=BWAPI::TilePositions::None)
          {
            b->position = BWAPI::Position(b->tilePosition.x()*32 + b->type.tileWidth()*16, b->tilePosition.y()*32 + b->type.tileHeight()*16);
            this->placer->reserveTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());
          }
        }
        if (b->tilePosition==BWAPI::TilePositions::None)
          continue;
      }

      if (b->builderUnit!=NULL && !b->builderUnit->exists())
        b->builderUnit=NULL;
      if (b->buildingUnit!=NULL && (!b->buildingUnit->exists() || b->buildingUnit->getType()!=b->type))
        b->buildingUnit=NULL;

      if (b->buildingUnit == NULL)
      {
        std::set<BWAPI::Unit*> unitsOnTile = BWAPI::Broodwar->unitsOnTile(b->tilePosition.x(), b->tilePosition.y());
        for(std::set<BWAPI::Unit*>::iterator t = unitsOnTile.begin(); t != unitsOnTile.end(); t++)
          if ((*t)->getType().isBuilding() && (*t)->getType() == b->type && !(*t)->isCompleted() && !(*t)->isLifted())
          {
            b->buildingUnit = *t;
            break;
          }
        if (b->buildingUnit == NULL && b->builderUnit!=NULL && b->builderUnit->getType().isBuilding())
        {
          b->buildingUnit = b->builderUnit;
        }
      }
      BWAPI::Unit* u = b->builderUnit;
      BWAPI::Unit* s = b->buildingUnit;
      if (s != NULL && s->isCompleted())
      {
        //If the building is complete, we can forget about it.
        this->incompleteBuildings.erase(i);
        if (u != NULL)
        {
          this->builders.erase(u);
          arbitrator->setBid(this,u,0);
        }
        this->placer->freeTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());
      }
      else
      {
        if (s == NULL)
        {
          if (BWAPI::Broodwar->canMake(NULL,b->type))
          {
            if (u == NULL)
              buildingsNeedingBuilders[*b->type.whatBuilds().first].insert(b);
            else
            {
              if (!u->isConstructing())
              {
                double distance = u->getPosition().getDistance(b->position);
                if (distance > 100)
                  u->rightClick(b->position);
                else
                  if (BWAPI::Broodwar->canBuildHere(u, b->tilePosition, b->type))
                  {
                    if (BWAPI::Broodwar->canMake(u, b->type))
                      u->build(b->tilePosition, b->type);
                  }
                  else
                  {
                    this->placer->freeTiles(b->tilePosition, b->type.tileWidth(), b->type.tileHeight());
                    b->tilePosition = BWAPI::TilePositions::None;
                    b->position = BWAPI::Positions::None;
                  }
              }
            }
          }
        }
        else
        {
          if (b->type.getRace() != BWAPI::Races::Terran)
          {
            if (u != NULL)
            {
              this->builders.erase(u);
              arbitrator->setBid(this, u, 0);
              b->builderUnit = NULL;  
              u = b->builderUnit;
            }
          }
          else
          {
            if (u == NULL)
              buildingsNeedingBuilders[*b->type.whatBuilds().first].insert(b);
            else
            {
              if (BWAPI::Broodwar->getFrameCount()%(4*BWAPI::Broodwar->getLatency())==0)
              {
                if (!u->isConstructing() || !s->isBeingConstructed())
                {
                  u->rightClick(s);
                }
              }
            }
          }
        }
      }
    }
  }
}

std::string ConstructionManager::getName() const
{
  return "Construction Manager";
}

void ConstructionManager::onRemoveUnit(BWAPI::Unit* unit)
{
  if (builders.find(unit) != builders.end())
  {
    Building* building = builders.find(unit)->second;
    building->builderUnit = NULL;
  }
}

bool ConstructionManager::build(BWAPI::UnitType type)
{
  if (!type.isBuilding()) return false;
  Building newBuilding;
  newBuilding.type          = type;
  newBuilding.tilePosition  = BWAPI::TilePositions::None;
  newBuilding.builderUnit   = NULL;
  newBuilding.buildingUnit  = NULL;
  newBuilding.position      = BWAPI::Positions::None;
  newBuilding.lastOrderFrame = 0;
  this->incompleteBuildings.push_back(newBuilding);
  return true;
}
