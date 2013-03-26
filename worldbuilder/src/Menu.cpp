
#include "arPrecompiled.h"

#include "arGlut.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "Menu.h"

namespace fs = boost::filesystem;

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
        tearDownMenu( this );
        currentSelected = (*items)--;
        buildMenu( this );
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
        tearDownMenu( this );
        currentSelected = (*items)++;
        buildMenu( this );
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
                tearDownMenu( this );
                items = objects;
                buildMenu( this );
            }
            else if( t == tabs.materialTab )
            {
                tearDownMenu( this );
                items = textures;
                buildMenu( this );
            }
            else if( t == tabs.toolsTab )
            {
                tearDownMenu( this );
                items = textures;
                buildMenu( this );
            }
            return REDRAW;
        }
        break;
    case ITEM:
        if( Item *i = dynamic_cast<Item*>( currentSelected ) )
        {
            if( i->type == Item::OBJECT )
            {
                ObjNode *obj = new ObjNode( i->getFilename(), i->getPath() );
                sg->addChild( obj );
                obj->setNodeTransform( primary.getMatrix() );
                interactableObjects.push_back( obj );
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
    }
    return menu;
}

void buildMenu( MenuNode *menu )
{
    sg->addChild( menu );
    sg->addChild( menu->tabs.objectTab, menu );
    sg->addChild( menu->tabs.materialTab, menu );
    sg->addChild( menu->tabs.toolsTab, menu );
    
    float i = -1.5;
    for( list<Item*>::iterator it = menu->items->itemlist.begin(); it != menu->items->itemlist.end() && i < 2; ++it, i += 1.5 )
    {
        (*it)->setNodeTransform( ar_TM( i, 0, 0 ) );
        sg->addChild( *it, menu );
    }
}

void tearDownMenu( MenuNode *menu )
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