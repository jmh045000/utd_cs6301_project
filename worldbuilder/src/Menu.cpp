
#include "arPrecompiled.h"

#include "arGlut.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "Menu.h"

namespace fs = boost::filesystem;

SceneGraph *menuGraph;

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
                    v[i][0] += 0.05;
                else
                    v[i][0] -= 0.05;
                if( v[i][1] > 0 )
                    v[i][1] += 0.05;
                else
                    v[i][1] -= 0.05;
                glVertex3fv( v[i] );
            }
        glEnd();
    }
}


Item::Item( ItemType t, string f, string p, bool selected ) : MenuItem( selected ), type( t ), filename( f ), path( p )
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

Item::Item( ItemType t, ToolType t2, bool selected ) : MenuItem( selected ), type( t ), tooltype( t2 )
{
    switch( tooltype )
    {
        case DELETE_TOOL:
            name = "DELETE";
            break;
        case GROUP_TOOL:
            name = "GROUP";
            break;
        case UNGROUP_TOOL:
            name = "UNGROUP";
            break;
        case COPY_TOOL:
            name = "COPY";
            break;
        case PASTE_TOOL:
            name = "PASTE";
            break;
    }
}

void Item::draw()
{
    glColor3f( 1, 1, 1 );
    float size = 0.8;
    GLfloat v[4][3] =
    {
        { -size / 2, -size / 2, 0.0001 },
        { size / 2, -size / 2, 0.0001 },
        { size / 2, size / 2, 0.0001 },
        { -size / 2, size / 2, 0.0001 }
    };
 
	switch( type )
	{
	case OBJECT:
	{
        arBoundingSphere s = obj.getBoundingSphere();
        float scale = size / ( s.radius * 2 );
        glPushMatrix();
			glMultMatrixf( ar_SM( scale, scale, scale ).v );
			obj.draw();
		glPopMatrix();
	}
	break;
    case TEXTURE:
    {
        texture.activate();
        glBegin( GL_QUADS );
            glTexCoord2f( 0, 0 ); glVertex3fv( v[0] );
            glTexCoord2f( 1, 0 ); glVertex3fv( v[1] );
            glTexCoord2f( 1, 1 ); glVertex3fv( v[2] );
            glTexCoord2f( 0, 1 ); glVertex3fv( v[3] );
        glEnd();
		texture.deactivate();
    }
	break;
	case TOOL:
	{
		glColor3f( 0, 0.3, 0.8 );
		glBegin( GL_QUADS );
            glVertex3fv( v[0] );
            glVertex3fv( v[1] );
            glVertex3fv( v[2] );
            glVertex3fv( v[3] );
        glEnd();
		glColor3f( 0, 0, 0 );
		glPushMatrix();
            int len = strlen( name );
			glTranslatef( -( (float)(size+1) / (float)len ), 0, 0.001);
			glScalef(0.0009, 0.0009, 0.0009);
			for (const char* c = name; *c; ++c)
			{
				glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
			}
		glPopMatrix();
	}
	break;
	}
    
    if( selected )
    {
        glColor3f( 0, 0, 0 );
        glBegin( GL_LINE_LOOP );
            for( int i = 0; i < 4; i++ )
            {
                if( v[i][0] > 0 )
                    v[i][0] += 0.05;
                else
                    v[i][0] -= 0.05;
                if( v[i][1] > 0 )
                    v[i][1] += 0.05;
                else
                    v[i][1] -= 0.05;
                glVertex3fv( v[i] );
            }
        glEnd();
    }
}

void Item::doAction()
{

    if( type == OBJECT )
    {
        ObjNode *obj = new ObjNode( filename, path );
        sg->addChild( obj );
        obj->setNodeTransform( primary.getMatrix() );
        interactableObjects.push_back( obj );
    }
    else if( type == TEXTURE )
    {
        if( ObjNode *obj = dynamic_cast<ObjNode*>( interactableObjects.front() ) )
        {
            arTexture *t = new arTexture();
            t->readJPEG( filename, "", path );
            for( int i = 0; i < obj->numTextures(); ++i )
                obj->setTexture( i, t );
        }
    }
    else if( type == TOOL )
    {
        switch( tooltype )
        {
        case DELETE_TOOL:
            cout << "DELETING object" << endl;
            if( Node *n = dynamic_cast<Node*>( interactableObjects.front() ) )
            {
                interactableObjects.pop_front();
                sg->removeChild( n );
            }
            break;
        case GROUP_TOOL:
        {
            cout << "GROUPING" << endl;
            NothingNode *nothing = new NothingNode();
            
            for( list<arInteractable*>::iterator it = interactableObjects.begin(); it != interactableObjects.end(); ++it )
                if( Node *n = dynamic_cast<Node*>( *it ) )
                {
                    sg->removeChild( n );
                    n->setParent( nothing );
                }
            
            sg->addChild( nothing );
            for( list<arInteractable*>::iterator it = interactableObjects.begin(); it != interactableObjects.end(); ++it )
            {
                if( Node *n = dynamic_cast<Node*>( *it ) )
                {
                    sg->addChild( n, nothing );
                }
            }
        }
        break;
        case UNGROUP_TOOL:
            cout << "UNGROUP objects" << endl;
            for( list<arInteractable*>::iterator it = interactableObjects.begin(); it != interactableObjects.end(); ++it )
            {
                if( Node *n = dynamic_cast<Node*>( *it ) )
                {
                    sg->removeChild( n );
                    n->setParent( n );
                    sg->addChild( n );
                }
            }
            break;
        case COPY_TOOL:
            cout << "COPYING object" << endl;
            break;
        case PASTE_TOOL:
            cout << "PASTING object" << endl;
            break;
        }
    }
}

void MenuNode::draw()
{
    float hsize = 4;
    float vsize = 2;
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
                items = tools;
                buildMenu( this );
            }
            return REDRAW;
        }
        break;
    case ITEM:
        if( Item *i = dynamic_cast<Item*>( currentSelected ) )
        {
            i->doAction();
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
        
        if( it->path().extension() == ".obj" )
        {
            stringstream ss;
            ss << p;
            cout << "Loading object: " << it->path().filename() << endl;
            l.push_back( new Item( Item::OBJECT, it->path().filename(), ss.str() ) );
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
            cout << "Loading Texture: " << it->path().filename() << endl;
            l.push_back( new Item( Item::TEXTURE, it->path().filename(), ss.str() ) );
        }
    }
    
    return l;
}

MenuNode* initMenu( arSZGAppFramework &fw )
{
    menuGraph = new SceneGraph( fw );
    MenuNode *menu = new MenuNode();
    menu->setColor( CYAN );
    
    {   // Initialize Tabs
        Tab *objectTab = new Tab( "Objects", true );
        objectTab->setNodeTransform( ar_TM( -1.4, 0.5, 0 ) );
        menu->tabs.objectTab = objectTab;
        menu->currentSelected = objectTab;
        
        Tab *materialTab = new Tab( "Materials" );
        materialTab->setNodeTransform( ar_TM( 0, 0.5, 0 ) );
        menu->tabs.materialTab = materialTab;
        
        Tab *toolsTab = new Tab( "Tools" );
        toolsTab->setNodeTransform( ar_TM( 1.4, 0.5, 0 ) );
        menu->tabs.toolsTab = toolsTab;
    }
    
    {   // Initialize Items in each Tab
        menu->setObjects( findObjects() );
        menu->setTextures( findTextures() );
		
		list<Item*> tools;
		tools.push_back( new Item( Item::TOOL, Item::DELETE_TOOL ) );
		tools.push_back( new Item( Item::TOOL, Item::GROUP_TOOL ) );
        tools.push_back( new Item( Item::TOOL, Item::UNGROUP_TOOL ) );
		tools.push_back( new Item( Item::TOOL, Item::COPY_TOOL ) );
		tools.push_back( new Item( Item::TOOL, Item::PASTE_TOOL ) );
		menu->setTools( tools );
		
        menu->items = menu->objects;
    }
    return menu;
}

void buildMenu( MenuNode *menu )
{
    menuGraph->addChild( menu );
    menuGraph->addChild( menu->tabs.objectTab, menu );
    menuGraph->addChild( menu->tabs.materialTab, menu );
    menuGraph->addChild( menu->tabs.toolsTab, menu );
    
    float i = -1.4;
    for( list<Item*>::iterator it = menu->items->itemlist.begin(); it != menu->items->itemlist.end() && i < 2; ++it, i += 1.4 )
    {
        (*it)->setNodeTransform( ar_TM( i, -0.3, 0 ) );
        menuGraph->addChild( *it, menu );
    }
}

void tearDownMenu( MenuNode *menu )
{
    int i = 0;
    list<Item*> l;
    for( list<Item*>::iterator it = menu->items->itemlist.begin(); it != menu->items->itemlist.end() && i++ < 3; ++it )
        l.push_front( *it );
    for( list<Item*>::iterator it = l.begin(); it != l.end(); ++it )
        menuGraph->removeChild( *it );
    
    menuGraph->removeChild( menu->tabs.toolsTab );
    menuGraph->removeChild( menu->tabs.materialTab );
    menuGraph->removeChild( menu->tabs.objectTab );
    menuGraph->removeChild( menu );
}

void drawMenu( MenuNode *menu, arSZGAppFramework &fw )
{
    arMatrix4 menuPlacement = ar_getNavMatrix() * fw.getMidEyeMatrix() * ar_TM( 0, 0, -5 );
    menu->setNodeTransform( menuPlacement );
    
    menuGraph->drawSceneGraph();
}