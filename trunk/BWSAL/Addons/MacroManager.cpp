#include <MacroManager.h>
#include <BWAPI.h>
#include <ResourceRates.h>
#include <math.h>
using namespace BWAPI;
using namespace std;
MacroManager* TheMacroManager = NULL;
Arbitrator::Arbitrator<BWAPI::Unit*,double>* TheArbitrator = NULL;
    int v=0;

MacroManager* MacroManager::create(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  if (TheMacroManager) return TheMacroManager;
  return new MacroManager(arbitrator);
}

MacroManager::MacroManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
  TheArbitrator = arbitrator;
  TheMacroManager = this;
  taskstream_list_visible = true;
}
MacroManager::~MacroManager()
{
  TheMacroManager = NULL;
  for(std::list<TaskStream*>::iterator i=taskStreams.begin();i!=taskStreams.end();i++)
    delete *i;
}
void MacroManager::update()
{
  spentResources.setSupply(0);//don't keep track of spent supply
  Resources r = CumulativeResources(Broodwar->self())-spentResources;
  rtl.reset(r,Broodwar->self()->supplyTotal(),TheResourceRates->getGatherRate().getMinerals(),TheResourceRates->getGatherRate().getGas());
  uctl.reset();
  ttl.reset();
  utl.reset();
  plan.clear();
  for each(TaskStream* ts in killSet)
  {
    for(std::list<TaskStream*>::iterator i=taskStreams.begin();i!=taskStreams.end();i++)
    {
      if (*i == ts)
      {
        taskStreams.erase(i);
        break;
      }
    }
    delete ts;
  }
  for each(TaskStream* ts in taskStreams)
    ts->clearPlanningData();
  Broodwar->drawTextScreen(452,16,"\x07%d",(int)(TheResourceRates->getGatherRate().getMinerals()*23*60));
  Broodwar->drawTextScreen(520,16,"\x07%d",(int)(TheResourceRates->getGatherRate().getGas()*23*60));
  killSet.clear();
  int y=30;
  Broodwar->drawTextScreen(10,5,"Frame: %d",Broodwar->getFrameCount());
  bool plannedAdditionalResources = true;
  while(plannedAdditionalResources)
  {
    plannedAdditionalResources = false;
    for each(TaskStream* ts in taskStreams)
    {
      if (ts->updateStatus())
      {
        plannedAdditionalResources = true;
        break;
      }
    }
  }
  for each(TaskStream* ts in taskStreams)
    ts->update();

  if (taskstream_list_visible)
  {
    for each(TaskStream* ts in taskStreams)
    {
      ts->printToScreen(10,y);
      y+=20;
    }
  }
  {
    int currentFrameCount = Broodwar->getFrameCount();
    double ymax = 200;
    double y=ymax;
    double hscale = 0.3;
    double vscale = 0.3;
    if (v==0)
    {
      y+=r.getMinerals()*vscale;
      for each(std::pair<int, std::list< std::pair<TaskStream*, Task> > > tl in plan)
      {
        int frame = tl.first;
        for each(std::pair<TaskStream*, Task> tp in tl.second)
        {
          Task t=tp.second;
          if (t.getName()=="None") continue;
          double x=(frame - currentFrameCount)*hscale;
          double res=t.getResources().getMinerals()*vscale;
          y-=res;
          double duration=t.getTime()*hscale;
          Broodwar->drawBoxScreen((int)(x),(int)(y),(int)(x+duration),(int)(y+res),Colors::White);
          Broodwar->drawTextScreen((int)(x),(int)(y),"%s",t.getName().c_str());
        }
      }
      y=ymax;
      int y2=(int)(y-TheResourceRates->getGatherRate().getMinerals()*(640/hscale)*vscale);
      if (y2>-1000 && y2<=y)
        Broodwar->drawLineScreen(0,(int)(y),640,y2,Colors::Cyan);
    }
    else if (v==1)
    {
      y+=r.getGas()*vscale;
      for each(std::pair<int, std::list< std::pair<TaskStream*, Task> > > tl in plan)
      {
        int frame = tl.first;
        for each(std::pair<TaskStream*, Task> tp in tl.second)
        {
          Task t=tp.second;
          double x=(frame - currentFrameCount)*hscale;
          double res=t.getResources().getGas()*vscale;
          y-=res;
          double duration=t.getTime()*hscale;
          Broodwar->drawBoxScreen((int)(x),(int)(y),(int)(x+duration),(int)(y+res),Colors::White);
          Broodwar->drawTextScreen((int)(x),(int)(y),"%s",t.getName().c_str());
        }
      }
      y=ymax;
      int y2=(int)(y-TheResourceRates->getGatherRate().getGas()*(640/hscale)*vscale);
      if (y2>-1000 && y2<=y)
        Broodwar->drawLineScreen(0,(int)(y),640,y2,Colors::Cyan);
    }
  }
  /*
    for(std::map<int, Resources>::iterator i=rtl.resourceEvents.begin();i!=rtl.resourceEvents.end();i++)
    {
      Broodwar->drawTextScreen(10,y,"%d: %s",(*i).first,(*i).second.toString().c_str());
      y+=20;
    }
  */
}
bool MacroManager::insertTaskStreamAbove(TaskStream* newTS, TaskStream* existingTS)
{
  if (newTS==NULL || existingTS==NULL) return false;
  std::list<TaskStream*>::iterator e_iter = taskStreams.end();
  for(std::list<TaskStream*>::iterator i=taskStreams.begin();i!=taskStreams.end();i++)
  {
    if (*i==existingTS)
    {
      e_iter = i;
      break;
    }
  }
  if (e_iter == taskStreams.end()) return false;
  taskStreams.insert(e_iter,newTS);
  return true;
}
bool MacroManager::insertTaskStreamBelow(TaskStream* newTS, TaskStream* existingTS)
{
  if (newTS==NULL || existingTS==NULL) return false;
  std::list<TaskStream*>::iterator e_iter = taskStreams.end();
  for(std::list<TaskStream*>::iterator i=taskStreams.begin();i!=taskStreams.end();i++)
  {
    if (*i==existingTS)
    {
      e_iter = i;
      break;
    }
  }
  if (e_iter == taskStreams.end()) return false;
  e_iter++;
  taskStreams.insert(e_iter,newTS);
  return true;
}
bool MacroManager::swapTaskStreams(TaskStream* a, TaskStream* b)
{
  if (a==NULL || b==NULL) return false;
  std::list<TaskStream*>::iterator a_iter = taskStreams.end();
  std::list<TaskStream*>::iterator b_iter = taskStreams.end();
  for(std::list<TaskStream*>::iterator i=taskStreams.begin();i!=taskStreams.end();i++)
  {
    if (*i==a)
      a_iter = i;
    if (*i==b)
      b_iter = i;
  }
  if (a_iter == taskStreams.end() || b_iter == taskStreams.end()) return false;
  *a_iter = b;
  *b_iter = a;
  return true;
}
TaskStream* MacroManager::getTaskStream(BWAPI::Unit* unit) const
{
  std::map<BWAPI::Unit*, TaskStream*>::const_iterator i=unitToTaskStream.find(unit);
  if (i==unitToTaskStream.end())
    return NULL;
  return i->second;
}