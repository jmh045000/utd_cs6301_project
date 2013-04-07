
#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_

#include <set>
#include <map>

#include "Node.h"

class Vertex
{
private:
    Node *me;
    std::set<Vertex*> children;
    Vertex *parent;
    
    Vertex( Node *n ) : me( n ), parent( NULL ) {}
    
friend class SceneGraph;
};

typedef std::map<NodeIdType, Vertex*> IdToVertexMap;

// The SceneGraph MUST be instantiated during or after the start callback of the framework.
class SceneGraph
{
private:
    Vertex root_vertex_;
    IdToVertexMap idToVertex_;
    bool drawing_;
    
    void addChild_( Node *newnode, Vertex *parent );
    Node* removeChild_( Vertex *v );
    void processDeferred_();
    void dfs_( Vertex *r );

public:
    SceneGraph( arSZGAppFramework &fw );
    ~SceneGraph();

    void drawSceneGraph();
    
    void addChild( Node *newNode );
    void addChild( Node *newNode, Node *parent );
    void addChild( Node *newNode, NodeIdType parentId );
    
    void removeChild( Node *node );
    void removeChild( NodeIdType id );
    
    Node* getChild( NodeIdType id );
    Node* getRoot() { return root_vertex_.me; }
};

class GraphAction
{
    typedef enum { ADD, REMOVE, INVALID } GraphActionType;
    
    GraphActionType type;
    Node *newtoadd;
    Vertex *parent;
    Vertex *todelete;
    
    GraphAction( Node *a, Vertex *p ) : type( ADD ), newtoadd( a ), parent( p ) {}
    GraphAction( Vertex *d ) : type( REMOVE ), todelete( d ) {}
    
    friend class SceneGraph;
};

class easyException : public std::exception
{
public:
    std::string s;
    virtual const char* what() const throw()
    {
        //if exception makes it to the user, s is printed
        return s.c_str();
    }
    easyException(): s("") {}

    //construct an exception from a string
    easyException(std::string s) : s(s) {}

    //construct an exception from a string, but include __FILE__ and __LINE__
    easyException(std::string msg, const char * f, uint32_t l)
    {
        std::stringstream ss;
        ss << msg << " (" << f << ":" << l << ")";
        s=ss.str();
    }
    ~easyException() throw() {}
};

class NodeError : public easyException
{
public:
    NodeError( std::string s ) : easyException( s ) {}
    NodeError( std::string s, const char *f, uint32_t l ) : easyException( s, f, l ) {}
};

#endif /*_SCENEGRAPH_H_*/
