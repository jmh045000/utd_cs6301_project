
#include "arDragManager.h"
#include "arInteractableThing.h"
#include "arInteractionUtilities.h"

#include <utility>
#include <list>
#include <set>

typedef std::pair<arGrabCondition, arEffector*> ConditionEffectorPair;

//! \brief This is a grab condition that takes a vector of many conditions and the effectors for them, and makes sure they are all met before returning true
class UnionGrabCondition : public arGrabCondition
{
private:
	std::vector<ConditionEffectorPair> conditions;
	
public:
	//! \brief Empty constructor, if you use this, you must add conditions using add()
	UnionGrabCondition() : arGrabCondition() {}
	
	//! \brief Constructor you SHOULD use.  Takes in a vector of conditions that are checked
	//! \param[in] c Is the vector of conditions that are checked
	UnionGrabCondition(std::vector<ConditionEffectorPair> c) : arGrabCondition(), conditions(c) {}
	UnionGrabCondition(const UnionGrabCondition& c) : arGrabCondition(), conditions(c.conditions) {}
	
	~UnionGrabCondition() {}
	
	UnionGrabCondition& operator=(const UnionGrabCondition& g);
	bool operator==(const UnionGrabCondition& g) const;
	bool check(arEffector* effector);
	void add(ConditionEffectorPair c) { conditions.push_back(c); }
	
	arGrabCondition* copy() const;
};


class ScaleWithProportions : public arDragBehavior {
public:
	ScaleWithProportions(arEffector *eff1, arEffector *eff2, bool allowOffset = true);
	ScaleWithProportions(const ScaleWithProportions& c);
	~ScaleWithProportions() {}
	
	void init( const arEffector* const effector,
               const arInteractable* const object );
    void update( const arEffector* const effector,
                 arInteractable* const object,
                 const arGrabCondition* const grabCondition );
	arDragBehavior* copy() const;
				 
private:
	arEffector *_eff1;
	arEffector *_eff2;
	
    bool _allowOffset;
    arMatrix4 _scaleOffsetMatrix;
	
	float olddist;
	float newdist;
};

class ScaleWithoutProportions : public arDragBehavior {
public:
	ScaleWithoutProportions(arEffector *eff1, arEffector *eff2, bool allowOffset = true);
	ScaleWithoutProportions(const ScaleWithoutProportions& c);
	~ScaleWithoutProportions() {}
	
	void init( const arEffector* const effector,
               const arInteractable* const object );
    void update( const arEffector* const effector,
                 arInteractable* const object,
                 const arGrabCondition* const grabCondition );
	arDragBehavior* copy() const;
				 
private:
	arEffector *_eff1;
	arEffector *_eff2;
	
    bool _allowOffset;
    arMatrix4 _scaleOffsetMatrix;
	
	float oldX, oldY, oldZ;
	float oldDiffX, oldDiffY, oldDiffZ;
};

typedef std::list<std::pair<arGrabCondition*, arDragBehavior*> > CondDragList;

class myDragManager : public arDragManager {
public:
	myDragManager();
	~myDragManager();
	
	myDragManager( const myDragManager& dm );
    myDragManager& operator=( const myDragManager& dm );
    virtual void setDrag( const arGrabCondition& cond,
                  const arDragBehavior& behave );
    virtual void deleteDrag( const arGrabCondition& cond );
    virtual void getActiveDrags( arEffector* eff,
                         const arInteractable* const object,
                         arDragMap_t& draggers ) const;
						 
private:
	void _deleteDrags();
	CondDragList _draggers;
};
