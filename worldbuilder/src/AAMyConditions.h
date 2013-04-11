#include "arInteractionUtilities.h"

#include <utility>
#include <list>
#include <set>



class WB_ScaleObjectAlongXAxis : public arDragBehavior {
public:
	/*
	Constructor takes left and right effectors, a scaling factor
	that determines how much to scale object by and a boolean
	parameter allowOffset.
	grabbingEffector is the effector grabbing the object. 
	changingEffector is the effector that is used to scale object.
	
	*/
	WB_ScaleObjectAlongXAxis(arEffector *grabbingEffector, arEffector *changingEffector, float scalingFactor, bool allowOffset);
	WB_ScaleObjectAlongXAxis(const WB_ScaleObjectAlongXAxis& c);

	~WB_ScaleObjectAlongXAxis() {} // destructor

	void init(const arEffector* const effector, const arInteractable* const object);
	void update(const arEffector* const effector, arInteractable* const object,
		const arGrabCondition* const grabCondition);
	arDragBehavior* copy() const;

private:
	arEffector *_grabbingEffector;
	arEffector *_changingEffector;

	float _scalingFactor;

	bool _allowOffset;
	arMatrix4 _scaleOffsetMatrix;

	float _oldDist;
	float _newDist;
};
