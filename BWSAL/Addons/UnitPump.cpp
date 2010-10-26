#include <UnitPump.h>
using namespace BWAPI;
UnitPump::UnitPump(UnitType t)
{
  type = t;
}
void UnitPump::update(TaskStream* ts)
{
  if (ts->getTask()==Task())
    ts->setTask(Task(type));
  if (ts->getNextTask()==Task())
    ts->setNextTask(Task(type));
}