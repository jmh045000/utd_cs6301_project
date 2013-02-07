
#include "arPrecompiled.h"

#include "MyConditions.h"

#include <cmath>
#include <iostream>
#include <list>
using namespace std;

// In aszgard, this isn't really necessary, but if compiled outside aszgard who knows...
#ifndef NAN
// Compiler independent NaN
inline float gen_NaN()
{
	int x = 0x7F800001; // Hex representation of NaN
	return *(float*)&x;
}
#define NAN gen_NaN()
#endif

UnionGrabCondition& UnionGrabCondition::operator=(const UnionGrabCondition& g)
{
	if(&g == this) return *this;
	conditions = g.conditions;
	return *this;
}

bool UnionGrabCondition::operator==(const UnionGrabCondition& g) const
{
	return conditions == g.conditions;
}

bool UnionGrabCondition::check(arEffector* /*effector*/)
{
	bool val = true;
	for(vector<ConditionEffectorPair>::iterator it = conditions.begin(); it != conditions.end(); ++it)
	{
		if(!it->first.check(it->second))
		{
			val = false;
		}
	}
	
	//cout << "index=" << conditions[0].first.index() << ", Returning: " << val << endl;

	return val;
}

arGrabCondition* UnionGrabCondition::copy() const
{
	return (arGrabCondition*)new UnionGrabCondition(*this);
}

ScaleWithProportions::ScaleWithProportions(arEffector *eff1, arEffector *eff2, bool allowOffset) :
	arDragBehavior(),
	_eff1(eff1), _eff2(eff2), _allowOffset(allowOffset), _scaleOffsetMatrix(),
	olddist(NAN), newdist(NAN)
{}

ScaleWithProportions::ScaleWithProportions(const ScaleWithProportions& d) : 
	arDragBehavior(),
	_eff1(d._eff1), _eff2(d._eff2), 
	_allowOffset(d._allowOffset), _scaleOffsetMatrix(d._scaleOffsetMatrix),
	olddist(d.olddist), newdist(d.newdist)
{}

void ScaleWithProportions::init(const arEffector* const /*effector*/, const arInteractable* const /*object*/)
{
	arMatrix4 mat = _eff1->getMatrix();
	float x1 = mat.v[12];
	float y1 = mat.v[13];
	float z1 = mat.v[14];
	
	mat = _eff2->getMatrix();
	float x2 = mat.v[12];
	float y2 = mat.v[13];
	float z2 = mat.v[14];
	
	olddist = arVector3(x2-x1, y2-y1, z2-z1).magnitude();
}

void ScaleWithProportions::update(const arEffector* const /*effector*/, arInteractable* const object, const arGrabCondition* const /*grabCondition*/)
{
	arMatrix4 mat = _eff1->getMatrix();
	float x1 = mat.v[12];
	float y1 = mat.v[13];
	float z1 = mat.v[14];
	
	mat = _eff2->getMatrix();
	float x2 = mat.v[12];
	float y2 = mat.v[13];
	float z2 = mat.v[14];
	
	newdist = arVector3(x2-x1, y2-y1, z2-z1).magnitude();
	
	if(newdist > olddist + 0.005 || newdist < olddist - 0.005)
	{
		float diff = newdist / olddist;
		object->setMatrix( object->getMatrix()*ar_SM(diff, diff, diff) );
	}
	
	olddist = newdist;
	
}

arDragBehavior* ScaleWithProportions::copy() const
{
	return (arDragBehavior*)new ScaleWithProportions(*this);
}

ScaleWithoutProportions::ScaleWithoutProportions(arEffector *eff1, arEffector *eff2, bool allowOffset) :
	arDragBehavior(),
	_eff1(eff1), _eff2(eff2), _allowOffset(allowOffset), _scaleOffsetMatrix(),
	oldX(NAN), oldY(NAN), oldZ(NAN)
{}

ScaleWithoutProportions::ScaleWithoutProportions(const ScaleWithoutProportions& d) : 
	arDragBehavior(),
	_eff1(d._eff1), _eff2(d._eff2), 
	_allowOffset(d._allowOffset), _scaleOffsetMatrix(d._scaleOffsetMatrix),
	oldX(d.oldX), oldY(d.oldY), oldZ(d.oldZ)
{}

void ScaleWithoutProportions::init(const arEffector* const /*effector*/, const arInteractable* const /*object*/)
{
	cout << "We've been initialized!" << endl;
	arMatrix4 mat = _eff1->getMatrix();
	float x1 = mat.v[12];
	float y1 = mat.v[13];
	float z1 = mat.v[14];
	
	mat = _eff2->getMatrix();
	float x2 = mat.v[12];
	float y2 = mat.v[13];
	float z2 = mat.v[14];
	
	//Right here I'm dividing by 10 (for seemingly no reason...)
	//I can't really explain why, but it seems the y direction is MUCH more sensitive than the x direction
	//I divide by 10 here to basically quiet down the difference
	//Without this, a few millimeters of movement (in the simulator) make the y direction explode, or disappear completely...
	oldX = x2 - x1;
	oldY = y2 - y1;
	oldZ = z2 - z1;
}

void ScaleWithoutProportions::update(const arEffector* const /*effector*/, arInteractable* const object, const arGrabCondition* const /*grabCondition*/)
{
	float newX, newY, newZ;
	arMatrix4 mat = _eff1->getMatrix();
	float x1 = mat.v[12];
	float y1 = mat.v[13];
	float z1 = mat.v[14];
	
	mat = _eff2->getMatrix();
	float x2 = mat.v[12];
	float y2 = mat.v[13];
	float z2 = mat.v[14];
	
	//See above for the bad explanation of why I divide by 10 here
	newX = x2 - x1;
	newY = y2 - y1;
	newZ = z2 - z1;
	
	//cout << "oldX=" << oldX << ", newX=" << newX << endl;
	
	float diffX = 1, diffY = 1, diffZ = 1;
	int tenmultiply = 0;
	
	if(newX > oldX + 0.005 || newX < oldX - 0.005)
	{
		while(oldX > 0 && oldX < 1)
		{
			oldX *= 10;
			newX *= 10;
			tenmultiply++;
		}
		while(oldX < 0 && oldX > -1)
		{
			oldX *= 10;
			newX *= 10;
			tenmultiply++;
		}
		diffX = 1 + ((newX - oldX) / oldX);
		while(tenmultiply)
		{
			newX /= 10;
			oldX /= 10;
			tenmultiply--;
		}
		oldX = newX;
	}
	
	if(newY > oldY + 0.005 || newY < oldY - 0.005)
	{
		while(oldY > 0 && oldY < 1)
		{
			oldY *= 10;
			newY *= 10;
			tenmultiply++;
		}
		while(oldY < 0 && oldY > -1)
		{
			oldY *= 10;
			newY *= 10;
			tenmultiply++;
		}
		diffY = 1 + ((newY - oldY) / oldY);
		while(tenmultiply)
		{
			newY /= 10;
			oldY /= 10;
			tenmultiply--;
		}
		oldY = newY;
	}
	if(newZ > oldZ + 0.005 || newZ < oldZ - 0.005)
	{
		while(oldZ > 0 && oldZ < 1)
		{
			oldZ *= 10;
			newZ *= 10;
			tenmultiply++;
		}
		while(oldZ < 0 && oldZ > -1)
		{
			oldZ *= 10;
			newZ *= 10;
			tenmultiply++;
		}
		diffZ = 1 + ((newZ - oldZ) / oldZ);
		while(tenmultiply)
		{
			newZ /= 10;
			oldZ /= 10;
			tenmultiply--;
		}
		oldZ = newZ;
	}
	//cout << "diffX=" << diffX << endl;
	object->setMatrix( object->getMatrix()*ar_SM(diffX, diffY, diffZ) );
	//cout << "done" << endl;
	
}

arDragBehavior* ScaleWithoutProportions::copy() const
{
	return (arDragBehavior*)new ScaleWithoutProportions(*this);
}

myDragManager::myDragManager() : arDragManager()
{
	cout << "myDragManager cons()" << endl;
}

myDragManager::~myDragManager()
{
	_deleteDrags();
}

void myDragManager::setDrag(const arGrabCondition& cond, const arDragBehavior& behave)
{
	cout << "Adding a drag condition..." << typeid(behave).name() << endl;
	/*
	for(CondDragList::iterator it = _draggers.begin(); it != _draggers.end(); ++it)
	{
		if(*(it->first) == cond)
		{
			if(it->second)
				delete it->second;
			it->second = behave.copy();
			return;
		}
	}
	*/
	_draggers.push_back(std::make_pair(cond.copy(), behave.copy()));
}

void myDragManager::deleteDrag(const arGrabCondition& cond)
{
	for(CondDragList::iterator it = _draggers.begin(); it != _draggers.end(); ++it)
	{
		if(*(it->first) == cond)
		{
			if(it->first)
				delete it->first;
			if(it->second)
				delete it->second;
			_draggers.erase(it);
			return;
		}
	}
}

void myDragManager::getActiveDrags(arEffector* eff, const arInteractable* 
	const object, arDragMap_t& draggers) const
{
	if(!draggers.empty())
	{
		vector<arDragMap_t::iterator> deletions;
		for(arDragMap_t::iterator it = draggers.begin(); it != draggers.end(); ++it)
		{
			arGrabCondition *gc = it->first;
			if(!gc->check(eff))
			{
				delete it->first;
				delete it->second;
				deletions.push_back(it);
			}
		}
		for(vector<arDragMap_t::iterator>::iterator it = deletions.begin(); 
			it != deletions.end(); ++it)
		{
			draggers.erase(*it);
		}
	}
	
	if(!_draggers.empty())
	{
		for(CondDragList::const_iterator it = _draggers.begin(); it != _draggers.end(); ++it)
		{
			arGrabCondition *gc = it->first;
			if(gc->check(eff))
			{
				bool found = false;
				for(arDragMap_t::iterator it2 = draggers.begin(); 
					it2 != draggers.end(); it2 = draggers.begin())
				{
					if(!(*(it2->first) == *gc))
					{
						if(it2->first)
							delete it2->first;
						if(it2->second)
							delete it2->second;
						draggers.erase(it2);
						return;
					}
					else
					{
						found = true;
						break;
					}
				}
				if(!found)
				{
					arDragBehavior* db = it->second->copy();
					db->init(eff, object);
					draggers.insert(std::make_pair(gc->copy(), db));
				}
				return;
			}
		}
	}
}

void myDragManager::_deleteDrags()
{
	for (CondDragList::iterator iter = _draggers.begin(); iter != _draggers.end(); iter++)
	{
		arGrabCondition* cond = iter->first;
		arDragBehavior* behave = iter->second;
		if (cond)
			delete cond;
		if (behave)
			delete behave;
	}
	_draggers.clear();
}
