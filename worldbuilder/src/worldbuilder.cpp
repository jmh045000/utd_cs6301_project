
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

SolidCubeNode *cube1;
SolidCubeNode *cube2;

bool initSceneGraph( arMasterSlaveFramework &fw, arSZGClient &client )
{
    sg = new SceneGraph( fw );
    sg->getRoot()->setNodeTransform( ar_SM( 2, 2, 2 ) );
    
    menu = initMenu();
    
    primary.setDrag( primary.getGrabCondition( WiiMote::A ), arWandRelativeDrag() );
    secondary.setDrag( primary.getGrabCondition( WiiMote::A ), arWandRelativeDrag() );
    
    cube1 = new SolidCubeNode( 1 );
    cube1->setColor( RED );
    cube1->setNodeTransform( ar_TM( 2, 5, -5 ) );
    
    cube2 = new SolidCubeNode( 1 );
    cube2->setColor( BLUE );
    cube2->setNodeTransform( ar_TM( -2, 5, -5 ) );
    
    sg->addChild( cube1 );
    sg->addChild( cube2 );
    
    interactableObjects.push_back( cube1 );
    interactableObjects.push_back( cube2 );
    /**/
    
    return true;
}

void onPreExchange( arMasterSlaveFramework &fw )
{
    static int i = 0;
    static float scale = 1.0;
    scale += 0.001;
    sg->getRoot()->setNodeTransform( ar_SM( scale, scale, scale ) );
#if 0
    static int i = 0;
    static bool grouped = false;
    
    if( ++i == 500 )
    {
        if( !grouped )
        {
            cout << "Grouping cube1 and cube2" << endl;
            NothingNode *nothing = new NothingNode();
            
            for( list<arInteractable*>::iterator it = interactableObjects.begin(); it != interactableObjects.end(); ++it )
                if( Node *n = dynamic_cast<Node*>( *it ) )
                {
                    sg->removeChild( n );
                    n->setParent( nothing );
                }
            
            sg->addChild( nothing );
            for( list<arInteractable*>::iterator it = interactableObjects.begin(); it != interactableObjects.end(); ++it )
                if( Node *n = dynamic_cast<Node*>( *it ) )
                    sg->addChild( n, nothing );
            
            /*
            sg->removeChild( cube1 );
            cube1->setParent( nothing );
            sg->removeChild( cube2 );
            cube2->setParent( nothing );
            
            sg->addChild( nothing );
            sg->addChild( cube1, nothing );
            sg->addChild( cube2, nothing );
            /**/
        }
        else
        {
            cout << "Ungrouping cube1 and cube2" << endl;
            sg->removeChild( cube1 );
            sg->removeChild( cube2 );
            
            Node *toDelete = cube1->getParent();
            cube1->setParent( cube1 );
            cube2->setParent( cube2 );
            
            sg->removeChild( toDelete );
            delete toDelete;
            
            sg->addChild( cube1 );
            sg->addChild( cube2 );
        }
        i = 0;
        grouped = !grouped;
    }
    
#endif

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