
#include "arPrecompiled.h" // This HAS to be first... ugh


#include <list>
#include <vector>
#include <sstream>


#include "arMasterSlaveFramework.h"
#include "arGlut.h"
#include "arInteractionUtilities.h"

#include "Node.h"
#include "SceneGraph.h"

#include "MyConditions.h"
#include "WiiMote.h"
#include "Menu.h"

class Wall
{
    float yPosition;
    arTexture tex;
public:
    Wall() : yPosition( 0 ) {}
    
    
    void init();
    void draw();
};

void Wall::init()
{
    tex.repeating( true );
    tex.readJPEG( "grass.jpg" );
}

void Wall::draw()
{
    float v[4][3] =
    {
        { -500, 0, -500 },
        { 500, 0, -500 },
        { 500, 0, 500 },
        { -500, 0, 500 }
    };
	glColor3f( 1, 1, 1 );
    tex.activate();
    glPushMatrix();
        glMultMatrixf( ar_TM( 0, yPosition, 0).v );
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3fv( v[0] );
        glTexCoord2f(50.0f, 0.0f); glVertex3fv( v[1] );
        glTexCoord2f(50.0f, 50.0f); glVertex3fv( v[2] );
        glTexCoord2f(0.0f, 50.0f); glVertex3fv( v[3] );
        glEnd();
    glPopMatrix();
    tex.deactivate();
}

SceneGraph *sg;
MenuNode *menu;
WiiMote primary( WiiMote::CONTROLLER_1 );
WiiMote secondary( WiiMote::CONTROLLER_2 );
Wall ground;
bool menuOn = false;

std::list<arInteractable*> interactableObjects;

myDragManager myDm;

void drawWiimote( WiiMote &wm )
{
	static arOBJRenderer obj;
	static bool init = false;
	if( !init )
	{
		obj.readOBJ( "Hand1.obj" );
		init = true;
	}
	
	glPushMatrix();
		glMultMatrixf( wm.getMatrix().v );
		obj.draw();
	glPopMatrix();
}

bool initSceneGraph( arMasterSlaveFramework &fw, arSZGClient &client )
{
    sg = new SceneGraph( fw );
    sg->getRoot()->setNodeTransform( ar_SM( 2, 2, 2 ) );
    
    menu = initMenu();
    
    primary.setDrag( primary.getGrabCondition( WiiMote::A ), arWandRelativeDrag() );
    secondary.setDrag( secondary.getGrabCondition( WiiMote::A ), arWandRelativeDrag() );
    
    /**/
    
    return true;
}

inline void setMenuOn( arMasterSlaveFramework &fw, WiiMote &eff )
{
    arMatrix4 me, nav;
    nav = ar_getNavMatrix();
    me  = fw.getMidEyeMatrix();

    menuOn = true;

    menu->setNodeTransform( nav * me * ar_TM( 0, 0, -5 ) );

    buildMenu( menu );
}

inline void setMenuOff( arMasterSlaveFramework &fw )
{
    menuOn = false;
    tearDownMenu( menu );
}

inline void toggleMenu( arMasterSlaveFramework &fw, WiiMote &eff )
{
    if( menuOn )
        setMenuOff( fw );
    else
        setMenuOn( fw, eff );
}

void scaleWorld()
{
    int num_buttons = 0;
    WiiMote::ButtonList buttons = primary.getOnButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {  // Process all butons just pressed on primary
        switch( *it )
        {
        case WiiMote::A:
        case WiiMote::B:
            num_buttons++;
        }
    }

    buttons = secondary.getOnButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {  // Process all butons just pressed on secondary
        switch( *it )
        {
        case WiiMote::A:
        case WiiMote::B:
            num_buttons++;
        }
    }
    if(num_buttons == 4)
    {
        cout << "ALL FOUR BUTTONS I NEED ARE PRESSED!" << endl;
    }
}

void onPreExchange( arMasterSlaveFramework &fw )
{
    fw.navUpdate();
    
    // update the input state (placement matrix & button states) of our effector.
    primary.updateState( fw.getInputState() );
    secondary.updateState( fw.getInputState() );

    //used for scale the world (and possibly other scales later)
    WiiMote::updateTipDistance(primary, secondary);
    scaleWorld();
    
    WiiMote::ButtonList buttons = secondary.getDownButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {  // Process all butons just pressed on secondary
        switch( *it )
        {
        case WiiMote::HOME:
            toggleMenu( fw, secondary );
            break;
        case WiiMote::DOWN:
            if( menuOn )
                menu->pressedDown();
            break;
        case WiiMote::RIGHT:
            if( menuOn )
                menu->pressedRight();
            break;
        case WiiMote::LEFT:
            if( menuOn )
                menu->pressedLeft();
            break;
        case WiiMote::UP:
            if( menuOn )
                menu->pressedUp();
            break;
        case WiiMote::A:
            if( menuOn )
            {
                switch( menu->pressedA() )
                {
                case REDRAW:
                    tearDownMenu( menu );
                    buildMenu( menu );
                    break;
                case CLOSE:
                    tearDownMenu( menu );
                    menuOn = false;
                    break;
                }
            }
            break;
        }
    }
    buttons = primary.getDownButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {  // Process all butons just pressed on primary
        switch( *it )
        {
        case WiiMote::HOME:
            toggleMenu( fw, primary );
            break;
        case WiiMote::DOWN:
            if( menuOn )
                menu->pressedDown();
            break;
        case WiiMote::RIGHT:
            if( menuOn )
                menu->pressedRight();
            break;
        case WiiMote::LEFT:
            if( menuOn )
                menu->pressedLeft();
            break;
        case WiiMote::UP:
            if( menuOn )
                menu->pressedUp();
            break;
        case WiiMote::A:
            if( menuOn )
            {
                switch( menu->pressedA() )
                {
                case REDRAW:
                    tearDownMenu( menu );
                    buildMenu( menu );
                    break;
                case CLOSE:
                    tearDownMenu( menu );
                    menuOn = false;
                    break;
                }
            }
            break;
        }
    }    

    // Handle any interaction with the square (see interaction/arInteractionUtilities.h).
    // Any grabbing/dragging happens in here.
    ar_pollingInteraction( primary, interactableObjects );
    ar_pollingInteraction( secondary, interactableObjects );
}

void doSceneGraph( arMasterSlaveFramework &fw )
{
    fw.loadNavMatrix();
    primary.draw();
    secondary.draw();
	ground.draw();
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
	
	primary.setDrawCallback( &drawWiimote );
	secondary.setDrawCallback( &drawWiimote );
	
	primary.setDragManager( &myDm );
	secondary.setDragManager( &myDm );
	
	vector<ConditionEffectorPair> c1;
	c1.push_back(ConditionEffectorPair(primary.getGrabCondition(WiiMote::A), &primary));
	c1.push_back(ConditionEffectorPair(secondary.getGrabCondition(WiiMote::A), &secondary));
	primary.setDrag(UnionGrabCondition(c1), ScaleWithProportions(&primary, &secondary));
	
	vector<ConditionEffectorPair> c2;
	c2.push_back(ConditionEffectorPair(primary.getGrabCondition(WiiMote::B), &primary));
	c2.push_back(ConditionEffectorPair(secondary.getGrabCondition(WiiMote::B), &secondary));
	primary.setDrag(UnionGrabCondition(c2), ScaleWithoutProportions(&primary, &secondary));
    
    primary.setDrag( primary.getGrabCondition( WiiMote::A ), arWandRelativeDrag() );
	ground.init();
    
    framework.setStartCallback( initSceneGraph );
    framework.setDrawCallback( doSceneGraph );
    framework.setPreExchangeCallback( onPreExchange );
    
    return framework.start() ? 0 : 1; // Return 0 if framework.start exits ok
    
    return 0;
}
