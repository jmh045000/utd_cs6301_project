
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

void drawRighthand( WiiMote &wm )
{
	static arOBJRenderer obj;
	static bool init = false;
	if( !init )
	{
		obj.readOBJ( "RightHand.obj" );
		init = true;
	}
	
	glPushMatrix();
		glMultMatrixf( wm.getMatrix().v );
		obj.draw();
	glPopMatrix();
}

void drawLefthand( WiiMote &wm )
{
	static arOBJRenderer obj;
	static bool init = false;
	if( !init )
	{
		obj.readOBJ( "LeftHand.obj" );
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
    menu = initMenu( fw );
    return true;
}

inline void setMenuOn( arMasterSlaveFramework &fw )
{
    menuOn = true;
    buildMenu( menu );
    /* Cancel movement while in menu */
    fw.setNavTransSpeed( 0.000001 );
    fw.setNavRotSpeed( 0.000001 );
}

inline void setMenuOff( arMasterSlaveFramework &fw )
{
    menuOn = false;
    tearDownMenu( menu );
    /* Allow movement */
    fw.setNavTransSpeed( 5 );
    fw.setNavRotSpeed( 30 );
}

inline void toggleMenu( arMasterSlaveFramework &fw )
{
    if( menuOn )
        setMenuOff( fw );
    else
        setMenuOn( fw );
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
    
    std::map<WiiMote::button_t, std::list<WiiMote*> > buttonMap;
    WiiMote::ButtonList buttons = secondary.getDownButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {  // Process all butons just pressed on secondary
        if( buttonMap.find( *it ) == buttonMap.end() )
            buttonMap.insert( std::make_pair( *it, std::list<WiiMote*>( 2, &secondary ) ) );
        else
            buttonMap[*it].push_back( &secondary );
    }
    
    buttons = primary.getDownButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {   // Process all butons just pressed on primary
        if( buttonMap.find( *it ) == buttonMap.end() )
            buttonMap.insert( std::make_pair( *it, std::list<WiiMote*>( 2, &secondary ) ) );
        else
            buttonMap[*it].push_back( &secondary );
    }
    
    for( std::map<WiiMote::button_t, std::list<WiiMote*> >::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it )
    {
        switch( it->first )
        {
        case WiiMote::HOME:
            toggleMenu( fw );
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
                    setMenuOff( fw );
                    setMenuOn( fw );
                    break;
                case CLOSE:
                    setMenuOff( fw );
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
    if( menuOn )
        drawMenu( menu, fw );
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
	
	primary.setDrawCallback( &drawRighthand );
	secondary.setDrawCallback( &drawLefthand );
	
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
    secondary.setDrag( secondary.getGrabCondition( WiiMote::A ), arWandRelativeDrag() );
	ground.init();
    
    framework.setStartCallback( initSceneGraph );
    framework.setDrawCallback( doSceneGraph );
    framework.setPreExchangeCallback( onPreExchange );
    
    return framework.start() ? 0 : 1; // Return 0 if framework.start exits ok
    
    return 0;
}
