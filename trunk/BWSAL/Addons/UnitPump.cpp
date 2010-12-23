#include <UnitPump.h>
using namespace BWAPI;
UnitPump::UnitPump(UnitType t)
{
  type = t;
}
void UnitPump::onFrame(TaskStream* ts)
{
  if (ts==NULL) return;
  int n = ts->workBenches.size()*4;
  int l = ts->queuedTasks.size();
  int d= n-l;
  for(int i=0;i<d;i++)
    ts->queuedTasks.push_back(new Task(type));
}