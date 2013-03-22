
#include "arPrecompiled.h" // This HAS to be first... ugh


#include <list>
#include <vector>
#include <sstream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

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
    
    void deselect() { selected = false; }
    void select() { selected = true; }
    
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
    string filename;
    string path;
    arOBJRenderer obj;
    arTexture texture;
    void draw();
public:
    Item( const char *name, ItemType t, string f, string p, bool selected = false );
    
    ItemType type;
    
    friend struct _ItemGroup_s;
};

Item::Item( const char *name, ItemType t, string f, string p, bool selected ) : MenuItem( name, selected ), type( t ), filename( f ), path( p )
{
    if( type == OBJECT )
    {
        obj.readOBJ( filename, path );
    }
    else if( type == TEXTURE )
    {
        texture.readJPEG( filename, "", path );
        texture.repeating( true );
    }
}

void Item::draw()
{
    glColor3f( 1, 1, 1 );
    float size = 1;
    GLfloat v[4][3] =
    {
        { -size / 2, -size / 2, 0.0001 },
        { size / 2, -size / 2, 0.0001 },
        { size / 2, size / 2, 0.0001 },
        { -size / 2, size / 2, 0.0001 }
    };
    
    if( type == OBJECT )
    {
        arBoundingSphere s = obj.getBoundingSphere();
        float scale = size / ( s.radius * 2 );
        glPushMatrix();
        glMultMatrixf( ar_SM( scale, scale, scale ).v );
        obj.draw();
    }
    else if( type == TEXTURE )
    {
        texture.activate();
        glBegin( GL_QUADS );
            glTexCoord2f( 0, 0 ); glVertex3fv( v[0] );
            glTexCoord2f( 1, 0 ); glVertex3fv( v[1] );
            glTexCoord2f( 1, 1 ); glVertex3fv( v[2] );
            glTexCoord2f( 0, 1 ); glVertex3fv( v[3] );
        glEnd();
    }
    
    if( type == OBJECT )
        glPopMatrix();
    else if( type == TEXTURE )
        texture.deactivate();
    
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

typedef struct _MenuGroup_s
{
    virtual string name() = 0;
    virtual MenuItem* first() = 0;
    virtual MenuItem* operator++(int) = 0;
    virtual MenuItem* operator--(int) = 0;
} MenuGroup_t;

typedef struct _TabGroup_s : public MenuGroup_t
{
    int cur;
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
        switch( cur )
        {
        case 0:
            cur = 1;
            return materialTab;
        case 1:
            cur = 2;
            return toolsTab;
        case 2:
            cur = 0;
            return objectTab;
        }
    }
    
    MenuItem* operator--(int)
    {
        switch( cur )
        {
        case 0:
            cur = 2;
            return toolsTab;
        case 1:
            cur = 0;
            return objectTab;
        case 2:
            cur = 1;
            return materialTab;
        }
    }
    
    _TabGroup_s() : MenuGroup_t(), cur( 0 ) {}
} TabGroup;

typedef struct _ItemGroup_s : public MenuGroup_t
{
    int cur;
    list<Item*> itemlist;
    
    string name()
    {
        return "ItemGroup";
    }
    
    MenuItem* first()
    {
        return itemlist.front();
    }
    
    MenuItem* operator++ (int)
    {
        int _ = 0;
        Item *i;
        switch( cur )
        {
        case 0:
            cur = 1;
            for( list<Item*>::iterator it = itemlist.begin(); it != itemlist.end() && _++ <= cur; ++it )
                i = *it;
            return i;
        case 1:
            cur = 2;
            for( list<Item*>::iterator it = itemlist.begin(); it != itemlist.end() && _++ <= cur; ++it )
                i = *it;
            return i;
        case 2:
            cur = 0;
            for( int _ = 0; _ < 3; _++ )
            {
                Item *i = itemlist.front();
                itemlist.push_back( i );
                itemlist.pop_front();
            }
            return itemlist.front();
        }
    }
    
    MenuItem* operator-- (int)
    {
        int _ = 0;
        Item *i;
        switch( cur )
        {
        case 0:
            cur = 2;
            for( _ = 0; _ < 3; _++ )
            {
                i = itemlist.back();
                itemlist.push_front( i );
                itemlist.pop_back();
            }
            for( list<Item*>::iterator it = itemlist.begin(); it != itemlist.end() && _++ <= cur; ++it )
                i = *it;
            return i;
        case 1:
            cur = 0;
            return itemlist.front();
        case 2:
            cur = 1;
            for( list<Item*>::iterator it = itemlist.begin(); it != itemlist.end() && _++ <= cur; ++it )
                i = *it;
            return i;
        }
    }
    
    _ItemGroup_s( list<Item*> i ) : MenuGroup_t(), cur( 0 ), itemlist( i ) {}
    
} ItemGroup;

typedef enum { NONE, REDRAW, CLOSE, MAX } MenuAction;

class MenuNode : public Node
{
private:
    typedef enum { TAB, ITEM, MAX_GROUPS } SelectedGroup;
    
    TabGroup tabs;
    ItemGroup *items;
    
    SelectedGroup selectedGroup;
    MenuItem *currentSelected;
    
    ItemGroup *objects;
    ItemGroup *textures;
    
    SceneGraph *scenegraph;
    
    void draw();
public:
    MenuNode( ) : Node(), selectedGroup( TAB ), currentSelected( NULL )  {}
    
    void setObjects( list<Item*> l ) { objects = new ItemGroup( l ); 
        for(list<Item*>::const_iterator it = l.begin(); it != l.end(); ++it )
            cout << "Object=" << (*it)->name << endl;
    }
    void setTextures( list<Item*> l ) { textures = new ItemGroup( l ); 
        for(list<Item*>::const_iterator it = l.begin(); it != l.end(); ++it )
            cout << "Texture=" << (*it)->name << endl;
    }
    
    void pressedDown();
    void pressedLeft();
    void pressedRight();
    void pressedUp();
    MenuAction pressedA();
    
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
    currentSelected->deselect();
    switch( selectedGroup )
    {
    case TAB:
        currentSelected = tabs.first();
        break;
    case ITEM:
        currentSelected = items->first();
        break;
    }
    currentSelected->select();
}

void MenuNode::pressedLeft()
{
    currentSelected->deselect();
    switch( selectedGroup )
    {
    case TAB:
        currentSelected = tabs--;
        break;
    case ITEM:
        currentSelected = (*items)--;
        break;
    }
    currentSelected->select();
}

void MenuNode::pressedRight()
{
    currentSelected->deselect();
    switch( selectedGroup )
    {
    case TAB:
        currentSelected = tabs++;
        break;
    case ITEM:
        currentSelected = (*items)++;
        break;
    }
    currentSelected->select();
}

void MenuNode::pressedUp()
{
    selectedGroup = ++selectedGroup;
    currentSelected->deselect();
    switch( selectedGroup )
    {
    case TAB:
        currentSelected = tabs.first();
        break;
    case ITEM:
        currentSelected = items->first();
        break;
    }
    currentSelected->select();
}

MenuAction MenuNode::pressedA()
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
            if( t == tabs.objectTab )
            {
                tearDownMenu();
                items = objects;
                buildMenu();
            }
            else if( t == tabs.materialTab )
            {
                tearDownMenu();
                items = textures;
                buildMenu();
            }
            else if( t == tabs.toolsTab )
            {
                tearDownMenu();
                items = textures;
                buildMenu();
            }
            return REDRAW;
        }
        break;
    case ITEM:
        if( Item *i = dynamic_cast<Item*>( currentSelected ) )
        {
            if( i->type == Item::OBJECT )
            {
                
            }
            return CLOSE;
        }
        break;
    }
    
    return NONE;
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
        char *n = new char[it->path().filename().length()];
        strcpy( n, it->path().filename().c_str() );
        if( it->path().extension() == ".obj" )
        {
            stringstream ss;
            ss << p;
            l.push_back( new Item( n, Item::OBJECT, it->path().filename(), ss.str() ) );
        }
    }
    
    return l;
}

list<Item*> findTextures()
{
    list<Item*> l;
    
    fs::path p( "./worldbuilder_rsc/" );
    if( !fs::exists( p ) )
        throw( "Must run in directory with worldbuilder_rsc" );
    
    p /= "textures";
    if( !fs::exists( p ) )
        throw( "worldbuilder_rsc must contain an \"objects\" directory" );
    
    fs::directory_iterator end_itr;
    for( fs::directory_iterator it( p ); it != end_itr; ++it )
    {
        if( it->path().extension() == ".jpg" || it->path().extension() == ".jpeg" )
        {
            stringstream ss;
            ss << p;
            l.push_back( new Item( it->path().filename().c_str(), Item::TEXTURE, it->path().filename(), ss.str() ) );
        }
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
        menu->setObjects( findObjects() );
        menu->setTextures( findTextures() );
        menu->items = menu->objects;
        
        cout << "objectlist.size() = " << menu->objects->itemlist.size() << endl;
        
        int i = 0;
        for( list<Item*>::iterator it = menu->objects->itemlist.begin(); it != menu->objects->itemlist.end(); ++it )
        {
            switch( i )
            {
            case 0:
                (*it)->setNodeTransform( ar_TM( -1.5, 0, 0 ) );
                break;
            case 1:
                (*it)->setNodeTransform( ar_TM( 0, 0, 0 ) );
                break;
            case 2:
                (*it)->setNodeTransform( ar_TM( 1.5, 0, 0 ) );
                break;
            }
            i = (i+1) % 3;
        }
        
        i = 0;
        for( list<Item*>::iterator it = menu->textures->itemlist.begin(); it != menu->textures->itemlist.end(); ++it )
        {
            switch( i )
            {
            case 0:
                (*it)->setNodeTransform( ar_TM( -1.5, 0, 0 ) );
                break;
            case 1:
                (*it)->setNodeTransform( ar_TM( 0, 0, 0 ) );
                break;
            case 2:
                (*it)->setNodeTransform( ar_TM( 1.5, 0, 0 ) );
                break;
            }
            i = (i+1) % 3;
        }
    }
    return menu;
}

void buildMenu()
{
    sg->addChild( menu );
    sg->addChild( menu->tabs.objectTab, menu );
    sg->addChild( menu->tabs.materialTab, menu );
    sg->addChild( menu->tabs.toolsTab, menu );
    
    int i = 0;
    for( list<Item*>::iterator it = menu->items->itemlist.begin(); it != menu->items->itemlist.end() && i++ < 3; ++it )
        sg->addChild( *it, menu );
}

void tearDownMenu()
{
    int i = 0;
    list<Item*> l;
    for( list<Item*>::iterator it = menu->items->itemlist.begin(); it != menu->items->itemlist.end() && i++ < 3; ++it )
        l.push_front( *it );
    for( list<Item*>::iterator it = l.begin(); it != l.end(); ++it )
        sg->removeChild( *it );
    
    sg->removeChild( menu->tabs.toolsTab );
    sg->removeChild( menu->tabs.materialTab );
    sg->removeChild( menu->tabs.objectTab );
    sg->removeChild( menu );
}

bool initSceneGraph( arMasterSlaveFramework &fw, arSZGClient &client )
{
    sg = new SceneGraph( fw );
    
    menu = initMenu();
    
    ObjNode *obj = new ObjNode( "al.obj", "worldbuilder_rsc/objects" );
    obj->setNodeTransform( ar_TM( 0, 5, -5 ) );
    
    
    cout << "teapot: number of materials=" << obj->obj_.getNumberMaterials() << endl;
    cout << "teapot: number of textures=" << obj->obj_.getNumberTextures() << endl;
    
    
    arTexture *t = obj->obj_.getTexture( 4 );
    cout << "texture*=" << t << endl;
    if( t )
    {
        t->readJPEG( "brick.jpg", "", "worldbuilder_rsc/textures" );
        cout << "Texture is good!" << endl;
    }
    else
    {
        cerr << "Failed to set texture!" << endl;
    }
    /**/
    
    arMaterial *m = obj->obj_.getMaterial( 2 );
    cout << "material*=" << m << endl;
    if( m )
    {
        m->diffuse = arVector3( 0, 0, 1 );
        m->ambient = arVector3( 0, 0, 1 );
        m->activateMaterial();
    }
    else
    {
        cerr << "Failed to set material " << 2 << "!" << endl;
    }
    
    m = obj->obj_.getMaterial( 3 );
    m = 0;
    t = new arTexture();
    
    t->readJPEG( "rainbow.jpg", "", "worldbuilder_rsc/textures" );
    
    obj->obj_.setTexture( 3, t );
    
    //obj->obj_.activateTextures();
    
    for( int i = 0; i < obj->obj_.getNumberGroups(); i++ )
    {
        arOBJGroupRenderer *g = obj->obj_.getGroup( i );
        cout << "Found group: " << g->getName() << endl;
    }
    
    sg->addChild( obj );
    
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
            switch( menu->pressedA() )
            {
            case REDRAW:
                tearDownMenu();
                buildMenu();
                break;
            case CLOSE:
                tearDownMenu();
                menuOn = false;
                break;
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