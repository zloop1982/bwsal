#include <MacroManager/WorkerTaskTimeline.h>
#include <MacroManager.h>
#include <math.h>
using namespace std;
using namespace BWAPI;

WorkerTaskTimeline::WorkerTaskTimeline()
{
}

void WorkerTaskTimeline::reset()
{
  taskStartTimes.clear();
}

pair<int,int> WorkerTaskTimeline::getFirstFreeInterval(Unit* worker, Task* task, int earliestStartTime)
{
  if (earliestStartTime==-1)
    earliestStartTime = Broodwar->getFrameCount();
  if (earliestStartTime<Broodwar->getFrameCount())
    return make_pair(-1,-1);
  if (!task)
    return make_pair(-1,-1);
  int minimumDuration = task->getTime();
  map<Unit*, map<int,Task*> >::iterator tst=taskStartTimes.find(worker);
  if (tst==taskStartTimes.end())
    return make_pair(earliestStartTime,-1);
  map<int,Task*>::iterator j=tst->second.begin();j++;
  for(map<int,Task*>::iterator i=tst->second.begin();i!=tst->second.end();i++)
  {
    if (i->second) continue;
    int intervalStart = i->first;
    int start = max(intervalStart,earliestStartTime);
    int intervalEnd = -1;
    if (j!=tst->second.end())
      intervalEnd=j->first;
    if (intervalEnd==-1 || intervalEnd-start>=minimumDuration)
    {
      return make_pair(start,intervalEnd);
    }
    if (j!=tst->second.end())
      j++;    
  }
  return make_pair(-1,-1);
}

bool WorkerTaskTimeline::reserveTime(Unit* worker, int startFrame, Task* task)
{
  if (!task) return false;
  if (startFrame<Broodwar->getFrameCount()) return false;
  map<Unit*, map<int,Task*> >::iterator tst=taskStartTimes.find(worker);
  if (tst==taskStartTimes.end())
  {
    taskStartTimes[worker].insert(make_pair(Broodwar->getFrameCount(),(Task*)NULL));
    tst=taskStartTimes.find(worker);
  }
  int endFrame = startFrame + task->getTime();
  map<int,Task*>::iterator j=tst->second.begin();j++;
  for(map<int,Task*>::iterator i=tst->second.begin();i!=tst->second.end();i++)
  {
    if (i->second) continue;
    int intervalStart = i->first;
    int intervalEnd = -1;
    if (j!=tst->second.end())
      intervalEnd=j->first;
    if (startFrame>=intervalStart && (intervalEnd==-1 || endFrame<intervalEnd))
    {
      if (startFrame==intervalStart)
        i->second = task;
      else
        i = tst->second.insert(i,make_pair(startFrame,task));
      if (endFrame!=intervalEnd)
        tst->second.insert(i,make_pair(endFrame,(Task*)NULL));
      return true;
    }
    if (j!=tst->second.end())
      j++;
  }
  return false;
}
Task* WorkerTaskTimeline::getPlannedTask(Unit* worker, int frame)
{
  map<Unit*, map<int,Task*> >::iterator tst=taskStartTimes.find(worker);
  if (tst==taskStartTimes.end())
    return NULL;
  map<int,Task*>::iterator j=tst->second.begin();j++;
  for(map<int,Task*>::iterator i=tst->second.begin();i!=tst->second.end();i++)
  {
    if (i->second) continue;
    int intervalStart = i->first;
    int intervalEnd = -1;
    if (j!=tst->second.end())
      intervalEnd=j->first;
    if (intervalStart<=frame && (intervalEnd == -1 || frame<intervalEnd))
    {
      return i->second;
    }
    if (j!=tst->second.end())
      j++;
  }
  return NULL;
}