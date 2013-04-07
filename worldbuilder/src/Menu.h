
#ifndef _MENU_H_
#define _MENU_H_

#include "arSZGAppFramework.h"

#include "WiiMote.h"
#include "SceneGraph.h"
#include "Node.h"

class MenuItem : public Node
{
protected:
    bool selected;
    
    MenuItem* operator++(int);
public:
    MenuItem( bool s ) : Node(),selected( s ) {}
    
    void deselect() { selected = false; }
    void select() { selected = true; }
    
    friend class MenuNode;
};

class Tab : public MenuItem
{
private:
    bool active;
    void draw();
    const char *name;
public:
    Tab( const char *name, bool selected = false ) : MenuItem( selected ), name( name ), active( selected ) {}
    
    void deactivate() { active = false; }
    void activate() { active = true; }
    
    friend struct _TabGroup_s;
};

class Item : public MenuItem
{
public:
    typedef enum { OBJECT, TEXTURE, TOOL, MAX_ITEM_TYPES } ItemType;
	typedef enum { DELETE_TOOL, GROUP_TOOL, UNGROUP_TOOL, COPY_TOOL, PASTE_TOOL, MAX_TOOL_TYPES } ToolType;
private:
    std::string filename;
    std::string path;
    arOBJRenderer obj;
    arTexture texture;
    const char *name;
    void draw();
public:
    Item( ItemType t, std::string f, std::string p, bool selected = false );
	Item( ItemType t, ToolType t2, bool selected = false );
    
    const ItemType type;
	ToolType tooltype;
    std::string getFilename() { return filename; }
    std::string getPath() { return path; }
    
    void doAction( arSZGAppFramework *fw );
    
    friend struct _ItemGroup_s;
};

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
            _ = 0;
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
	ItemGroup *tools;
    
    arSZGAppFramework *framework;
    
    void draw();
public:
    MenuNode( arSZGAppFramework &fw ) : Node(), selectedGroup( TAB ), currentSelected( NULL ), framework( &fw )  {}
    
    void setObjects( list<Item*> l ) { objects = new ItemGroup( l ); }
    void setTextures( list<Item*> l ) { textures = new ItemGroup( l ); }
	void setTools( list<Item*> l ) { tools = new ItemGroup( l ); }
    
    void pressedDown();
    void pressedLeft();
    void pressedRight();
    void pressedUp();
    MenuAction pressedA();
    
    friend SelectedGroup operator++(SelectedGroup);
    friend SelectedGroup operator--(SelectedGroup);
    friend MenuNode* initMenu( arSZGAppFramework& );
    friend void buildMenu( MenuNode* );
    friend void tearDownMenu( MenuNode* );
};

// These are all defined in worldbuilder.cpp, but used in Menu.cpp
extern SceneGraph *sg;
extern WiiMote primary;
extern WiiMote secondary;
extern std::list<arInteractable*> interactableObjects;

// These are all in Menu.cpp, but used in worldbuilder.cpp
extern MenuNode* initMenu( arSZGAppFramework &fw );
extern void buildMenu( MenuNode *menu );
extern void tearDownMenu( MenuNode *menu );
extern void drawMenu( MenuNode *menu, arSZGAppFramework &fw );


#endif /*_MENU_H_*/
