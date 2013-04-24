
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

class Ground : public Node
{
    float yPosition;
    arTexture tex;
public:
    Ground() : yPosition( 0 ) {}
    
    void init();
    void draw();
};

void Ground::init()
{
    tex.repeating( true );
    tex.readJPEG( "grasswithborder.jpg" );
}

void Ground::draw()
{
    static const float size = 500;
    static const float v[4][3] =
    {
        { -size, 0, -size },
        { size, 0, -size },
        { size, 0, size },
        { -size, 0, size }
    };
    tex.activate();
    glPushMatrix();
        glMultMatrixf( ar_TM( 0, yPosition, 0).v );
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3fv( v[0] );
        glTexCoord2f(size, 0.0f); glVertex3fv( v[1] );
        glTexCoord2f(size, size); glVertex3fv( v[2] );
        glTexCoord2f(0.0f, size); glVertex3fv( v[3] );
        glEnd();
    glPopMatrix();
    tex.deactivate();
}

SceneGraph *sg;
MenuNode *menu;
WiiMote primary( WiiMote::CONTROLLER_1 );
WiiMote secondary( WiiMote::CONTROLLER_2 );
Ground ground;
bool menuOn = false;

std::list<arInteractable*> interactableObjects;

//Selected objects
int iSelectMenuObj = 0;
std::list<arInteractable*> SelectedObjects;

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

bool initSceneGraph( arMasterSlaveFramework &fw, arSZGClient& /*Unused*/)
{
    sg = new SceneGraph( fw );
	SolidCylinderNode *cyl = new SolidCylinderNode( 1, 1, 20, 20, 20 );
    cyl->setColor( VIOLET );
    cyl->setNodeTransform( ar_RM( 'x', -1.59 ) );
	sg->addChild( cyl );
	sg->addChild( &ground );
    menu = initMenu( fw );
    return true;
}

inline void setMenuOn()
{
    menuOn = true;
    buildMenu( menu );
}

inline void setMenuOff()
{
    menuOn = false;
    tearDownMenu( menu );
}

inline void toggleMenu( )
{
    if( menuOn )
        setMenuOff();
    else
        setMenuOn();
}

void scaleWorld()
{

    static bool scaling = false;
    static float start_dist = 1.0;
    static float scale = 1.0;
    static float ratio = 1.0;
    if( primary.getButton( WiiMote::B ) && secondary.getButton( WiiMote::B ) 
        && ( primary.getGrabbedObject() == NULL ) && ( secondary.getGrabbedObject() == NULL ) )
    {
        //cout << "STW: ";
        if(!scaling)
        {
            scaling = true;
            start_dist = WiiMote::tipDistance;
            if(start_dist < 0.01)
                start_dist = 0.01;
            ratio = 1.0;
        }
        else
        {
            ratio = WiiMote::tipDistance / start_dist;
            //cout << WiiMote::tipDistance << " / " << start_dist << " = " << ratio;
            float sscale = ratio * scale;
            sg->getRoot()->setNodeScale( ar_SM( sscale, sscale, sscale ) );
            //cout << " " << sscale;
        }
        //cout << endl;
    }
    else
    {
        if(scaling) {
            scaling = false;
            scale = scale * ratio;
        }
    }
}

ObjNode *rightClosest = NULL;
ObjNode *leftClosest = NULL;

void moveWorld( arSZGAppFramework &fw )
{
    static const float MOVE_SPEED = 0.25;
    
    if( !menuOn )
    {
        float z = 0, x = 0;
        
        WiiMote::ButtonList buttons;
        buttons = primary.getOnButtons();
        for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
        {  // Process all butons just pressed on primary
            switch( *it )
            {
            case WiiMote::UP:
                z += 0.5;
                break;
            case WiiMote::DOWN:
                z -= 0.5;
                break;
            case WiiMote::LEFT:
                x += 0.5;
                break;
            case WiiMote::RIGHT:
                x -= 0.5;
                break;
            default:
                break;
            }
        }

        buttons = secondary.getOnButtons();
        for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
        {  // Process all butons just pressed on secondary
            switch( *it )
            {
            case WiiMote::UP:
                z += 0.5;
                break;
            case WiiMote::DOWN:
                z -= 0.5;
                break;
            case WiiMote::LEFT:
                x += 0.5;
                break;
            case WiiMote::RIGHT:
                x -= 0.5;
                break;
            default:
                break;
            }
        }
        
        if( z != 0 || x != 0 )
        {
            
            sg->getRoot()->setNodeTransform( sg->getRoot()->getNodeTransform() * ar_ETM( ar_ERM( fw.getMidEyeMatrix() ) * ar_TM( MOVE_SPEED * x, 0, MOVE_SPEED * z ) ) );
        }
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
    moveWorld( fw );
    
    std::map<WiiMote::button_t, std::list<WiiMote*> > buttonMap;
    WiiMote::ButtonList buttons = secondary.getDownButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {  // Process all butons just pressed on secondary
        if( buttonMap.find( *it ) == buttonMap.end() )
            buttonMap.insert( std::make_pair( *it, std::list<WiiMote*>( 1, &secondary ) ) );
        else
            buttonMap[*it].push_back( &secondary );
    }
    
    buttons = primary.getDownButtons();
    for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
    {   // Process all butons just pressed on primary
        if( buttonMap.find( *it ) == buttonMap.end() )
            buttonMap.insert( std::make_pair( *it, std::list<WiiMote*>( 1, &primary ) ) );
        else
            buttonMap[*it].push_back( &primary );
    }
	
	buttons = primary.getUpButtons();
	for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
	{
		switch( *it )
		{
		case WiiMote::A:
		case WiiMote::B:
			if( primary.getGrabbedObject() )
				primary.requestUngrab( primary.getGrabbedObject() );
			break;
		default:
			break;
		}
	}
	
	buttons = secondary.getUpButtons();
	for( WiiMote::ButtonList::iterator it = buttons.begin(); it != buttons.end(); ++it )
	{
		switch( *it )
		{
		case WiiMote::A:
		case WiiMote::B:
			if( secondary.getGrabbedObject() )
				secondary.requestUngrab( secondary.getGrabbedObject() );
			break;
		default:
			break;
		}
	}
    
    for( std::map<WiiMote::button_t, std::list<WiiMote*> >::iterator it = buttonMap.begin(); it != buttonMap.end(); ++it )
    {
        switch( it->first )
        {
        case WiiMote::HOME:
            toggleMenu();
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
		case WiiMote::MINUS:
			cout << "MINUS was pressed" << endl;
            if( !menuOn )
            {
                iSelectMenuObj = 1;
				for( std::list<arInteractable*>::iterator it = SelectedObjects.begin(); it != SelectedObjects.end(); ++it )
					if( Node *n = dynamic_cast<Node*>( *it ) )
						n->setSelected( false );
                SelectedObjects.clear();
            }
            break;
		case WiiMote::PLUS:
			cout << "PLUS was pressed" << endl;
            if( !menuOn )
                iSelectMenuObj = 2;
            break;
        case WiiMote::A:
			iSelectMenuObj = 0;
            if( menuOn )
            {
                switch( menu->pressedA() )
                {
                case REDRAW:
                    setMenuOff();
                    setMenuOn();
                    break;
                case CLOSE:
                    setMenuOff();
                    break;
                default:
                    break;
                }
            }
            break;
        case WiiMote::ONE:
            for(std::list<WiiMote*>::iterator itt = it->second.begin(); itt != it->second.end(); ++itt)
            {
                (*itt)->toggleLightSaber();
            }
            break;
        default:
            break;
        }
    }
	
    // do ray-casting after menu actions
    if ( !menuOn ) {
        rightClosest = primary.closestObject(interactableObjects);
        leftClosest = secondary.closestObject(interactableObjects);
    }
	
    if( rightClosest )
    {
        rightClosest->touch( primary );
		
		if( primary.getButton( WiiMote::A ) && primary.getButton( WiiMote::B ) )
		{
			primary.requestScaleGrab( rightClosest );
		}
        else if( primary.getButton( WiiMote::A ) )
        {
            primary.requestPosGrab( rightClosest );
        }
		else if( primary.getButton( WiiMote::B ) )
		{
			primary.requestRotGrab( rightClosest );
		}
		else if(((primary.getButton( WiiMote::PLUS )) || (primary.getButton( WiiMote::MINUS ))) && ((iSelectMenuObj == 1) || (iSelectMenuObj == 2)))
		{
			bool found = false;
			std::list<arInteractable*> tempSelectedObjects;
			while (!SelectedObjects.empty())
			{
				if ( SelectedObjects.front() == rightClosest )
				{
					cout << "Removing right hand object to selected list" << endl;
					rightClosest->setSelected( false );
					SelectedObjects.pop_front();
					found = true;
					iSelectMenuObj = 0;
					break;
				}
				else
				{
					tempSelectedObjects.push_back( SelectedObjects.front() );
					SelectedObjects.pop_front();
				}
			}
			
			while (!tempSelectedObjects.empty())
			{
				SelectedObjects.push_back( tempSelectedObjects.front() );
				if( Node *n = dynamic_cast<Node*>( tempSelectedObjects.front() ) )
					n->setSelected( true );
				tempSelectedObjects.pop_front();
			}
			
			if (!found)
			{
				cout << "Adding right hand object to selected list" << endl;
				SelectedObjects.push_back( rightClosest );
				rightClosest->setSelected( true );
				iSelectMenuObj = 0;
			}
		}
    }
        
    if( leftClosest )
    {
        leftClosest->touch( secondary );
		if( secondary.getButton( WiiMote::A ) && secondary.getButton( WiiMote::B ) )
		{
			secondary.requestScaleGrab( leftClosest );
		}
        else if( secondary.getButton( WiiMote::A ) )
        {
            secondary.requestPosGrab( leftClosest );
        }
		if( secondary.getButton( WiiMote::B ) )
		{
			secondary.requestRotGrab( leftClosest );
		}
		else if(((secondary.getButton( WiiMote::PLUS )) || (secondary.getButton( WiiMote::MINUS ))) && ((iSelectMenuObj == 1) || (iSelectMenuObj == 2)))
		{
			bool found = false;
			std::list<arInteractable*> tempSelectedObjects;
			while (!SelectedObjects.empty())
			{
				if ( SelectedObjects.front() == leftClosest )
				{
					cout << "Removing left hand object to selected list" << endl;
					leftClosest->setSelected( false );
					SelectedObjects.pop_front();
					found = true;
					iSelectMenuObj = 0;
					break;
				}
				else
				{
					tempSelectedObjects.push_back( SelectedObjects.front() );
					SelectedObjects.pop_front();
				}
			}
			
			while (!tempSelectedObjects.empty())
			{
				SelectedObjects.push_back( tempSelectedObjects.front() );
				if( Node *n = dynamic_cast<Node*>( tempSelectedObjects.front() ) )
					n->setSelected( true );
				tempSelectedObjects.pop_front();
			}
			
			if (!found)
			{
				cout << "Adding left hand object to selected list" << endl;
				SelectedObjects.push_back( leftClosest );
				leftClosest->setSelected( true );
				iSelectMenuObj = 0;
			}
		}
    }
	
}

void doSceneGraph( arMasterSlaveFramework &fw )
{
    static int loops = 0;
    static ar_timeval starttime = ar_time();
    static ar_timeval lastdrawtime = ar_time();
    ar_timeval now = ar_time();
    long long curtime = ( now.sec * 1000000 ) + now.usec, lasttime = ( lastdrawtime.sec * 1000000 ) + lastdrawtime.usec;
    long long sleeptime = 2000 - ( curtime - lasttime );
    ar_usleep( max( (int)sleeptime, 1 ) );
    
    fw.loadNavMatrix();
    glClearColor( 0, 0.749, 1, 0 );
    primary.draw();
    secondary.draw();
    sg->drawSceneGraph();
    if( menuOn )
        drawMenu( menu, fw );
    
    lastdrawtime = ar_time();
    loops++;
    if( lastdrawtime.sec > starttime.sec )
    {
        cout << "fps=" << loops << endl;
        starttime = lastdrawtime;
        loops = 0;
    }
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
    
    framework.setNavTransSpeed( 0 );
    framework.setNavRotSpeed( 0 );
	
	primary.setDrawCallback( &drawRighthand );
	secondary.setDrawCallback( &drawLefthand );
	
	ground.init();
    
    framework.setStartCallback( initSceneGraph );
    framework.setDrawCallback( doSceneGraph );
    framework.setPreExchangeCallback( onPreExchange );
    
    return framework.start() ? 0 : 1; // Return 0 if framework.start exits ok
    
    return 0;
}
