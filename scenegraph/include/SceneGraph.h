
#ifndef _SCENEGRAPH_H_
#define _SCENEGRAPH_H_

#include <map>


#include <boost/graph/adjacency_list.hpp>


#include "Node.h"

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, Node* > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
typedef boost::graph_traits<Graph>::edge_descriptor Edge;

typedef std::map<NodeIdType, Vertex> IdToVertexMap;

// The SceneGraph MUST be instantiated during or after the start callback of the framework.
class SceneGraph
{
private:
    Graph g_;
    Vertex root_vertex_;
    IdToVertexMap idToVertex_;
    
    void addChild_( Node *newnode, Vertex parent );
    void removeChild_( Vertex v );

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