#include <BuildOrderManager.h>
#include <BuildManager.h>
#include <TechManager.h>
#include <UpgradeManager.h>
#include <WorkerManager.h>
#include <algorithm>
using namespace std;
using namespace BWAPI;
map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >* globalUnitSet;
int y;
map<const Unit*,int> nextFreeTimeData;
map<BWAPI::UnitType, set<BWAPI::UnitType> > makes;
BuildOrderManager::BuildOrderManager(BuildManager* buildManager, TechManager* techManager, UpgradeManager* upgradeManager, WorkerManager* workerManager)
{
  this->buildManager       = buildManager;
  this->techManager        = techManager;
  this->upgradeManager     = upgradeManager;
  this->workerManager      = workerManager;
  this->usedMinerals       = 0;
  this->usedGas            = 0;
  this->dependencyResolver = false;
  UnitItem::getBuildManager() = buildManager;
  for(set<BWAPI::UnitType>::iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end();i++)
  {
    makes[*(*i).whatBuilds().first].insert(*i);
  }
}

//returns the next frame that the given unit type will be ready to produce units or research tech or upgrades
int BuildOrderManager::nextFreeTime(const Unit* unit)
{
  int ctime=Broodwar->getFrameCount();
  if (!unit->isCompleted())
  {
    if (unit->getType().getRace()==Races::Protoss && unit->getType().isBuilding() && unit->getRemainingBuildTime()>0 && nextFreeTimeData[unit]<ctime+24*3)
      nextFreeTimeData[unit]=ctime+24*3;
    if (!unit->isBeingConstructed() && !unit->getType().isAddon())
      return -1;
  }
  int natime=ctime;
  natime=max(ctime,ctime+unit->getRemainingBuildTime());
  if (unit->getType().getRace()==Races::Protoss && unit->getType().isBuilding() && unit->getRemainingBuildTime()>0)
    natime=max(ctime,ctime+unit->getRemainingBuildTime()+24*3);
  natime=max(natime,ctime+unit->getRemainingTrainTime());
  natime=max(natime,ctime+unit->getRemainingResearchTime());
  natime=max(natime,ctime+unit->getRemainingUpgradeTime());
  natime=max(natime,nextFreeTimeData[unit]);
  if (natime==ctime && this->buildManager->getBuildType((Unit*)unit)!=UnitTypes::None)
    natime=ctime+this->buildManager->getBuildType((Unit*)unit).buildTime();
  return natime;
}

//returns the next available time that at least one unit of the given type (buildings only right now) will be completed 
int BuildOrderManager::nextFreeTime(UnitType t)
{
  //if one unit of the given type is already completed, return the given frame count
  if (Broodwar->self()->completedUnitCount(t)>0)
    return Broodwar->getFrameCount();

  //if no units of the given type are being constructed, return -1
  if (Broodwar->self()->incompleteUnitCount(t)==0)
    return -1;

  set<Unit*> allUnits = Broodwar->self()->getUnits();
  int time;
  bool setflag=false;
  for(set<Unit*>::iterator i=allUnits.begin();i!=allUnits.end();i++)
  {
    if ((*i)->getType()==t)
    {
      int ntime=nextFreeTime(*i);
      if (ntime>-1)
      {
        //set time to the earliest available time
        if (!setflag || ntime<time)
        {
          time=ntime;
          setflag=true;
        }
      }
    }
  }
  if (setflag)
    return time;

  //we can get here if construction has been halted by an SCV
  return -1;
}

//returns the next available time that the given unit will be able to train the given unit type
//takes into account required units
//todo: take into account required tech and supply
int BuildOrderManager::nextFreeTime(const Unit* unit, UnitType t)
{
  int time=nextFreeTime(unit);
  for(map<const UnitType*,int>::const_iterator i=t.requiredUnits().begin();i!=t.requiredUnits().end();i++)
  {
    int ntime=nextFreeTime(*i->first);
    if (ntime==-1)
      return -1;
    if (ntime>time)
      time=ntime;
    if (i->first->isAddon() && unit->getAddon()==NULL)
      return -1;
  }
  return time;
}

set<BWAPI::UnitType> BuildOrderManager::unitsCanMake(BWAPI::Unit* builder, int time)
{
  set<BWAPI::UnitType> result;
  for(set<BWAPI::UnitType>::iterator i=makes[builder->getType()].begin();i!=makes[builder->getType()].end();i++)
  {
    int t=nextFreeTime(builder,*i);
    if (t>-1 && t<=time)
      result.insert(*i);
  }
  return result;
}


//prefer unit types that have larger remaining unit counts
//if we need a tie-breaker, we prefer cheaper units
bool unitTypeOrderCompare(const pair<BWAPI::UnitType, int >& a, const pair<BWAPI::UnitType, int >& b)
{
  int rA=a.second;
  int rB=b.second;
  int pA=a.first.mineralPrice()+a.first.gasPrice();
  int pB=b.first.mineralPrice()+b.first.gasPrice();
  return rA>rB || (rA == rB && pA<pB);
}

UnitType getUnitType(set<UnitType>& validUnitTypes,vector<pair<BWAPI::UnitType, int > >& unitCounts)
{
  UnitType answer=UnitTypes::None;
  sort(unitCounts.begin(),unitCounts.end(),unitTypeOrderCompare);
  for(vector<pair<BWAPI::UnitType, int > >::iterator i=unitCounts.begin();i!=unitCounts.end();i++)
  {
    if (validUnitTypes.find(i->first)!=validUnitTypes.end())
    {
      answer=i->first;
      i->second--;
      if (i->second<=0)
      {
        unitCounts.erase(i);
      }
      break;
    }
  }
  return answer;
}
bool BuildOrderManager::updateUnits()
{
  set<Unit*> allUnits = Broodwar->self()->getUnits();
  map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >::iterator i2;
  for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >::iterator i=globalUnitSet->begin();i!=globalUnitSet->end();i=i2)
  {
    i2=i;
    i2++;
    map<BWAPI::UnitType, UnitItem* >::iterator j2;
    for(map<BWAPI::UnitType, UnitItem* >::iterator j=i->second.begin();j!=i->second.end();j=j2)
    {
      j2=j;
      j2++;
      if (j->second==NULL || j->second->getRemainingCount()==0)
      {
        i->second.erase(j);
      }
    }
    if (i->second.empty())
      globalUnitSet->erase(i);
  }

  if (globalUnitSet->empty())
    return false;

  set<Unit*> factories;
  for(set<Unit*>::iterator i=allUnits.begin();i!=allUnits.end();i++)
  {
    Unit* u=*i;
    UnitType type=u->getType();
    if (globalUnitSet->find(type)!=globalUnitSet->end() && this->reservedUnits.find(u)==this->reservedUnits.end())
      factories.insert(u);
  }
  set<int> times;

  for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >::iterator i=globalUnitSet->begin();i!=globalUnitSet->end();i++)
  {
    set<Unit*> factoriesOfType;
    UnitType unitType=i->first;
    for(set<Unit*>::iterator f=factories.begin();f!=factories.end();f++)
    {
      Unit* u=*f;
      UnitType type=u->getType();
      if (type==i->first)
        factoriesOfType.insert(u);
    }
    for(set<Unit*>::iterator f=factoriesOfType.begin();f!=factoriesOfType.end();f++)
    {
      for(map<BWAPI::UnitType, UnitItem* >::iterator j=i->second.begin();j!=i->second.end();j++)
      {
        int time=nextFreeTime(*f,j->first);
        if (time>Broodwar->getFrameCount())
          times.insert(time);
      }
    }
  }
  vector<pair<BWAPI::UnitType, int > > remainingUnitCounts;
  for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >::iterator i=globalUnitSet->begin();i!=globalUnitSet->end();i++)
  {
    for(map<BWAPI::UnitType, UnitItem* >::iterator j=i->second.begin();j!=i->second.end();j++)
    {
      remainingUnitCounts.push_back(make_pair(j->first,j->second->getRemainingCount()));
    }
  }
  map<Unit*,set<UnitType> > buildableUnitTypesNow;
  for(set<Unit*>::iterator f=factories.begin();f!=factories.end();f++)
  {
    Unit* factory=*f;
    UnitType t=getUnitType(unitsCanMake(*f,Broodwar->getFrameCount()),remainingUnitCounts);
    if (t==UnitTypes::None)
      continue;
    if (hasResources(t))
    {
      this->spendResources(t);
      this->reservedUnits.insert(factory);
      TilePosition tp=(*globalUnitSet)[factory->getType()][t]->decrementAdditional();
      if (factory->getAddon()==NULL)
        this->buildManager->build(t,tp,true);
      else
        this->buildManager->build(t,tp);
      BWAPI::Broodwar->printf("Building %s",t.getName().c_str());
      nextFreeTimeData[factory]=Broodwar->getFrameCount()+60;
    }
    else
    {
      this->reserveResources(factory,t);
      this->reservedUnits.insert(factory);
      BWAPI::Broodwar->drawTextScreen(5,y,"Planning to make a %s as soon as possible",t.getName().c_str());y+=20;
      BWAPI::Broodwar->drawTextScreen(5,y,"resource-limited");y+=20;
      return true;
    }
  }

  //reserve units
  for(set<int>::iterator t=times.begin();t!=times.end();t++)
  {
    int ctime=*t;
    map<Unit*,set<UnitType> > buildableUnitTypesNow;
    set<Unit*>::iterator f2;
    for(set<Unit*>::iterator f=factories.begin();f!=factories.end();f=f2)
    {
      f2=f;
      f2++;
      if (this->reservedUnits.find(*f)!=this->reservedUnits.end())
        factories.erase(f);
    }
    for(set<Unit*>::iterator f=factories.begin();f!=factories.end();f++)
    {
      Unit* factory=*f;
      UnitType t=getUnitType(unitsCanMake(*f,ctime),remainingUnitCounts);
      if (t==UnitTypes::None)
        continue;
      if (hasResources(t))
      {
        this->reserveResources(factory,t);
        this->reservedUnits.insert(factory);
        BWAPI::Broodwar->drawTextScreen(5,y,"Planning to make a %s at time %d",t.getName().c_str(),nextFreeTime(factory,t));y+=20;
      }
      else
      {
        this->reserveResources(factory,t);
        this->reservedUnits.insert(factory);
        BWAPI::Broodwar->drawTextScreen(5,y,"Planning to make a %s as soon as possible",t.getName().c_str());y+=20;
        BWAPI::Broodwar->drawTextScreen(5,y,"resource-limited");y+=20;
        return true;
      }
    }
  }
  return false;
}
void BuildOrderManager::update()
{
  if (items.empty()) return;
  map< int, PriorityLevel >::iterator l=items.end();
  l--;
  while (l->second.techs.empty() && l->second.units.empty())
  {
    items.erase(l);
    if (items.empty()) return;
    l=items.end();
    l--;
  }
  this->reservedResources.clear();
  this->reservedUnits.clear();
  y=5;

  //Iterate through priority levels in decreasing order
  for(;l!=items.end();l--)
  {
    BWAPI::Broodwar->drawTextScreen(5,y,"Priority: %d",l->first);y+=20;
    //First consider all techs and upgrades for this priority level
    for(list<TechItem>::iterator i=l->second.techs.begin();i!=l->second.techs.end();i++)
    {
    }
    //Next consider all buildings for this priority level

    //Finally consider all units for this priority level
    map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > > buildings;
    map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > > unitsA;//units that require addons
    map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > > units;
    for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i=l->second.units.begin();i!=l->second.units.end();i++)
    {
      for(map<BWAPI::UnitType, UnitItem >::iterator j=i->second.begin();j!=i->second.end();j++)
      {
        if (j->first.isBuilding())
          buildings[i->first][j->first]=&(j->second);
        else
        {
          //see if the unit type requires an addon
          if (j->first==UnitTypes::Terran_Siege_Tank_Tank_Mode ||
              j->first==UnitTypes::Terran_Siege_Tank_Siege_Mode ||
              j->first==UnitTypes::Terran_Dropship ||
              j->first==UnitTypes::Terran_Battlecruiser ||
              j->first==UnitTypes::Terran_Science_Vessel ||
              j->first==UnitTypes::Terran_Valkyrie)
            unitsA[i->first][j->first]=&(j->second);
          else
            units[i->first][j->first]=&(j->second);
        }
        if (this->dependencyResolver)
        {
          //check dependencies (required units)
          for(map<const BWAPI::UnitType*, int>::const_iterator k=j->first.requiredUnits().begin();k!=j->first.requiredUnits().end();k++)
          {
            if (this->getPlannedCount(*k->first)==0)
            {
              this->build(1,*k->first,l->first);
            }
          }
          //also check to see if we have enough gas, or a refinery planned
          if (j->first.gasPrice()>BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas)
          {
            UnitType refinery=*j->first.getRace().getRefinery();
            if (this->getPlannedCount(refinery)==0)
            {
              this->build(1,refinery,l->first);
            }
          }
        }
      }
    }
    globalUnitSet=&buildings;
    if (updateUnits()) return;
    globalUnitSet=&unitsA;
    if (updateUnits()) return;
    globalUnitSet=&units;
    if (updateUnits()) return;
    map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i2;
    for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i=l->second.units.begin();i!=l->second.units.end();i=i2)
    {
      i2=i;
      i2++;
      map<BWAPI::UnitType, UnitItem >::iterator j2;
      for(map<BWAPI::UnitType, UnitItem >::iterator j=i->second.begin();j!=i->second.end();j=j2)
      {
        j2=j;
        j2++;
        if (j->second.getRemainingCount()==0)
          i->second.erase(j);
      }
      if (i->second.empty())
        l->second.units.erase(i);
    }
  }
  BWAPI::Broodwar->drawTextScreen(5,y,"unit-limited");y+=20;
}

string BuildOrderManager::getName() const
{
  return "Build Order Manager";
}
void BuildOrderManager::build(int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedPosition)
{
  if (t == BWAPI::UnitTypes::None || t == BWAPI::UnitTypes::Unknown) return;
  if (seedPosition == BWAPI::TilePositions::None || seedPosition == BWAPI::TilePositions::Unknown)
    seedPosition=BWAPI::Broodwar->self()->getStartLocation();

  if (items[priority].units[*t.whatBuilds().first].find(t)==items[priority].units[*t.whatBuilds().first].end())
    items[priority].units[*t.whatBuilds().first].insert(make_pair(t,UnitItem(t)));
  items[priority].units[*t.whatBuilds().first][t].setNonAdditional(count,seedPosition);
}

void BuildOrderManager::buildAdditional(int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedPosition)
{
  if (t == BWAPI::UnitTypes::None || t == BWAPI::UnitTypes::Unknown) return;
  if (seedPosition == BWAPI::TilePositions::None || seedPosition == BWAPI::TilePositions::Unknown)
    seedPosition=BWAPI::Broodwar->self()->getStartLocation();

  if (items[priority].units[*t.whatBuilds().first].find(t)==items[priority].units[*t.whatBuilds().first].end())
    items[priority].units[*t.whatBuilds().first].insert(make_pair(t,UnitItem(t)));
  items[priority].units[*t.whatBuilds().first][t].addAdditional(count,seedPosition);
}

void BuildOrderManager::research(BWAPI::TechType t, int priority)
{
  if (t==BWAPI::TechTypes::None || t==BWAPI::TechTypes::Unknown) return;

  items[priority].techs.push_back(TechItem(t));
}

void BuildOrderManager::upgrade(int level, BWAPI::UpgradeType t, int priority)
{
  if (t==BWAPI::UpgradeTypes::None || t==BWAPI::UpgradeTypes::Unknown) return;
  items[priority].techs.push_back(TechItem(t,level));
}

bool BuildOrderManager::hasResources(BWAPI::UnitType t)
{
  if (BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals<t.mineralPrice())
    return false;
  if (BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas<t.gasPrice())
    return false;
  double m=BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals-t.mineralPrice();
  double g=BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas-t.gasPrice();
  for(map<int, Resources>::iterator i=this->reservedResources.begin();i!=this->reservedResources.end();i++)
  {
    double t=i->first-Broodwar->getFrameCount();
    if (m+t*this->workerManager->getMineralRate()<i->second.minerals)
      return false;
    if (g+t*this->workerManager->getGasRate()<i->second.gas)
      return false;
    m-=i->second.minerals;
    g-=i->second.gas;
  }
  return true;
}

bool BuildOrderManager::hasResources(BWAPI::TechType t)
{
  if (BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals<t.mineralPrice())
    return false;
  if (BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas<t.gasPrice())
    return false;
  double m=BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals-t.mineralPrice();
  double g=BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas-t.gasPrice();
  for(map<int, Resources>::iterator i=this->reservedResources.begin();i!=this->reservedResources.end();i++)
  {
    double t=i->first-Broodwar->getFrameCount();
    if (m+t*this->workerManager->getMineralRate()<i->second.minerals)
      return false;
    if (g+t*this->workerManager->getGasRate()<i->second.gas)
      return false;
    m-=i->second.minerals;
    g-=i->second.gas;
  }
  return true;
}

bool BuildOrderManager::hasResources(BWAPI::UpgradeType t)
{
  int mineralPrice=t.mineralPriceBase()+t.mineralPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(t)-1);
  int gasPrice=t.gasPriceBase()+t.gasPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(t)-1);
  if (BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals<mineralPrice)
    return false;
  if (BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas<gasPrice)
    return false;
  double m=BWAPI::Broodwar->self()->cumulativeMinerals()-this->usedMinerals-mineralPrice;
  double g=BWAPI::Broodwar->self()->cumulativeGas()-this->usedGas-gasPrice;
  for(map<int, Resources>::iterator i=this->reservedResources.begin();i!=this->reservedResources.end();i++)
  {
    double t=i->first-Broodwar->getFrameCount();
    if (m+t*this->workerManager->getMineralRate()<i->second.minerals)
      return false;
    if (g+t*this->workerManager->getGasRate()<i->second.gas)
      return false;
    m-=i->second.minerals;
    g-=i->second.gas;
  }
  return true;
}

void BuildOrderManager::spendResources(BWAPI::UnitType t)
{
  this->usedMinerals+=t.mineralPrice();
  this->usedGas+=t.gasPrice();
}

void BuildOrderManager::spendResources(BWAPI::TechType t)
{
  this->usedMinerals+=t.mineralPrice();
  this->usedGas+=t.gasPrice();
}

void BuildOrderManager::spendResources(BWAPI::UpgradeType t)
{
  this->usedMinerals+=t.mineralPriceBase()+t.mineralPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(t)-1);
  this->usedGas+=t.gasPriceBase()+t.gasPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(t)-1);
}

//returns the BuildOrderManager's planned count of units for this type
int BuildOrderManager::getPlannedCount(BWAPI::UnitType t)
{
  //builder unit type
  UnitType builder=*t.whatBuilds().first;

  int c=this->buildManager->getPlannedCount(t);

  //sum all the remaining units for every priority level
  for(map<int, PriorityLevel>::iterator p=items.begin();p!=items.end();p++)
  {
    map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >* units=&(p->second.units);
    map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i=units->find(builder);

    if (i!=units->end())
    {
      map<BWAPI::UnitType, UnitItem >* units2=&(i->second);
      map<BWAPI::UnitType, UnitItem >::iterator j=units2->find(t);
      if (j!=units2->end())
      {
        c+=j->second.getRemainingCount();
      }
    }
  }
  return c;
}

//reserves resources for this unit type
pair<int, BuildOrderManager::Resources> BuildOrderManager::reserveResources(Unit* builder, UnitType unitType)
{
  int t=Broodwar->getFrameCount();
  if (builder)
    t=nextFreeTime(builder,unitType);
  pair<int, Resources> ret;
  ret.first=t;
  ret.second.minerals=unitType.mineralPrice();
  ret.second.gas=unitType.gasPrice();
  this->reservedResources[t].minerals+=unitType.mineralPrice();
  this->reservedResources[t].gas+=unitType.gasPrice();
  return ret;
}

//unreserves the given resources
void BuildOrderManager::unreserveResources(pair<int, BuildOrderManager::Resources> res)
{
  this->reservedResources[res.first].minerals-=res.second.minerals;
  this->reservedResources[res.first].gas-=res.second.gas;
}
void BuildOrderManager::enableDependencyResolver()
{
  this->dependencyResolver=true;
}