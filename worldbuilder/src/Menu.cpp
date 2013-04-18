
#include "arPrecompiled.h"

#include "arGlut.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "Menu.h"

namespace fs = boost::filesystem;

SceneGraph *menuGraph;
std::list<Node*> CopyBuffer;

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


Item::Item( ItemType t, string f, string p, bool selected ) : MenuItem( selected ), filename( f ), path( p ), type( t )
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
		case SCALE_ZERO_TOOL:
			name = "RESET SCALE";
			break;
        default: name = "UNKNOWN"; break;
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
    default: break;
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

void Item::doAction( arSZGAppFramework *fw )
{

    if( type == OBJECT )
    {
        ObjNode *obj = new ObjNode( filename, path );
        
        arAxisAlignedBoundingBox bbox = obj->getAxisAlignedBoundingBox();
        arMatrix4 centerOnHead = ar_TM( arVector3( 0, 0, 0 ) - ar_ET( sg->getRoot()->getNodeTransform() ) ) * fw->getMidEyeMatrix();
        arMatrix4 arbitraryMove = ar_TM( 0, 0, -5 );
        
        arMatrix4 rootscale = sg->getRoot()->getNodeScale();
        
        obj->setNodeTransform( ar_SM( 1 / rootscale.v[0], 1 / rootscale.v[5], 1 / rootscale.v[10] ) * centerOnHead * arbitraryMove  );
        
        sg->addChild( obj );
        interactableObjects.push_back( obj );
    }
    else if( type == TEXTURE )
    {
		for( std::list<arInteractable*>::iterator it = SelectedObjects.begin(); it != SelectedObjects.end(); ++it )
		{
			if( ObjNode *obj = dynamic_cast<ObjNode*>( *it ) )
			{
				arTexture *t = new arTexture();
				t->readJPEG( filename, "", path );
				for( int i = 0; i < obj->numTextures(); ++i )
					obj->setTexture( i, t );
			}
		}
    }
    else if( type == TOOL )
    {
        switch( tooltype )
        {
        case DELETE_TOOL:
            cout << "DELETING object" << endl;
			if( SelectedObjects.size() > 0 )
			{
				while(!SelectedObjects.empty())
				{
					if( Node *n = dynamic_cast<Node*>( SelectedObjects.front() ) )
					{
						if( n != NULL )
						{
							sg->removeChild( n ); 
						}
					}
					SelectedObjects.pop_front();
				}
			}
            break;
        case GROUP_TOOL:
        {
            cout << "GROUPING" << endl;
            
            if( !SelectedObjects.empty() )
            {
                for( list<arInteractable*>::iterator it = SelectedObjects.begin(); it != SelectedObjects.end(); ++it )
                    if( Node *n = dynamic_cast<Node*>( *it ) )
                    {
                        sg->removeChild( n );
                        n->setParent( (Node*)SelectedObjects.front() );
                    }
                
                sg->addChild( (Node*)SelectedObjects.front() );
                for( list<arInteractable*>::iterator it = SelectedObjects.begin(); it != SelectedObjects.end(); ++it )
                {
                    if( *it == SelectedObjects.front() ) continue;
                    if( Node *n = dynamic_cast<Node*>( *it ) )
                    {
                        sg->addChild( n, (Node*)SelectedObjects.front() );
                    }
                }
            }
        }
        break;
        case UNGROUP_TOOL:
            cout << "UNGROUP objects" << endl;
            for( list<arInteractable*>::iterator it = SelectedObjects.begin(); it != SelectedObjects.end(); ++it )
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
            for( list<Node*>::iterator it = CopyBuffer.begin(); it != CopyBuffer.end(); ++it )
            {
                delete *it;
            }
            CopyBuffer.clear();
            for( list<arInteractable*>::iterator it = SelectedObjects.begin(); it != SelectedObjects.end(); ++it )
            {
                if( ObjNode *n = dynamic_cast<ObjNode*>( *it ) )
                {
                    CopyBuffer.push_back( new ObjNode( *n ) );
                }
            }
            break;
        case PASTE_TOOL:
            cout << "PASTING object" << endl;
            for( list<Node*>::iterator it = CopyBuffer.begin(); it != CopyBuffer.end(); ++it )
            {
                (*it)->setNodeTransform( (*it)->getNodeTransform() * ar_TM( -2, 2, 0 ) );
                interactableObjects.push_back( *it );
                sg->addChild( *it );
            }
            CopyBuffer.clear();
            break;
		case SCALE_ZERO_TOOL:
			cout << "SCALE_ZERO" << endl;
			sg->getRoot()->setNodeScale( ar_SM( 1, 1, 1 ) );
			break;
        default: break;
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
    
    glColor3f( CYAN.v[0], CYAN.v[1], CYAN.v[2] );
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
    default: break;
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
    default: break;
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
    default: break;
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
    default: break;
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
            i->doAction( framework );
            return CLOSE;
        }
        break;
    default: break;
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
    menuGraph = new SceneGraph( fw, true );
    MenuNode *menu = new MenuNode( fw );
    
    
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
		tools.push_back( new Item( Item::TOOL, Item::SCALE_ZERO_TOOL ) );
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
    glDisable( GL_DEPTH_TEST );
    arMatrix4 menuPlacement = ar_getNavMatrix() * fw.getMidEyeMatrix() * ar_TM( 0, 0, -5 );
    menu->setNodeTransform( menuPlacement );
    
    menuGraph->drawSceneGraph();
    
    glEnable( GL_DEPTH_TEST );
}
