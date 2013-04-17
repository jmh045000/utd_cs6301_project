
#include "arPrecompiled.h"

#include <stdint.h>

#include <iostream>

#include "Node.h"

#include "arGlut.h"
#include "arNavigationUtilities.h"
#include "arSoundAPI.h"
#include "arSZGAppFramework.h"

uint64_t Node::numObjects_ = 0;

void Node::posGrab( arEffector *g )
{ 
    if( parentNode_ == this )
        posGrabbers_.insert( g );
    else
        parentNode_->posGrab( g );
}

void Node::rotGrab( arEffector *g )
{ 
    if( parentNode_ == this )
        rotGrabbers_.insert( g );
    else
        parentNode_->rotGrab( g );
}

void Node::scaleGrab( arEffector *g )
{
	if( parentNode_ == this )
		scaleGrabbers_.insert( g );
	else
		parentNode_->scaleGrab( g );
}

void Node::ungrab( arEffector *g )
{
    if( parentNode_ == this )
	{
        posGrabbers_.erase( g );
		rotGrabbers_.erase( g );
		scaleGrabbers_.erase( g );
	}
    else
        parentNode_->ungrab( g );
}

void Node::drawBegin( arMatrix4 &currentView, arMatrix4 &currentScale )
{
    if( posGrabbers_.size() == 1 )
    {
        arVector3 curEffRotation, curEffPosition;
        arEffector *eff = *(posGrabbers_.begin());
        if( !posGrabbed )
        {
            origEffPosition = ar_ET( eff->getMatrix() );
            posGrabbed = true;
        }
        
        curEffPosition = ar_ET( eff->getMatrix() );
        
        translation = curEffPosition - origEffPosition;
    }
    else if( rotGrabbers_.size() == 1 )
	{
		arVector3 curEffRotation;
		
		arEffector *eff = *(rotGrabbers_.begin());
        if( !rotGrabbed )
        {
            origEffRotation = ar_ER( eff->getMatrix(), AR_XYZ );
            rotGrabbed = true;
        }
        
        curEffRotation = ar_ER( eff->getMatrix(), AR_XYZ );
        
        rotation = curEffRotation - origEffRotation;
	}
	else if( scaleGrabbers_.size() == 2 )
	{
		
	}
	else
    {
        if(  posGrabbed )
        {
            posGrabbed = false;
            nodeTransform = ar_TM( translation ) * nodeTransform;
            translation = arVector3();
            origEffPosition = arVector3();
        }
		if(  rotGrabbed )
        {
            rotGrabbed = false;
            nodeTransform = nodeTransform * arEulerAngles( AR_XYZ, rotation ).toMatrix();
            rotation = arVector3();
            origEffRotation = arVector3();
        }
    }
        
    glPushMatrix();
        glMultMatrixf( ( ar_TM( translation ) * nodeTransform * arEulerAngles( AR_XYZ, rotation ).toMatrix() ).v );
        
        currentView = currentView * ar_TM( translation ) * nodeTransform * arEulerAngles( AR_XYZ, rotation ).toMatrix();
        currentScale = currentScale * ar_ESM( nodeTransform );
        actualPosition = currentView;
}

void Node::drawEnd( arMatrix4 &currentView, arMatrix4 &currentScale )
{
    glPopMatrix();
    currentView = currentView * arEulerAngles( AR_XYZ, rotation ).toMatrix().inverse() * nodeTransform.inverse() * ar_TM( translation ).inverse();
    currentScale = currentScale * ar_ESM( nodeTransform ).inverse();
    //std::cout << "drawEnd, currentView=\n" << currentView << std::endl;
}

void Node::drawLocalBegin( arMatrix4 &currentView, arMatrix4 &currentScale )
{
    if( opengl_callback != NULL )
    {
        opengl_callback();
    }
    glPushMatrix();
        //glMultMatrixf( currentScale.v );
        glColor3f( color[0], color[1], color[2] );
        
        if( soundFile != "" )
        {
            arVector3 tmp = ar_ET( ar_getNavMatrix( ) );
            tmp -= ar_ET( currentView );
            float loudness = 1.0 - (tmp.magnitude() / 100.0 );
            
            arVector3 soundPosition = ar_ET( currentView ) - ar_ET( ar_getNavMatrix() );
            
            dsLoop( soundId_, soundFile, soundState_, loudness, soundPosition );
        }
        
        if( touched() )
        {
            if( ObjNode *o = dynamic_cast<ObjNode*>( this ) )
            {
                arBoundingSphere sphere = o->getBoundingSphere();
                glPushMatrix();
                    glMultMatrixf( (ar_TM(sphere.position)).v );
                    selected_ ? glColor3f( 0, 0, 1 ) : glColor3f(1.0, 1.0, 0.0);
                    glutWireSphere(sphere.radius, 16, 16);
                glPopMatrix();
            }
        }
        
        
        if( !(!texture) ) texture.activate();
}

void Node::drawLocalEnd( arMatrix4 &currentView, arMatrix4 &currentScale )
{
    glPopMatrix(); // currentScale
    
    //std::cout << "drawLocalEnd, currentView=\n" << currentView << std::endl;
    if( !(!texture) ) texture.deactivate();
}

void Node::setSound( string filename )
{
    if( soundFile != "" )
    {
        dsErase( soundFile );
    }
    soundFile = filename;
    string soundNodeName = soundFile;
    soundNodeName += id;
    soundId_ = dsLoop( soundNodeName, "root", soundFile, 0, 1, arVector3() );
    if( soundId_ == -1 )
    {
        //std::cerr << "Error creating sound!" << std::endl;
        soundFile = "";
    }
}

void Node::setTexture( string filename, string subdirectory, string path )
{
    if( filename.find( ".jpg" ) != string::npos || filename.find( ".jpeg" ) != string::npos )
    {
        texture.readJPEG( filename, subdirectory, path );
    }

    glEnable( GL_TEXTURE_2D );
}
    
RootNode::RootNode( arSZGAppFramework &fw ) : Node(), fw_( fw )
{
    // Set up the root sound node, this way, all sound has a good root position
    soundId_ = dsTransform("root", fw.getNavNodeName(), ar_SM(1.0) );
    if( soundId_ == -1 )
    {
        std::cerr << "Failed to create root sound node" << std::endl;
    }
}

void RootNode::drawBegin( arMatrix4 &currentView, arMatrix4 &currentScale )
{
    glPushMatrix();
        glMultMatrixf( ( transform_ * scale_ ).v );
        currentView = currentView * transform_ * scale_;
        currentScale = currentScale * scale_;
        
    dsTransform( soundId_, transform_ );
}

void RootNode::drawEnd( arMatrix4 &currentView, arMatrix4 &currentScale )
{
    glPopMatrix();
        currentView = currentView * transform_.inverse() * scale_.inverse();
        currentScale = currentScale * scale_.inverse();
}

SolidSphereNode::SolidSphereNode( double radius, int slices, int stacks, bool wireframe ) : SolidNode( SPHERE, wireframe ), radius_( radius ), slices_( slices ), stacks_( stacks )
{
    sphere_ = gluNewQuadric();
    
    gluQuadricTexture(sphere_, GL_TRUE);
    gluQuadricNormals(sphere_, GLU_SMOOTH);
}

void SolidSphereNode::draw()
{
    if( wireframe_ )
        gluQuadricDrawStyle(sphere_, GLU_LINE);
    else
        gluQuadricDrawStyle(sphere_, GLU_FILL);
        
    gluSphere( sphere_, radius_, slices_, stacks_ );
}

void drawBox( GLfloat size, GLenum type )
{
    static const GLfloat n[6][3] =
    {
        {-1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0},
        {0.0, 0.0, 1.0},
        {0.0, 0.0, -1.0}
    };
    static const GLint faces[6][4] =
    {
        {0, 1, 2, 3},
        {3, 2, 6, 7},
        {7, 6, 5, 4},
        {4, 5, 1, 0},
        {5, 6, 2, 1},
        {7, 4, 0, 3}
    };
    GLfloat v[8][3];
    GLint i;

    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

    for (i = 5; i >= 0; i--) {
        glBegin(type);
        glNormal3fv(&n[i][0]);
        glTexCoord2f(0.0f, 0.0f); glVertex3fv(&v[faces[i][0]][0]);
        glTexCoord2f(1.0f, 0.0f); glVertex3fv(&v[faces[i][1]][0]);
        glTexCoord2f(1.0f, 1.0f); glVertex3fv(&v[faces[i][2]][0]);
        glTexCoord2f(0.0f, 1.0f); glVertex3fv(&v[faces[i][3]][0]);
        glEnd();
    }

}

/* From http://www.opengl.org/resources/libraries/glut/ */
void SolidCubeNode::draw()
{
    
    if( wireframe_ )
        drawBox( size_, GL_LINE_LOOP );
    else
        drawBox( size_, GL_QUADS );
    
}

SolidCylinderNode::SolidCylinderNode( double base, double top, double height, int slices, int stacks, bool wireframe ) :
    SolidNode( CONE, wireframe ), base_( base ), top_( top ), height_( height ), slices_( slices ), stacks_( stacks )
{
    cylinder_ = gluNewQuadric();
    
    gluQuadricTexture(cylinder_, GL_TRUE);
    gluQuadricNormals(cylinder_, GLU_SMOOTH);
}

void SolidCylinderNode::draw()
{
    if( wireframe_ )
        gluQuadricDrawStyle(cylinder_, GLU_LINE);
    else
        gluQuadricDrawStyle(cylinder_, GLU_FILL);
        
    gluCylinder( cylinder_, base_, top_, height_, slices_, stacks_ );
}

void SolidTeapotNode::draw()
{
    if( wireframe_ )
        glutWireTeapot( size_ );
    else
        glutSolidTeapot( size_ );
}

void wbOBJRenderer::setTexture( unsigned i, arTexture *t )
{
	if( i < _textures.size() )
	{
		_textures[i] = t;
	}
}

ObjNode::ObjNode( ObjNode &otherObj ) : Node( otherObj.nodeTransform ), filename_ ( otherObj.filename_ ), path_( otherObj.path_ )
{
    valid = obj_.readOBJ( filename_, path_ );
    
    for( int i = 0; i < numTextures(); ++i )
        setTexture( i, otherObj.obj_.getTexture( i ) );
}

void ObjNode::draw()
{
    if(!valid)
        return;

    if( !(!texture) ) std::cerr << "You shouldn't be setting a texture on an ObjNode, use the OBJ format" << std::endl;
    obj_.draw();
}

