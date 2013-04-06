
#include <exception>
#include <iostream>
#include <list>
#include <string>

#include "Node.h"
#include "SceneGraph.h"

static int indent = 1;
arMatrix4 currentView, currentScale;
std::list<GraphAction> deferredActions;

void SceneGraph::addChild_( Node *newnode, Vertex *parent )
{
    if( drawing_ )
    {
        deferredActions.push_back( GraphAction( newnode, parent ) );
    }
    else
    {
        if( idToVertex_.find( newnode->id ) != idToVertex_.end() )
            throw NodeError( "Node already exists in SceneGraph", __FILE__, __LINE__ );
        Vertex *v = new Vertex( newnode );
        parent->children.insert( v );
        v->parent = parent;
        
        idToVertex_[newnode->id] = v;
    }
}

Node* SceneGraph::removeChild_( Vertex *v )
{
    if( drawing_ )
    {
        deferredActions.push_back( GraphAction( v ) );
    }
    else
    {
        Vertex *p = v->parent;
        p->children.erase( v );
        Node *n = v->me;
        delete v;
        v = 0;
        return n;
    }
}

void SceneGraph::processDeferred_()
{
    for( std::list<GraphAction>::iterator it = deferredActions.begin(); it != deferredActions.end(); it = deferredActions.begin() )
    {
        switch( it->type )
        {
        case GraphAction::ADD:
            addChild_( it->newtoadd, it->parent );
            break;
        case GraphAction::REMOVE:
            removeChild_( it->todelete );
            break;
        }
        deferredActions.erase( it );
    }
}

void SceneGraph::dfs_( Vertex *r )
{
    if( !r )
        return;
        
    Node *n = r->me;
    n->drawBegin( currentView, currentScale );
    for( std::set<Vertex*>::iterator it = r->children.begin(); it != r->children.end(); ++it )
        dfs_( *it );
    n->drawLocalBegin( currentView, currentScale );
    n->draw();
    n->drawLocalEnd( currentView, currentScale );
    n->drawEnd( currentView, currentScale );
}

SceneGraph::SceneGraph( arSZGAppFramework &fw ) : root_vertex_ ( new RootNode( fw ) )
{
    idToVertex_[root_vertex_.me->id] = &root_vertex_;
}

void SceneGraph::drawSceneGraph()
{
    drawing_ = true;
    dfs_( &root_vertex_ );
    drawing_ = false;
    processDeferred_();
}

void SceneGraph::addChild( Node *newnode )
{
    addChild_( newnode, &root_vertex_ );
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
    return idToVertex_[ id ]->me;
}
