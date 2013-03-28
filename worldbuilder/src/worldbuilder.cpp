
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

SceneGraph *sg;
MenuNode *menu;
WiiMote primary( WiiMote::CONTROLLER_1 );
WiiMote secondary( WiiMote::CONTROLLER_2 );
bool menuOn = false;

std::list<arInteractable*> interactableObjects;

bool initSceneGraph( arMasterSlaveFramework &fw, arSZGClient &client )
{
    sg = new SceneGraph( fw );
    
    menu = initMenu();
    
    primary.setDrag( primary.getGrabCondition( WiiMote::A ), arWandRelativeDrag() );
    secondary.setDrag( primary.getGrabCondition( WiiMote::A ), arWandRelativeDrag() );
    
    return true;
}

void onPreExchange( arMasterSlaveFramework &fw )
{
    fw.navUpdate();
    
    // update the input state (placement matrix & button states) of our effector.
    primary.updateState( fw.getInputState() );
    secondary.updateState( fw.getInputState() );
    
    WiiMote::ButtonList buttons = secondary.getDownButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {  // Process all butons just pressed on secondary
        switch( *it )
        {
        case WiiMote::HOME:
            if( menuOn )
            {
                tearDownMenu( menu );
            }
            else
            {
                arEulerAngles angles( AR_YXZ );
                angles.extract( secondary.getMatrix() );
                angles.setAngles( arVector3( angles )[0], 0, 0 );
                menu->setNodeTransform( ar_ETM( secondary.getMatrix() ) * angles.toMatrix() );
                buildMenu( menu );
            }
            menuOn = !menuOn;
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
            if( menuOn )
            {
                tearDownMenu( menu );
            }
            else
            {
                arEulerAngles angles( AR_YXZ );
                angles.extract( primary.getMatrix() );
                angles.setAngles( arVector3( angles )[0], 0, 0 );
                menu->setNodeTransform( ar_ETM( primary.getMatrix() ) * angles.toMatrix() );
                buildMenu( menu );
            }
            menuOn = !menuOn;
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