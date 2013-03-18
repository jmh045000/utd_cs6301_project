
#include "arPrecompiled.h" // This HAS to be first... ugh


#include <list>
#include <assert.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
/**/

#include "arMasterSlaveFramework.h"
#include "arGlut.h"
#include "arInteractionUtilities.h"

#include "Node.h"
#include "SceneGraph.h"

#include "MyConditions.h"
#include "WiiMote.h"

namespace fs = boost::filesystem;

class MenuItem : public Node
{
protected:
    bool selected;
    
    MenuItem* operator++(int);
public:
    MenuItem( const char *n, bool s ) : Node(), name( n ), selected( s ) {}
    
    const char *name;
    
    friend class MenuNode;
};

class Tab : public MenuItem
{
private:
    bool active;
    void draw();
public:
    Tab( const char *name, bool selected = false ) : MenuItem( name, selected ), active( selected ) {}
    
    void deactivate() { active = false; }
    void activate() { active = true; }
    
    friend struct _TabGroup_s;
};

void Tab::draw()
{

    if( active )
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
    
    glColor3f( 0, 0, 0 );
    glPushMatrix();
        glTranslatef( ( -hsize / 2 ) + 0.3 , 0, 0.001);
        glScalef(0.0009, 0.0009, 0.0009);
        for (const char* c = name; *c; ++c)
        {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
        }
    glPopMatrix();
    
    if( selected )
    {
        glColor3f( 0, 0, 0 );
        glBegin( GL_LINE_LOOP );
            for( int i = 0; i < 4; i++ )
            {
                if( v[i][0] > 0 )
                    v[i][0] += 0.1;
                else
                    v[i][0] -= 0.1;
                if( v[i][1] > 0 )
                    v[i][1] += 0.1;
                else
                    v[i][1] -= 0.1;
                glVertex3fv( v[i] );
            }
        glEnd();
    }
}

class Item : public MenuItem
{
public:
    typedef enum { OBJECT, TEXTURE, TOOL, MAX_ITEM_TYPES } ItemType;
private:
    ItemType type;
    string filename;
    void draw();
public:
    Item( const char *name, ItemType t, string f, bool selected = false ) : MenuItem( name, selected ), type( t ), filename( f ) {}
    
    friend struct _ItemGroup_s;
};

void Item::draw()
{
    
    float hsize = 1;
    float vsize = 1;
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
    
    if( selected )
    {
        glBegin( GL_LINE );
            for( int i = 0; i < 4; i++ )
            {
                v[i][0] += 0.1;
                v[i][1] += 0.1;
                glVertex3fv( v[i] );
            }
        glEnd();
    }
}

typedef struct _MenuGroup_s
{
    virtual string name() = 0;
    virtual MenuItem* first() = 0;
    virtual MenuItem* operator++(int) = 0;
    virtual MenuItem* operator--(int) = 0;
} MenuGroup_t;


typedef struct _TabGroup_s : public MenuGroup_t
{
    Tab *objectTab;
    Tab *materialTab;
    Tab *toolsTab;
    
    string name()
    {
        return "TabGroup";
    }
    
    MenuItem* first()
    {
        return objectTab;
    }
    
    MenuItem* operator++(int)
    {
        assert( objectTab && materialTab && toolsTab );
        if( objectTab->selected ) { objectTab->selected = false; materialTab->selected = true; return materialTab; }
        else if( materialTab->selected ) { materialTab->selected = false; toolsTab->selected = true; return toolsTab; }
        else { toolsTab->selected = false; objectTab->selected = true; return objectTab; }
    }
    
    MenuItem* operator--(int)
    {
        assert( objectTab && materialTab && toolsTab );
        if( objectTab->selected ) { objectTab->selected = false; toolsTab->selected = true; return toolsTab; }
        else if( materialTab->selected ) { materialTab->selected = false; objectTab->selected = true; return objectTab; }
        else { toolsTab->selected = false; materialTab->selected = true; return materialTab; }
    }
} TabGroup;

typedef struct _ItemGroup_s : public MenuGroup_t
{
    list<Item*> items;
    
    string name()
    {
        return "ItemGroup";
    }
    
    MenuItem* first()
    {
        return items.front();
    }
    
    MenuItem* operator++ (int)
    {
        return items.front();
    }
    
    MenuItem* operator-- (int)
    {
        return items.front();
    }
    
    _ItemGroup_s( list<Item*> i ) : MenuGroup_t(), items( i ) {}
    
} ItemGroup;

class MenuNode : public Node
{
private:
    typedef enum { TAB, ITEM, MAX_GROUPS } SelectedGroup;
    
    TabGroup tabs;
    ItemGroup *items;
    SelectedGroup selectedGroup;
    MenuItem *currentSelected;
    void draw();
public:
    MenuNode() : Node(), selectedGroup( TAB ), currentSelected( NULL )  {}
    
    void pressedDown();
    void pressedLeft();
    void pressedRight();
    void pressedUp();
    bool pressedA();
    
    friend SelectedGroup operator++(SelectedGroup);
    friend SelectedGroup operator--(SelectedGroup);
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
    selectedGroup = ++selectedGroup;
    switch( selectedGroup )
    {
    case TAB:
        currentSelected = tabs.first();
        break;
    case ITEM:
        currentSelected = items->first();
        break;
    }
}

void MenuNode::pressedLeft()
{
    switch( selectedGroup )
    {
    case TAB:
        currentSelected = tabs--;
        break;
    case ITEM:
        break;
    }
}

void MenuNode::pressedRight()
{
    switch( selectedGroup )
    {
    case TAB:
        currentSelected = tabs++;
        break;
    case ITEM:
        break;
    }
}

void MenuNode::pressedUp()
{
    selectedGroup = ++selectedGroup;
    switch( selectedGroup )
    {
    case TAB:
        currentSelected = tabs.first();
        break;
    case ITEM:
        currentSelected = items->first();
        break;
    }
}

bool MenuNode::pressedA()
{
    switch( selectedGroup )
    {
    case TAB:
        tabs.objectTab->deactivate();
        tabs.materialTab->deactivate();
        tabs.toolsTab->deactivate();
        if( Tab *t = dynamic_cast<Tab*>(currentSelected) )
        {
            t->activate();
        }
        break;
    }
}

MenuNode::SelectedGroup operator++( MenuNode::SelectedGroup s )
{
    switch( s )
    {
    case MenuNode::TAB:
        return MenuNode::ITEM;
        break;
    case MenuNode::ITEM:
        return MenuNode::TAB;
        break;
    default:
        throw( "Bad operator++" );
    }
}

MenuNode::SelectedGroup operator--( MenuNode::SelectedGroup s )
{
    switch( s )
    {
    case MenuNode::TAB:
        return MenuNode::ITEM;
        break;
    case MenuNode::ITEM:
        return MenuNode::TAB;
        break;
    default:
        throw( "Bad operator--" );
    }
}

list<Item*> findObjects()
{

    list<Item*> l;
    
    fs::path p( "./worldbuilder_rsc/" );
    if( !fs::exists( p ) )
        throw( "Must run in directory with worldbuilder_rsc" );
    
    p /= "objects";
    if( !fs::exists( p ) )
        throw( "worldbuilder_rsc must contain an \"objects\" directory" );
        
    
    
    
    fs::directory_iterator end_itr;
    for( fs::directory_iterator it( p ); it != end_itr; ++it )
    {
        if( it->path().extension() == ".obj" )
            l.push_back( new Item( it->path().filename().c_str(), Item::OBJECT, it->path().filename() ) );
    }
    
    return l;
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
    
    {   // Initialize Tabs
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
    }
    
    {   // Initialize Items in each Tab
        list<Item*> l = findObjects();
    }
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