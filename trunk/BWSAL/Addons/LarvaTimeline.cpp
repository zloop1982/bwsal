#include <MacroManager/LarvaTimeline.h>
#include <MacroManager.h>
#include <math.h>
using namespace std;
using namespace BWAPI;

LarvaTimeline::LarvaTimeline()
{
}

void LarvaTimeline::reset()
{
  larvaEventTimes.clear();
  for each(Unit* u in Broodwar->self()->getUnits())
  {
    if (u->getType().producesLarva() && u->getLarva().size()<3)
    {
      larvaEventTimes[u].insert(std::make_pair(Broodwar->getFrameCount()+u->getRemainingTrainTime(),1));
      if (u->getLarva().size()<2)
        larvaEventTimes[u].insert(std::make_pair(Broodwar->getFrameCount()+u->getRemainingTrainTime()+334,1));
      if (u->getLarva().size()<1)
        larvaEventTimes[u].insert(std::make_pair(Broodwar->getFrameCount()+u->getRemainingTrainTime()+334+334,1));
    }
  }
}
std::pair<int,int> LarvaTimeline::getFirstFreeInterval(BWAPI::Unit* worker, int earliestStartTime)
{
  std::map<BWAPI::Unit*, std::map<int, int> > possibleNewLarvaEventTimes = larvaEventTimes;
  int frame = Broodwar->getFrameCount();
  int larva = worker->getLarva().size();
  if (larvaEventTimes.find(worker)==larvaEventTimes.end())
  {
    if (larva>0)
      return std::make_pair(frame,-1);
    return std::make_pair(-1,-1);
  }
  std::map<int,int>* eventTimes = &(larvaEventTimes.find(worker)->second);
  int startFrame = -1;
  int first3Frame = -1;
  for(std::map<int,int>::iterator i=eventTimes->begin();i!=eventTimes->end();i++)
  {
    int currentFrame = i->first;
    larva += i->second;
    if (larva>=1 && startFrame == -1)
      startFrame = currentFrame;
    if (startFrame != -1)
    {
      if (larva == 3 && first3Frame == -1)
        first3Frame = currentFrame;
      if (larva==0)
      {
        if (first3Frame != -1 && currentFrame>first3Frame+334)
          return std::make_pair(startFrame,currentFrame-334);
        else
        {
          startFrame = -1;
          first3Frame = -1;
        }
      }
    }
  }
  return std::make_pair(startFrame,-1);
}
std::pair<int,int> LarvaTimeline::getFirstFreeInterval(BWAPI::Unit* worker, const Task* task, int earliestStartTime)
{
}
bool LarvaTimeline::reserveLarva(BWAPI::Unit* worker, int startFrame, Task* task)
{
  larvaEventTimes[worker][startFrame]--;
  std::map<int,int>* eventTimes = &(larvaEventTimes.find(worker)->second);
  int larva = worker->getLarva().size();
  int endFrame = -1;
  bool isValid = true;
  bool checking = false;
  for(std::map<int,int>::iterator i=eventTimes->begin();i!=eventTimes->end();i++)
  {
    int currentFrame = i->first;
    larva += i->second;
    if (currentFrame < startFrame) continue;

    if (currentFrame == startFrame)
      checking=true;

    if (checking)
    {
      if (larva<0)
      {
        isValid = false;
        break;
      }
      if (larva<2) endFrame = -1;
      if (larva>=2) endFrame = currentFrame+334;
    }
    if (currentFrame>=endFrame && endFrame != -1)
    {
      isValid = true;
      checking = false;
      break;
    }
  }
  if (isValid)
    larvaEventTimes[worker][endFrame]++;
  else
    larvaEventTimes[worker][startFrame]--;
  return isValid;

}
int LarvaTimeline::getPlannedLarvaCount(BWAPI::Unit* worker, int frame)
{
  int larvaCount = worker->getLarva().size();
  if (larvaEventTimes.find(worker)==larvaEventTimes.end())
    return larvaCount;
  std::map<int,int>* eventTimes = &(larvaEventTimes.find(worker)->second);
  for(std::map<int,int>::iterator i=eventTimes->begin();i!=eventTimes->end();i++)
  {
    if ((*i).first > frame) break;
    larvaCount+=(*i).second;
  }
  return larvaCount;
}