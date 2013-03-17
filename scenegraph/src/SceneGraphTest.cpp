
#include "arPrecompiled.h" // This HAS to be first... ugh


#include <list>

#include "arMasterSlaveFramework.h"
#include "arGlut.h"
#include "arEffector.h"
#include "arInteractable.h"
#include "arInteractionUtilities.h"

#include "Node.h"
#include "SceneGraph.h"


class RodEffector : public arEffector {
  public:
    // set it to use matrix #1 (#0 is normally the user's head) and 3 buttons starting at 0 
    // (no axes, i.e. joystick-style controls; those are generally better-suited for navigation
    // than interaction)
    RodEffector() : arEffector( 1, 3, 0, 0, 0, 0, 0 ) {

      // set length to 5 ft.
      setTipOffset( arVector3(0,0,-5) );

      // set to interact with closest object within 1 ft. of tip
      // (see interaction/arInteractionSelector.h for alternative ways to select objects)
      setInteractionSelector( arDistanceInteractionSelector(0.5) );

      // set to grab an object (that has already been selected for interaction
      // using rule specified on previous line) when button 0 or button 2
      // is pressed. Button 0 will allow user to drag the object with orientation
      // change, button 2 will allow dragging but square will maintain orientation.
      // (see interaction/arDragBehavior.h for alternative behaviors)
      // The arGrabCondition specifies that a grab will occur whenever the value
      // of the specified button event # is > 0.5.
      setDrag( arGrabCondition( AR_EVENT_BUTTON, 0, 0.5 ), arWandRelativeDrag() );
      setDrag( arGrabCondition( AR_EVENT_BUTTON, 2, 0.5 ), arWandTranslationDrag() );
    }
    // Draw it. Maybe share some data from master to slaves.
    // The matrix can be gotten from updateState() on the slaves' postExchange().
    void draw() const;
  private:
};

void RodEffector::draw() const {
//std::cout << "Effector matrix:" << std::endl << getCenterMatrix() << std::endl;
  glPushMatrix();
    glMultMatrixf( getCenterMatrix().v );
    // draw grey rectangular solid 2"x2"x5'
    glScalef( 2./12, 2./12., 5. );
    glColor3f( .5,.5,.5 );
    glutSolidCube(1.);
    // superimpose slightly larger black wireframe (makes it easier to see shape)
    glColor3f(0,0,0);
    glutWireCube(1.03);
  glPopMatrix();
}

SceneGraph *sg;

std::list<arInteractable*> objects_;

NodeIdType nothingid;
Node *obj_node;

bool initSceneGraph( arMasterSlaveFramework &fw, arSZGClient &client )
{
    sg = new SceneGraph( fw );
    
    SolidCubeNode *cube = new SolidCubeNode( 2 );
    SolidSphereNode *sphere = new SolidSphereNode( 1, 25, 25 );
    SolidCylinderNode *cone = new SolidCylinderNode( 2, 0, 3, 20, 20 );
    SolidTeapotNode *teapot = new SolidTeapotNode( 1 );
    
    ObjNode *obj = new ObjNode( "al.obj" );

    cube->setTexture( "brick.jpg" );
    cube->setNodeTransform( ar_TM( 2, 5, -5 ) );
    sg->addChild( cube );
    
    sphere->setTexture( "rainbow.jpg" );
    sphere->setNodeTransform( ar_TM( -2, 5, -5 ) );
    sg->addChild( sphere );
    
    cone->setTexture( "rainbow.jpg" );
    cone->setNodeTransform( ar_TM( 0, 2, -8 ) * ar_RM( 'x', -1.59 ) );
    sg->addChild( cone );
    
    teapot->setTexture( "brick.jpg" );
    teapot->setNodeTransform( ar_TM( -4, 3, -8 ) );
    sg->addChild( teapot );
    
    sg->addChild( obj );
    
    return true;
}

RodEffector _effector;

void onPreExchange( arMasterSlaveFramework &fw )
{
    fw.navUpdate();

    // update the input state (placement matrix & button states) of our effector.
    _effector.updateState( fw.getInputState() );

    // Handle any interaction with the square (see interaction/arInteractionUtilities.h).
    // Any grabbing/dragging happens in here.
    ar_pollingInteraction( _effector, objects_ );
}

void doSceneGraph( arMasterSlaveFramework &fw )
{
    fw.loadNavMatrix();
    _effector.draw();
    sg->drawSceneGraph();
    ar_usleep( 100000 / 200 );
}


int main(int argc, char *argv[])
{

    // Initialize framework and bring up window
    
    arMasterSlaveFramework framework;
    if ( !framework.init( argc, argv ) )
    {
        std::cerr << "Failed to init framework!" << std::endl;
        return -1;
    }
    
    framework.setStartCallback( initSceneGraph );
    framework.setDrawCallback( doSceneGraph );
    framework.setPreExchangeCallback( onPreExchange );
    
    return framework.start() ? 0 : 1; // Return 0 if framework.start exits ok
    
    return 0;
}