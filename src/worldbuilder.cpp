//********************************************************
// Syzygy is licensed under the BSD license v2
// see the file SZG_CREDITS for details
//********************************************************

// precompiled header include MUST appear as the first non-comment line
#include "arPrecompiled.h"
// MUST come before other szg includes. See arCallingConventions.h for details.
#define SZG_DO_NOT_EXPORT
#include <stdio.h>
#include <stdlib.h>
#include "arEffector.h"
#include "arDistSceneGraphFramework.h"
#include "arPerspectiveCamera.h"
#include "arMesh.h"
#include "arInteractableThing.h"
#include "arInteractionUtilities.h"
#include "arGlut.h"
#include "arMaterial.h"

#include "MyConditions.h"
#include "WiiMote.h"

#include <windows.h>
// The following converts a sleep(n) in seconds call to a Sleep(n) in milliseconds call
#define sleep(n) Sleep(1000 * n)

const float FEET_TO_LOCAL_UNITS = 2.0f;
const float nearClipDistance = .1*FEET_TO_LOCAL_UNITS;
const float farClipDistance = 100.*FEET_TO_LOCAL_UNITS;

static const float EFFECTOR_LENGTH = 1;

class MyCombinedNode : public arInteractable
{
	
	arTransformNode *transformNode;
	arMesh		    *meshObj;

public:
	MyCombinedNode(arDatabaseNode *parent, arMesh *mesh) : transformNode((arTransformNode*)parent->newNode("transform")), meshObj(mesh)
	{
		transformNode->setTransform(ar_TM(0, 5, 0));
		setMatrix(ar_TM(0, 5, 0));
		meshObj->attachMesh((arGraphicsNode*)transformNode);
	}
	~MyCombinedNode()
	{
		delete transformNode;
		delete meshObj;
	}
	
	void setMatrix( const arMatrix4& matrix )
	{
		_matrix = matrix;
		transformNode->setTransform(matrix);
	}
	
	operator arTransformNode*() { return transformNode; }
	operator arInteractable*() { return this; }
	operator arInteractable() { return *this; }
	
};

class MyCombinedEffector : public WiiMote
{
	arTransformNode *transformNode;
	arMesh			*meshObj;
	arSZGAppFramework *framework;
	
public:
	MyCombinedEffector(WiiMote::controller_t con, arDatabaseNode *parent, arMesh *mesh, arSZGAppFramework *fw) : 
		WiiMote(con), transformNode((arTransformNode*)parent->newNode("transform")), meshObj(mesh), framework(fw)
	{
		meshObj->attachMesh((arGraphicsNode*)transformNode);
	}
	
	~MyCombinedEffector()
	{
		delete transformNode;
		delete meshObj;
	}
	
	void update()
	{
		updateState(framework->getInputState());
		transformNode->setTransform(getMatrix());
	}
	
	operator arTransformNode*() { return transformNode; }
};

int main(int argc, char** argv) {

	arDistSceneGraphFramework framework;

	framework.setNavTransCondition( 'z', AR_EVENT_AXIS, 1, 0.2 );
	framework.setNavRotCondition( 'y', AR_EVENT_AXIS, 0, 0.2 );    
	//framework.setUnitConversion(FEET_TO_LOCAL_UNITS);
	framework.setClipPlanes(nearClipDistance, farClipDistance);
	
	// initialize framework
	if(!framework.init(argc, argv))
	{
		return 1;
	}
	
	if(framework.getStandalone())
	{
		printf("Doesn't work in standalone\n");
		return 1;
	}
	
	// Set up the light...
	arLight light;
	light.lightID = 0;
	light.position = arVector4(0, 0, 1, 0);
	light.diffuse = arVector3(1, 1, 1);
	
	arLight omnilight;
	omnilight.lightID = 1;
	omnilight.position = arVector4(0, 5, 0, 1);
	omnilight.diffuse = arVector3(1, 1, 1);
	omnilight.specular = arVector3(0.5, 0.5, 0.5);
		
	// Add a material for the sphere to attach to
	arMaterial material;
	material.diffuse = arVector3(1, 0, 0);

	arGraphicsDatabase* graphics = framework.getDatabase();
	//graphics->setTexturePath(".");
	arDatabaseNode* root = (arDatabaseNode*)graphics->getRoot();
	arTransformNode* headNode = (arTransformNode*)root->newNode("transform");

	// Set a material, so we can change the color
	arMaterialNode* materialNode = (arMaterialNode*)root->newNode("material");
	materialNode->setMaterial(material);
	
	// Add a transform to the material, the sphere will attach to this node, so that it gets moved
	arTransformNode* transformNode = (arTransformNode*)root->newNode("transform");
	//transformNode->setTransform(ar_TM(0, 0, -3));
	
	// Add the light to the root
	arLightNode* lightNode = (arLightNode*)root->newNode("light");
	lightNode->setLight(light);
	
	arLightNode* omniLightNode = (arLightNode*)root->newNode("light");
	omniLightNode->setLight(omnilight);
	
	MyCombinedNode mynode(materialNode, new arSphereMesh(25));
	
	myDragManager myDrag;
	
	MyCombinedEffector primary(WiiMote::CONTROLLER_1, root, new arCubeMesh(ar_SM(2./12., 2./12., EFFECTOR_LENGTH)), &framework);
	MyCombinedEffector secondary(WiiMote::CONTROLLER_2, root, new arCubeMesh(ar_SM(2./12., 2./12., EFFECTOR_LENGTH)), &framework);
	
	primary.setDragManager(&myDrag);
	secondary.setDragManager(&myDrag);
	
	vector<ConditionEffectorPair> c1;
	c1.push_back(ConditionEffectorPair(arGrabCondition(AR_EVENT_BUTTON, 3, 0.5), &primary));
	c1.push_back(ConditionEffectorPair(arGrabCondition(AR_EVENT_BUTTON, 3, 0.5), &secondary));
	primary.setDrag(UnionGrabCondition(c1), ScaleWithProportions(&primary, &secondary));
	
	vector<ConditionEffectorPair> c2;
	c2.push_back(ConditionEffectorPair(arGrabCondition(AR_EVENT_BUTTON, 2, 0.5), &primary));
	c2.push_back(ConditionEffectorPair(arGrabCondition(AR_EVENT_BUTTON, 2, 0.5), &secondary));
	primary.setDrag(UnionGrabCondition(c2), ScaleWithoutProportions(&primary, &secondary));
  
	primary.setDrag(arGrabCondition(AR_EVENT_BUTTON, 3, 0.5), arWandRelativeDrag());

	if(!framework.start()) {
		return 1;
	}
	
	float position = 0.;
	float delta = 0.0001;
	
	primary.update();
	secondary.update();
	
	while(true)
	{
		framework.loadNavMatrix();

		headNode->setTransform(framework.getMidEyeMatrix());
		
		primary.update();
		secondary.update();
		
		ar_pollingInteraction( primary, mynode );
		ar_pollingInteraction( secondary, mynode );
		
		framework.setViewer();
		Sleep(1);
	}

	return 0;
}
