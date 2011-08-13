class Schedule
{
  list<Event*> events;
  State currentState;
  std::pair<int, Unit*> getFirstPossibleExecutionTime(TaskType t)
  {
    State state=currentState;
    int sufficientResourcesSince =-1;
    int scheduleForTime =-1;
    for( list<pair<int,Event*>>::iterator i=events.begin();i!=events.end();i=j,j++)
    {
      int t=i.first;
      Event* e=i.second;
      int d=time-t;
      minerals+=d*mineralGatherRate*state.workerCount;
      gas+=d*gasGatherRate*state.workerCount;
      e->apply(&t);
      time=t;
      if (state.hasSufficientEverythingFor(t))
      {
        if (sufficientResourcesSince==-1) sufficientResourcesSince=t;
        for each(Unit* u in unitsForType[t.whatBuilds()])
        {
          if (state.availableSince(u)<time-t.duration())
          {
            scheduleWorker=u;
             scheduleForTime=time;
            break;
          }
        }
      }
      else
      {
        if (timeWhenSufficentMineralsandGas<j.first && state.hasSufficientEverythingButMinGas(t))
        {
          state.proceedToTime(timeWhenSufficentMineralsandGas);
          sufficientResourcesSince=t;
            for each(Unit* u in unitsForType[t.whatBuilds()])
            {
              if (state.availableSince(u)<time-t.duration())
              {
                scheduleWorker=u;
                 scheduleForTime=time;
                break;
              }
            }
        }
        else
        {
          sufficientResourcesSince=-1;
        }
      }
    }
  }
  ScheduledTask* schedule(Unit* worker, Task task, int atTime)
  {
    int minerals = currentMinerals;
    int gas = currentGas;
    int supply = currentSupply;
    int time = currentTime;
    list<pair<int,Event>>::iterator startInterator=events.end();
    list<pair<int,Event>>::iterator endIterator=events.end();
    list<pair<int,Event>>::iterator j=events.begin();j++;
    for( list<pair<int,Event>>::iterator i=events.begin();i!=events.end();i=j,j++)
    {
      if (i.first>atTime && startInterator==events.end()) 
        startInterator=i;
      if (i.first>atTime+task.duration() && endIterator==events.end()) 
      {
        endIterator=i;
        break;
      }
    }
    events.insert(startInterator,task.getBeginEvent());
    events.insert(endIterator,task.getEndEvent());
  }  
}