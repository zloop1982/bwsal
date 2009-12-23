#pragma once
#include <list>
#include <set>
#include <map>
#include "Heap.h"
#include "Controller.h"
namespace Arbitrator
{
  template <class _Tp,class _Val>
  class Arbitrator
  {
  public:
    bool setBid(Controller<_Tp,_Val>* c, _Tp obj, _Val bid);
    bool setBid(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid);
    bool removeBid(Controller<_Tp,_Val>* c, _Tp obj);
    bool removeBid(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid);
    bool accept(Controller<_Tp,_Val>* c, _Tp obj, _Val bid);
    bool accept(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid);
    bool accept(Controller<_Tp,_Val>* c, _Tp obj);
    bool accept(Controller<_Tp,_Val>* c, std::set<_Tp> objs);
    bool decline(Controller<_Tp,_Val>* c, _Tp obj, _Val bid);
    bool decline(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid);
    bool hasBid(_Tp obj) const;
    const std::pair<Controller<_Tp,_Val>*, _Val>& getHighestBidder(_Tp obj) const;
    const std::list< std::pair<Controller<_Tp,_Val>*, _Val> > getAllBidders(_Tp obj) const;
    const std::set<_Tp>& getObjects(Controller<_Tp,_Val>* c) const;
    void onRemoveObject(_Tp obj);
    _Val getBid(Controller<_Tp,_Val>* c, _Tp obj) const;
    void update();
  private:
    std::map<_Tp,Heap<Controller<_Tp,_Val>*, _Val> > bids;
    std::map<_Tp,Controller<_Tp,_Val>* > owner;
    std::map<Controller<_Tp,_Val>*, std::set<_Tp> > objects;
    std::set<_Tp> updatedObjects;
  };

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::setBid(Controller<_Tp,_Val>* c, _Tp obj, _Val bid)
  {
    if (c == NULL || obj == NULL)
      return false;
    //set the bid for this object and insert the object into the updated set
    bids[obj].set(c,bid);
    updatedObjects.insert(obj);
    return true;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::setBid(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid)
  {
    bool result;
    for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); o++)
    {
      result |= setBid(c, *o, bid);
    }
    return result;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::removeBid(Controller<_Tp,_Val>* c, _Tp obj)
  {
    if (c == NULL || obj == NULL)
      return false;
    if (bids[obj].contains(c)) //check to see if the bid exists
    {
      bids[obj].erase(c); //if so, remove the bid
      updatedObjects.insert(obj); //insert the object into the updated set
    }
    return true;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::removeBid(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid)
  {
    bool result;
    for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); o++)
    {
      result |= removeBid(c, *o, bid);
    }
    return result;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::decline(Controller<_Tp,_Val>* c, _Tp obj, _Val bid)
  {
    if (c == NULL || obj == NULL)
      return false;
    if (bids[obj].top().first != c) //only the top bidder/controller can decline an object
      return false;
    if (bid == 0)
      bids[obj].erase(c);
    else
      bids[obj].set(c, bid);
    updatedObjects.insert(obj);
    return true;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::decline(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid)
  {
    bool result;
    for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); o++)
    {
      result |= decline(c, *o, bid);
    }
    return result;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::accept(Controller<_Tp,_Val>* c, _Tp obj)
  {
    if (c == NULL || obj == NULL)
      return false;
    if (bids[obj].top().first != c) //only the top bidder/controller can accept an object
      return false;
    if (owner[obj]) //if someone else already own this object, take it away from them
    {
      owner[obj]->onRevoke(obj,bids[obj].top().second);
      objects[owner[obj]].erase(obj); //remove this object from the set of objects owned by the former owner
    }
    owner[obj] = c; //set the new owner
    objects[c].insert(obj); //insert this object into the set of objects owned by this controller
    return true;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::accept(Controller<_Tp,_Val>* c, std::set<_Tp> objs)
  {
    bool result;
    for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); o++)
    {
      result |= accept(c, *o);
    }
    return result;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::accept(Controller<_Tp,_Val>* c, _Tp obj, _Val bid)
  {
    //same idea as accept(Controller<_Tp,_Val>* c, _Tp obj), but the controller also specifies a new bid value
    if (c == NULL || obj == NULL)
      return false;
    if (bids[obj].top().first != c) //only the top bidder/controller can accept an object
      return false;
    if (owner[obj]) //if someone else already own this object, take it away from them
    {
      owner[obj]->onRevoke(obj, bids[obj].top().second);
      objects[owner[obj]].erase(obj); //remove this object from the set of objects owned by the former owner
    }
    bids[obj].set(c,bid); //update the bid for this object
    owner[obj] = c; //set the new owner
    objects[c].insert(obj); //insert this object into the set of objects owned by this controller
    updatedObjects.insert(obj); //since the object was updated, insert it into the updated objects set
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::accept(Controller<_Tp,_Val>* c, std::set<_Tp> objs, _Val bid)
  {
    bool result;
    for (std::set<_Tp>::const_iterator o = objs.begin(); o != objs.end(); o++)
    {
      result |= accept(c, *o, bid);
    }
    return result;
  }

  template <class _Tp,class _Val>
  bool Arbitrator<_Tp,_Val>::hasBid(_Tp obj) const
  {
    //returns true if the given object exists in the bids map
    return (bids.find(obj)!=bids.end());
  }

  template <class _Tp,class _Val>
  const std::pair<Controller<_Tp,_Val>*, _Val>& Arbitrator<_Tp,_Val>::getHighestBidder(_Tp obj) const
  {
    //returns the controller at the top of the bid heap for this object
    return bids.find(obj)->second.top();
  }

  template <class _Tp,class _Val>
  const std::list< std::pair<Controller<_Tp,_Val>*, _Val> > Arbitrator<_Tp,_Val>::getAllBidders(_Tp obj) const
  {
    //returns all bidders for this object
    std::list< std::pair<Controller<_Tp,_Val>*, _Val> > bidders;
    if (bids.find(obj)==bids.end())
      return bidders; //return empty list if we cannot find this object

    Heap<Controller<_Tp,_Val>*, _Val> bid_heap=bids.find(obj)->second; //get the bid heap

    //push the bidders into the bidders list from top to bottom
    while(!bid_heap.empty())
    {
      bidders.push_back(bid_heap.top());
      bid_heap.pop();
    }
    return bidders;
  }

  template <class _Tp,class _Val>
  const std::set<_Tp>& Arbitrator<_Tp,_Val>::getObjects(Controller<_Tp,_Val>* c) const
  {
    //returns the set of objects owned by this bidder/controller
    //if the bidder doesn't exist in this->objects, this will probably crash :/
    return objects.find(c)->second;
  }

  template <class _Tp,class _Val>
  void Arbitrator<_Tp,_Val>::onRemoveObject(_Tp obj)
  {
    //called from AIModule::onUnitDestroy, remove all memory of the object
    bids.erase(obj);
    owner.erase(obj);
    updatedObjects.erase(obj);
    for(std::map<Controller<_Tp,_Val>*, std::set<_Tp> >::iterator c=objects.begin();c!=objects.end();c++)
    {
      (*c).second.erase(obj);
    }
  }

  template <class _Tp,class _Val>
  _Val Arbitrator<_Tp,_Val>::getBid(Controller<_Tp,_Val>* c, _Tp obj) const
  {
    //returns the bid the given controller has on the given object
    return bids.find(obj)->second.get(c);
  }

  template <class _Tp,class _Val>
  void Arbitrator<_Tp,_Val>::update()
  {
    //first we construct a map for the objects to offer
    std::map<Controller<_Tp,_Val>*, std::set<_Tp> > objectsToOffer;

    //go through all the updated objects
    for(std::set<_Tp>::iterator i = updatedObjects.begin(); i != updatedObjects.end(); i++)
    {
      if (!bids[*i].empty()) //if there is a bid on this object
      {
        if (owner.find(*i) == owner.end() || bids[*i].top().first != owner[*i]) //if the top bidder is not the owner
          objectsToOffer[bids[*i].top().first].insert(*i); //make a note to offer it to the top bidder.
      }
      else
      {
        //no bids on this object, remove it from the owner if there is one
        if (owner.find(*i) != owner.end())
        {
          _Val temp=0;
          owner.find(*i)->second->onRevoke(*i,temp);
          owner.erase(*i);
        }
      }
    }
    //reset updated objects
    updatedObjects.clear();

    //offer the objects to the top bidders
    for(std::map< Controller<_Tp,_Val>*, std::set<_Tp> >::iterator i = objectsToOffer.begin(); i != objectsToOffer.end(); i++)
      (*i).first->onOffer((*i).second);
  }
}