

#include <exception>
#include <iostream>
#include <string>

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_utility.hpp>

#include "Node.h"
#include "SceneGraph.h"

static int indent = 1;
arMatrix4 currentView;

class dfs_visitor : public boost::default_dfs_visitor
{
public:
    dfs_visitor() {}
    
    template <typename Vertex, typename Graph>
    void discover_vertex(Vertex u, const Graph &g) const
    {
        /*
        for(int i = 0; i < indent; i++)
            std::cout << "---";
        std::cout << "Discover node with id=" << g[u]->id << std::endl;
        indent++;
        /**/
        
        g[u]->drawBegin( currentView );
    }
    
    template <typename Vertex, typename Graph>
    void finish_vertex( Vertex u, const Graph &g) const
    {
        g[u]->drawLocalBegin( currentView );
        g[u]->draw();
        g[u]->drawLocalEnd( currentView );
        
        g[u]->drawEnd( currentView );
        
        /*
        indent--;
        for(int i = 0; i < indent; i++)
            std::cout << "---";
        std::cout << "Finish node with id=" << g[u]->id << std::endl;
        /**/
    }
};

void SceneGraph::addChild_( Node *newnode, Vertex parent )
{
    if( idToVertex_.find( newnode->id ) != idToVertex_.end() )
        throw NodeError( "Node already exists in SceneGraph", __FILE__, __LINE__ );
    Vertex v = add_vertex( newnode, g_ );
    add_edge( parent, v, g_ );
    idToVertex_[newnode->id] = v;
}

void SceneGraph::removeChild_( Vertex v )
{
    clear_vertex( v, g_ );
    remove_vertex( v, g_ );
}

SceneGraph::SceneGraph( arSZGAppFramework &fw )
{
    RootNode *root = new RootNode( fw );
    root_vertex_ = add_vertex( root, g_ );
    idToVertex_[root->id] = root_vertex_;
}

void SceneGraph::drawSceneGraph()
{
    dfs_visitor vis;
    boost::depth_first_search( g_, visitor( vis ).root_vertex( root_vertex_ ) );
}

void SceneGraph::addChild( Node *newnode )
{
    addChild_( newnode, root_vertex_ );
}

void SceneGraph::addChild( Node *newnode, Node *parent )
{
    addChild_( newnode, idToVertex_[ parent->id ] );
}

void SceneGraph::addChild( Node *newnode, NodeIdType parentId )
{
    addChild_( newnode, idToVertex_[parentId] );
}

void SceneGraph::removeChild( Node *node )
{
    if( idToVertex_.find( node->id ) == idToVertex_.end() )
        throw( NodeError( "Node does not exist in SceneGraph", __FILE__, __LINE__ ) );
    removeChild_( idToVertex_[ node->id ] );
    idToVertex_.erase( node->id );
}

void SceneGraph::removeChild( NodeIdType id )
{
    if( idToVertex_.find( id ) == idToVertex_.end() )
        throw( NodeError( "Node does not exist in SceneGraph", __FILE__, __LINE__ ) );
    removeChild_( idToVertex_[ id ] );
    idToVertex_.erase( id );
}

Node* SceneGraph::getChild( NodeIdType id )
{
    if( idToVertex_.find( id ) == idToVertex_.end() )
        throw NodeError( "Node does not exist in SceneGraph", __FILE__, __LINE__ );
    return g_[ idToVertex_[ id ] ];
}
