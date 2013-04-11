
#include "arPrecompiled.h"

#include "AAMyConditions.h"

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

 WB_ScaleObjectAlongXAxis::WB_ScaleObjectAlongXAxis(arEffector *grabbingEffector, arEffector *changingEffector, float scalingFactor, 
	bool allowOffset):
	arDragBehavior(),
	_grabbingEffector(grabbingEffector),
	_changingEffector(changingEffector),
	_scalingFactor(scalingFactor),
	_allowOffset(allowOffset),
	_scaleOffsetMatrix(),
	_oldDist(NAN),
	_newDist(NAN)
	{}

WB_ScaleObjectAlongXAxis::WB_ScaleObjectAlongXAxis(const WB_ScaleObjectAlongXAxis& d):
	arDragBehavior(),
	_grabbingEffector(d._grabbingEffector),
	_changingEffector(d._changingEffector),
	_scalingFactor(d._scalingFactor),
	_allowOffset(d._allowOffset),
	_scaleOffsetMatrix(d._scaleOffsetMatrix),
	_oldDist(d._oldDist),
	_newDist(d._newDist)
	{}

void WB_ScaleObjectAlongXAxis::init(const arEffector* const /* effector*/ , const arInteractable* const /*object*/)
	{
		arMatrix4 grabbingEffectorMat = _grabbingEffector->getMatrix();
		float x1 = grabbingEffectorMat.v[12];
		float y1 = grabbingEffectorMat.v[13];
		float z1 = grabbingEffectorMat.v[14];

		arMatrix4 changingEffectorMat = _changingEffector->getMatrix();
		float x2 = changingEffectorMat.v[12];
		float y2 = changingEffectorMat.v[13];
		float z2 = changingEffectorMat.v[14];

		// set up the initial distance between the effectors
		_oldDist = arVector3(x2-x1, y2-y1, z2-z1).magnitude();

		// set up the scaling factor. Currently a fixed value.
		// TODO we can add a construcutor to use the scaling factor 
		// derived from the object size and relative distance between wands

	}

void WB_ScaleObjectAlongXAxis::update(const arEffector* const /*effector*/ , arInteractable* const object,
										const arGrabCondition* const /*grabCondition*/)
	{
		arMatrix4 grabbingEffectorMat = _grabbingEffector->getMatrix();
		float x1 = grabbingEffectorMat.v[12];
		float y1 = grabbingEffectorMat.v[13];
		float z1 = grabbingEffectorMat.v[14];

		arMatrix4 changingEffectorMat = _changingEffector->getMatrix();
		float x2 = changingEffectorMat.v[12];
		float y2 = changingEffectorMat.v[13];
		float z2 = changingEffectorMat.v[14];

		// get the current distance between the effectors and set it as the
		// new distance
		_newDist = arVector3(x2-x1, y2-y1, z2-z1).magnitude();

		float epsilonChange = 0.005; // minimum change value required to update the matrix

		if (_newDist > _oldDist + epsilonChange || _newDist < _oldDist - epsilonChange)
		{
			assert(_oldDist != 0.0); // assert we are not dividing by zero

			float diff = (_newDist / _oldDist) * _scalingFactor; 
			arMatrix4 objectMatrix = object->getMatrix();
			object->setMatrix(objectMatrix * ar_SM(diff, 1.0, 1.0));
		}
		//_oldDist = _newDist;
	}

arDragBehavior* WB_ScaleObjectAlongXAxis::copy() const
{
	return (arDragBehavior*) new WB_ScaleObjectAlongXAxis(*this);
}





