
#include "arPrecompiled.h" // This HAS to be first... ugh


#include <list>
#include <assert.h>

#include "arMasterSlaveFramework.h"
#include "arGlut.h"
#include "arInteractionUtilities.h"

#include "Node.h"
#include "SceneGraph.h"

#include "MyConditions.h"
#include "WiiMote.h"

class MenuItem : public Node
{
protected:
    const char *name;
    bool selected;
public:
    MenuItem( const char *n, bool s ) : Node(), name( n ), selected( s ) {}
    
    friend class MenuNode;
};

class Tab : public MenuItem
{
private:
    
    void draw();
public:
    Tab( const char *name, bool selected = false ) : MenuItem( name, selected ) {}
    
    friend struct _TabGroup_s;
};

void Tab::draw()
{
//glDisable(GL_DEPTH_TEST);
    if( selected )
        glColor3f( 1, 1, 1 );
    else
        glColor3f( 0.5, 0.5, 0.5 );
    float hsize = 1;
    float vsize = 0.5;
    GLfloat v[4][3] =
    {
        { -hsize / 2, -vsize / 2, 0.0001 },
        { hsize / 2, -vsize / 2, 0.0001 },
        { hsize / 2, vsize / 2, 0.0001 },
        { -hsize / 2, vsize / 2, 0.0001 }
    };
    
    glBegin( GL_QUADS );
        glTexCoord2f( 0, 0 ); glVertex3fv( v[0] );
        glTexCoord2f( 1, 0 ); glVertex3fv( v[1] );
        glTexCoord2f( 1, 1 ); glVertex3fv( v[2] );
        glTexCoord2f( 0, 1 ); glVertex3fv( v[3] );
    glEnd();
    /**/
    
    glColor3f( 0, 0, 0 );
    glPushMatrix();
        glTranslatef( ( -hsize / 2 ) + 0.3 , 0, 0.001);
        glScalef(0.0009, 0.0009, 0.0009);
        for (const char* c = name; *c; ++c)
        {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
        }
    glPopMatrix();
//glEnable(GL_DEPTH_TEST);
}

typedef struct _TabGroup_s
{
    Tab *objectTab;
    Tab *materialTab;
    Tab *toolsTab;
    
    Tab* operator++(int)
    {
        assert( objectTab && materialTab && toolsTab );
        if( objectTab->selected ) { objectTab->selected = false; materialTab->selected = true; return materialTab; }
        else if( materialTab->selected ) { materialTab->selected = false; toolsTab->selected = true; return toolsTab; }
        else { toolsTab->selected = false; objectTab->selected = true; return objectTab; }
    }
    
    Tab* operator--(int)
    {
        assert( objectTab && materialTab && toolsTab );
        if( objectTab->selected ) { objectTab->selected = false; toolsTab->selected = true; return toolsTab; }
        else if( materialTab->selected ) { materialTab->selected = false; objectTab->selected = true; return objectTab; }
        else { toolsTab->selected = false; materialTab->selected = true; return materialTab; }
    }
} TabGroup;

class MenuNode : public Node
{
private:
    typedef enum { TAB, ITEM, ARROW, MAX_GROUPS } SelectedGroup;
    TabGroup tabs;
    
    MenuItem *currentSelected;
    void draw();
public:
    MenuNode() : Node() {}
    
    void pressedDown();
    void pressedLeft();
    void pressedRight();
    void pressedUp();
    void pressedA();
    
    friend MenuNode* initMenu();
    friend void buildMenu();
    friend void tearDownMenu();
};

void MenuNode::draw()
{
    float hsize = 5;
    float vsize = 3;
    GLfloat v[4][3] =
    {
        { -hsize / 2, -vsize / 2, 0 },
        { hsize / 2, -vsize / 2, 0 },
        { hsize / 2, vsize / 2, 0 },
        { -hsize / 2, vsize / 2, 0 }
    };
    glBegin( GL_QUADS );
        glTexCoord2f( 0, 0 ); glVertex3fv( v[0] );
        glTexCoord2f( 1, 0 ); glVertex3fv( v[1] );
        glTexCoord2f( 1, 1 ); glVertex3fv( v[2] );
        glTexCoord2f( 0, 1 ); glVertex3fv( v[3] );
    glEnd();
}

void MenuNode::pressedDown()
{

}

void MenuNode::pressedLeft()
{
    currentSelected = tabs--;
}

void MenuNode::pressedRight()
{
    currentSelected = tabs++;
}

void MenuNode::pressedUp()
{

}

void MenuNode::pressedA()
{
    cout << currentSelected->name << " is currently selected!" << endl;
}

SceneGraph *sg;
MenuNode *menu;
WiiMote primary( WiiMote::CONTROLLER_1 );
WiiMote secondary( WiiMote::CONTROLLER_2 );
bool menuOn = false;

std::list<arInteractable*> objects_;

MenuNode* initMenu()
{
    MenuNode *menu = new MenuNode();
    menu->setColor( CYAN );
    
    Tab *objectTab = new Tab( "Objects", true );
    objectTab->setNodeTransform( ar_TM( -1.5, 1, 0 ) );
    menu->tabs.objectTab = objectTab;
    menu->currentSelected = objectTab;
    
    Tab *materialTab = new Tab( "Materials" );
    materialTab->setNodeTransform( ar_TM( 0, 1, 0 ) );
    menu->tabs.materialTab = materialTab;
    
    Tab *toolsTab = new Tab( "Tools" );
    toolsTab->setNodeTransform( ar_TM( 1.5, 1, 0 ) );
    menu->tabs.toolsTab = toolsTab;
    
    return menu;
}

void buildMenu()
{
    sg->addChild( menu );
    sg->addChild( menu->tabs.objectTab, menu );
    sg->addChild( menu->tabs.materialTab, menu );
    sg->addChild( menu->tabs.toolsTab, menu );
}

void tearDownMenu()
{
    sg->removeChild( menu->tabs.toolsTab );
    sg->removeChild( menu->tabs.materialTab );
    sg->removeChild( menu->tabs.objectTab );
    sg->removeChild( menu );
}

bool initSceneGraph( arMasterSlaveFramework &fw, arSZGClient &client )
{
    sg = new SceneGraph( fw );
    
    menu = initMenu();
    
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
                tearDownMenu();
            }
            else
            {
                arEulerAngles angles( AR_YXZ );
                angles.extract( secondary.getMatrix() );
                angles.setAngles( arVector3( angles )[0], 0, 0 );
                menu->setNodeTransform( ar_ETM( secondary.getMatrix() ) * angles.toMatrix() );
                buildMenu();
            }
            menuOn = !menuOn;
            break;
        case WiiMote::DOWN:
            menu->pressedDown();
            break;
        case WiiMote::RIGHT:
            menu->pressedRight();
            break;
        case WiiMote::LEFT:
            menu->pressedLeft();
            break;
        case WiiMote::UP:
            menu->pressedUp();
            break;
        case WiiMote::A:
            menu->pressedA();
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
                tearDownMenu();
            }
            else
            {
                arEulerAngles angles( AR_YXZ );
                angles.extract( primary.getMatrix() );
                angles.setAngles( arVector3( angles )[0], 0, 0 );
                menu->setNodeTransform( ar_ETM( primary.getMatrix() ) * angles.toMatrix() );
                buildMenu();
            }
            menuOn = !menuOn;
            break;
        case WiiMote::DOWN:
            menu->pressedDown();
            break;
        case WiiMote::RIGHT:
            menu->pressedRight();
            break;
        case WiiMote::LEFT:
            menu->pressedLeft();
            break;
        case WiiMote::UP:
            menu->pressedUp();
            break;
        case WiiMote::A:
            menu->pressedA();
            break;
        }
    }    

    // Handle any interaction with the square (see interaction/arInteractionUtilities.h).
    // Any grabbing/dragging happens in here.
    ar_pollingInteraction( primary, objects_ );
    ar_pollingInteraction( secondary, objects_ );
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