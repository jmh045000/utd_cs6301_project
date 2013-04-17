
#ifndef _UTD_SCENEGRAPH_NODE_H_
#define _UTD_SCENEGRAPH_NODE_H_

#include <string>
#include <iostream>
#include <set>

#include <stdint.h>

#include "arInteractable.h"
#include "arMath.h"
#include "arOBJ.h"

typedef uint64_t NodeIdType;

static const arVector3 BLACK( 0, 0, 0 );
static const arVector3 WHITE( 1, 1, 1 );
static const arVector3 GREY( 0.5, 0.5, 0.5 );

static const arVector3 RED( 1, 0, 0 );
static const arVector3 GREEN( 0, 1, 0 );
static const arVector3 BLUE( 0, 0, 1 );

static const arVector3 YELLOW( 1, 1, 0 );
static const arVector3 VIOLET( 1, 0, 1 );
static const arVector3 CYAN( 0, 1, 1 );

static const arVector3 ORANGE( 1, 0.5, 0 );
static const arVector3 PURPLE( 0.75, 0, 1 );
static const arVector3 PINK( 1, 0, 0.75 );

typedef void (*custom_opengl_callback)(void);

typedef enum { SOUNDPLAYONCE = -1, SOUNDSTOP = 0, SOUNDPLAYCONTINOUS = 1 } SoundState;

class Node : public arInteractable
{
    friend class SceneGraph;
    friend class RootNode;

    static NodeIdType numObjects_;
private:
    int soundId_;
    SoundState soundState_; // What are we doing with current sound?
    
    Node *parentNode_; // This is used to group objects together
	
	bool selected_;
    
    std::set<arEffector*> rotGrabbers_, posGrabbers_, scaleGrabbers_;
    arVector3 origEffRotation, origEffPosition, origEffScale;
    arVector3 rotation, translation, scaling;
    float originalDistX, originalDistY, originalDistZ;
    bool rotGrabbed, posGrabbed, scaleGrabbed;
    
    // drawBegin_ is the function that will do the pushing, and multiply by our transform
    virtual void drawBegin( arMatrix4 &currentView, arMatrix4 &currentScale ); // RootNode overrides this, no others should
    // drawEnd_ will pop the matrix
    virtual void drawEnd( arMatrix4 &currentView, arMatrix4 &currentScale ); // RootNode overrides this, no others should
    
    void drawLocalBegin( arMatrix4 &currentView, arMatrix4 &currentScale );
    void drawLocalEnd( arMatrix4 &currentView, arMatrix4 &currentScale );
protected:
    
    // This is the transform matrix that this node and all children will have
    arMatrix4               nodeTransform;
    
    // This allows a developer to create a callback function that gets called during draw time
    custom_opengl_callback  opengl_callback;
    
    // A filename representing a sound that will be played by this node
    std::string             soundFile; // To play a sound with this node
    
    
    // A color that will be applied to this node
    arVector3               color;
    // A texture that will be applied to this node
    arTexture               texture; // Filename to the texture (I have no idea how to do this...)
    
    bool highlight;
    arMatrix4 actualPosition;
    
    // The generic draw functions
    // General flow: push_matrix, multi(node_transform), foreach c in child do c.draw(), opengl_callback(), color(), texture(), mydraw()

    virtual void draw() = 0;

public:
    // *structors
    Node() : arInteractable(), parentNode_( this ), selected_( false ), rotation(), translation(), rotGrabbed( false ), posGrabbed( false ), scaleGrabbed( false ), opengl_callback( NULL ), color( 0, 0, 0 ), highlight( false ), id( ++numObjects_ )
    {
    }
    
    Node( arMatrix4 &tm ) : arInteractable(), parentNode_( this ), selected_( false ), rotation(), translation(), rotGrabbed( false ), posGrabbed( false ), scaleGrabbed( false ), nodeTransform( tm ), opengl_callback( NULL ), color( 0, 0, 0 ), highlight( false ), id( ++numObjects_ ) {}
    virtual ~Node() {}
    
    // A globally unique id for this node.
    const NodeIdType          id;
    
    // Functions to change internal variables (matrices and colors...)
    void setNodeTransform( const arMatrix4 &mat ) { nodeTransform = mat; }
    
    arMatrix4& getNodeTransform() { return nodeTransform; }
    
    void setColor( const arVector3 &v ) { color = v; }
    
    void setSound( std::string filename );
    void soundPlayOnce( ) { soundState_ = SOUNDPLAYONCE; }
    void soundPlayCont( ) { soundState_ = SOUNDPLAYCONTINOUS; }
    void soundStop( ) { soundState_ = SOUNDSTOP; }
    
    void setTexture( std::string filename, std::string subdirectory = "", std::string path = "" );
    
    void setParent( Node *n ) 
    {
        posGrabbers_.clear();
		rotGrabbers_.clear();
		scaleGrabbers_.clear();
        if( n != this )
        {
            nodeTransform = nodeTransform * n->nodeTransform.inverse();
        }
        if( parentNode_ != this )
        {
                nodeTransform = nodeTransform * parentNode_->nodeTransform;
        }
        parentNode_ = n;
    }
    
    Node *getParent() { return parentNode_; }
    
    void setHighlight( bool h ) { highlight = h; }
    bool getHighlight() { return highlight; }
    arMatrix4 getActualPosition() const { return actualPosition; }
    
    void posGrab( arEffector *g );
	void rotGrab( arEffector *g );
	void scaleGrab( arEffector *g );
    void ungrab( arEffector *g );
	
	void setSelected( bool s ) { selected_ = s; }
};

class RootNode : public Node
{
private:
    arSZGAppFramework &fw_;
    
    arMatrix4 scale_;
    arMatrix4 transform_;
    
    void drawBegin( arMatrix4 &currentView, arMatrix4 &currentScale );
    void drawEnd( arMatrix4 &currentView, arMatrix4 &currentScale );
    void draw() {}
public:
    RootNode( arSZGAppFramework &fw );
    
    void setNodeScale( arMatrix4 scale ) { scale_ = scale; }
    void setNodeTransform( arMatrix4 trans ) { transform_ = trans; }
    
    arMatrix4 getNodeScale() { return scale_; }
    arMatrix4 getNodeTransform() { return transform_; }
};

class NothingNode : public Node
{
private:
    void draw() {}
public:
    NothingNode() : Node() {}
};

typedef enum { SPHERE, CUBE, CONE, TORUS, DODECAHEDRON, TEAPOT } SolidType;
static const char *SolidTypeStr[] = { "SPHERE", "CUBE", "CONE", "TORUS", "DODECAHEDRON", "TEAPOT" };
inline std::ostream &operator<< ( std::ostream &out, SolidType &t )
{
    out << SolidTypeStr[t];
    return out;
}

class SolidNode : public Node
{
private:
    SolidType type_;
protected:
    bool wireframe_;
public:
    SolidNode( SolidType type, bool wireframe ) : Node(), type_( type ), wireframe_( wireframe ) {}
    
    SolidType type() const { return type_; }
    
    void setWireFrame( bool wireframe ) { wireframe_ = wireframe; }
};

class SolidSphereNode : public SolidNode
{
private:
    GLUquadricObj *sphere_;
    double  radius_;
    int     slices_;
    int     stacks_;
protected:
    void draw();
public:
    SolidSphereNode( double radius, int slices, int stacks, bool wireframe = false );
};

class SolidCubeNode : public SolidNode
{
private:
    double size_;
protected:
    void draw();
public:
    SolidCubeNode( double size, bool wireframe = false ) : SolidNode( CUBE, wireframe ), size_( size ) {}
};

class SolidCylinderNode : public SolidNode
{
private:
    GLUquadricObj *cylinder_;
    double  base_;
    double  top_;
    double  height_;
    int     slices_;
    int     stacks_;
    
    void draw();
public:
    SolidCylinderNode( double base, double top, double height, int slices, int stacks, bool wireframe = false );
};

class SolidTeapotNode : public SolidNode
{
private:
    int size_;
    
    void draw();
public:
    SolidTeapotNode( int size, bool wireframe = false ) : SolidNode( TEAPOT, wireframe ), size_( size ) {}
};

class wbOBJRenderer : public arOBJRenderer
{
public:
	wbOBJRenderer() : arOBJRenderer() {}
	
    arTexture* getTexture( unsigned i ) { return _textures[i]; }
	void setTexture( unsigned i, arTexture *t );
};

class ObjNode : public Node
{
private:
    std::string     filename_;
    std::string     path_;
    bool            valid;
    
    void draw();
public:
    wbOBJRenderer   obj_;
	
public:
    ObjNode( std::string filename, std::string path = "" ) : Node(), filename_( filename ), path_( path )
    {
        valid = obj_.readOBJ( filename_, path_ );
        if(valid == false)
            cout << "Failed to read: " << path << "/" << filename << endl;
    }
    ObjNode( ObjNode &other_obj );
    
    void setTexture( int i, arTexture *t )
    {
        if(valid)
            obj_.setTexture( i, t );
    }
    int numTextures() {
        if(valid)
            return obj_.getNumberTextures();
        else
            return 0;
    }
    arAxisAlignedBoundingBox getAxisAlignedBoundingBox()
    {
        if(valid)
            return obj_.getAxisAlignedBoundingBox();
        else
            return arAxisAlignedBoundingBox();
    }
    arBoundingSphere getBoundingSphere()
    {
        if(valid)
            return obj_.getBoundingSphere();
        else
            return arBoundingSphere();
    }
    float getIntersection(const arRay &theRay)
    {
        if(valid)
            return obj_.getIntersection(theRay);
        else
            return -1;
    }
};

#endif
