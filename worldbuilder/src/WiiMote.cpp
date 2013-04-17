//********************************************************
// Syzygy is licensed under the BSD license v2
// see the file SZG_CREDITS for details
//********************************************************

// precompiled header include MUST appear as the first non-comment line
#include "arPrecompiled.h"
// MUST come before other szg includes. See arCallingConventions.h for details.
#define SZG_DO_NOT_EXPORT

#include "arInteractable.h"

#include "WiiMote.h"
#include "Node.h"
#include "MyConditions.h"

#include "arGlut.h"

using namespace std;

#ifdef AR_USE_WIN_32
  #define WIIUSE_PATH "wiiuse.dll"
#else
  #include <unistd.h>
  #define WIIUSE_PATH "./wiiuse.so"
#endif

#include "wiiuse.h" 

static bool 	dllLoaded_;
static wiimote_t **wiimotes_;
static int wiimote_ids_[] = { 1, 2};
static int connected_;



void standardDraw(WiiMote& wm)
{
	glPushMatrix();
		glMultMatrixf( wm.getCenterMatrix().v );
		// draw grey rectangular solid 2"x2"x5'
		glScalef( 2./12., 2./12., 5. );
		glColor3f( .5,.5,.5 );
		glutSolidCube(1.);
		// superimpose slightly larger black wireframe (makes it easier to see shape)
		glColor3f(0,0,0);
		glutWireCube(1.03);
	glPopMatrix();
}

void init_driver()
{
	wiiuse_startup(WIIUSE_PATH);
	wiimotes_ = wiiuse_init(2, wiimote_ids_, NULL, NULL, NULL);
	
	const int found = wiiuse_find(wiimotes_, 2, 5);
	
	connected_ = wiiuse_connect( wiimotes_, found );
}

bool WiiMote::requestPosGrab( arInteractable *grabee )
{
    bool ret = arEffector::requestGrab( grabee );
    if( ret )
        if( Node *n = dynamic_cast<Node*>( grabee ) )
            n->posGrab( this );
    return ret;
}

bool WiiMote::requestRotGrab( arInteractable *grabee )
{
	bool ret = arEffector::requestGrab( grabee );
    if( ret )
        if( Node *n = dynamic_cast<Node*>( grabee ) )
            n->rotGrab( this );
    return ret;
}

bool WiiMote::requestScaleGrab( arInteractable *grabee )
{
	bool ret = arEffector::requestGrab( grabee );
    if( ret )
        if( Node *n = dynamic_cast<Node*>( grabee ) )
            n->scaleGrab( this );
    return ret;
}

void WiiMote::requestUngrab( arInteractable *grabee )
{
    arEffector::requestUngrab( grabee );
    if( Node *n = dynamic_cast<Node*>( grabee ) )
        n->ungrab( this );
}

WiiMote::WiiMote(controller_t controller) : 
	arEffector( controller+1, NUM_BUTTONS, (controller*NUM_BUTTONS), 0, 0),
	controller_(controller), matrixOverride_(-1), draw_(standardDraw),
    selecting(false), laserOn(false)
{
	if(!dllLoaded_)
	{
		init_driver();
		dllLoaded_ = true;
	}
	setInteractionSelector( arDistanceInteractionSelector(2.) );
}


WiiMote::WiiMote(controller_t controller, int matrixOverride) :
	arEffector( matrixOverride, NUM_BUTTONS, (controller*NUM_BUTTONS), 0, 0),
	controller_(controller), matrixOverride_(matrixOverride), draw_(standardDraw),
    selecting(false), laserOn(false)
{
	if(!dllLoaded_)
	{
		init_driver();
		dllLoaded_ = true;
	}
	setInteractionSelector( arDistanceInteractionSelector(2.) );
}

void WiiMote::rumble(int on_off)
{
	cout << "Rumble called with controller=" << controller_ << ", on_off=" << on_off << endl;
	if(controller_ >= connected_)
	{
		return;
	}
	on_off = !(!on_off);
	
	cout << "Setting rumble on controller " << controller_ << " to " << on_off << endl;
	wiiuse_rumble(wiimotes_[controller_], on_off);
}

const WiiMote::ButtonList& WiiMote::getDownButtons()
{
	downButtons.clear();
	for(int i = 0; i < NUM_BUTTONS; i++)
	{
		if( getOnButton( i ) )
		{
			downButtons.push_back( (button_t)i );
		}
	}
	return downButtons;
}

const WiiMote::ButtonList& WiiMote::getOnButtons()
{
	onButtons.clear();
	for(int i = 0; i < NUM_BUTTONS; i++)
	{
		if( getButton( i ) )
		{
			onButtons.push_back( (button_t)i );
		}
	}
	return onButtons;
}

const WiiMote::ButtonList& WiiMote::getUpButtons()
{
	upButtons.clear();
	for(int i = 0; i < NUM_BUTTONS; i++)
	{
		if( getOffButton( i ) )
		{
			upButtons.push_back( (button_t)i );
		}
	}
	return upButtons;
}

arGrabCondition WiiMote::getGrabCondition(button_t button)
{
	return arGrabCondition(AR_EVENT_BUTTON, button, 0.5);
} 

ostream &operator<< (ostream &out, WiiMote::ButtonList l)
{
	for(WiiMote::ButtonList::iterator it = l.begin(); it != l.end(); ++it)
	{
		out << *it;
		if( (++it) != l.end() ) out << ", ";
		--it;
	}
	return out;
}

ostream &operator<< (ostream &out, WiiMote::button_t b)
{
	switch(b)
	{
	case WiiMote::TWO: out << "2"; break;
	case WiiMote::ONE: out << "1"; break;
	case WiiMote::B: out << "B"; break;
	case WiiMote::A: out << "A"; break;
	case WiiMote::MINUS: out << "-"; break;
	case WiiMote::HOME: out << "HOME"; break;
	case WiiMote::LEFT: out << "Left"; break;
	case WiiMote::RIGHT: out << "Right"; break;
	case WiiMote::DOWN: out << "Down"; break;
	case WiiMote::UP: out << "Up"; break;
	case WiiMote::PLUS: out << "+"; break;
    default: break;
	}
	return out;
}


float WiiMote::tipDistance = 0.0;
float WiiMote::lastTipDistance = 0.0;

float scalar_distance(arMatrix4 m1, arMatrix4 m2)
{
    //we may not want the sqrt...
    float distance = sqrt((m1[12] - m2[12])*(m1[12] - m2[12]) +
                          (m1[13] - m2[13])*(m1[13] - m2[13]) +
                          (m1[14] - m2[14])*(m1[14] - m2[14]));
    return distance;
}

void WiiMote::updateTipDistance(WiiMote &primary, WiiMote &secondary)
{
    lastTipDistance = tipDistance;
    tipDistance = scalar_distance(primary.getMatrix(), secondary.getMatrix());

/*
    if (tipDistance != lastTipDistance)
        cout <<  "New tip distance: " << tipDistance << endl;
*/
}

arVector3 WiiMote::extractDirection()
{
    arMatrix4 m1 = _matrix * ar_translationMatrix( 0, 0, -1);
    arVector3 retval = ar_ET(m1) - ar_ET(_matrix);
    return retval;
}
arRay WiiMote::ray()
{
    arRay r;
    r.origin = ar_ET(_matrix);
    r.direction = extractDirection();
    return r;
}

inline float vdistance2(arVector3 v1, arVector3 v2)
{
    float d[3];
    d[0] = v1[0]-v2[0];
    d[1] = v1[1]-v2[1];
    d[2] = v1[2]-v2[2];
    return d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
}

inline bool behind(arVector3 v1, arVector3 v2, arVector3 dir)
{
    arVector3 dir2 = (v1 - v2).normalize();
    arVector3 dir3 = dir + dir2;
    if (dir3.magnitude() > 1)
        return true;
    return false;
}

ObjNode * WiiMote::closestObject(interlist &objects)
{
    float min = INFINITY;
    ObjNode *retval = NULL;
    
    if( getButton( WiiMote::A ) || getButton( WiiMote::B ) )
        if( arInteractable* inter = const_cast<arInteractable*>( getGrabbedObject() ) )
            if( ObjNode *p = dynamic_cast<ObjNode*>( inter ) ) {
                linePoint = ar_ET( p->getActualPosition() );
                selecting = true;
                return p;
            }
            
    arVector3 direction = extractDirection().normalize();
    arVector3 origin = ar_ET(_matrix);
    //cout << "direction: " << direction << endl;
    //cout << "origin: " << origin << endl;

    selecting = false;
    
    for(interlist::iterator it = objects.begin(); it != objects.end(); ++it)
    {
        ObjNode * ndptr = dynamic_cast<ObjNode *>(*it);
        if(ndptr == NULL) {
            //cout << "Not ObjNode" << endl;
            continue;
        }
        requestUngrab( ndptr );
        ndptr->untouch( *this );
        arBoundingSphere sphere = ndptr->getBoundingSphere();
        sphere.position = ar_ET( ndptr->getActualPosition() * ar_TM( sphere.position ) );
        //cout << "center: " << sphere.position << " radius: " << sphere.radius << endl;
        arVector3 lp = ar_projectPointToLine(origin, direction, sphere.position);
        //cout << "line point: " << lp << endl;

        // outside sphere...
        if( vdistance2(lp, sphere.position) > sphere.radius*sphere.radius ) {
            //cout << "outside sphere" << endl;
            continue;
        }
        float distance = vdistance2(lp, origin);

        if(behind(origin, lp, direction)) {
            //cout << "behind wand" << endl;
            continue;
        }

        if(distance < min) {
            min = distance;
            retval = ndptr;
            linePoint = lp;
            selecting = true;
        }
        else {
            //cout << "not closest object" << endl;
        }
    }
    
    
    return retval;
}

void WiiMote::drawdot()
{
    if( laserOn )
    {
        if(selecting) 
        {
            glPushMatrix();
                glMultMatrixf( (ar_TM(linePoint)).v );
                glColor3f(1.0, 0.0, 0.0);
                glutWireSphere(0.1, 32, 32);
            glPopMatrix();
            glPushMatrix();
                glLineWidth(2.5); 
                glColor3f(1.0, 0.0, 0.0);
                glBegin(GL_LINES);
                arVector3 o = ar_ET(_matrix);
                glVertex3f( o[0], o[1], o[2] );
                glVertex3f( linePoint[0], linePoint[1], linePoint[2] );
                glEnd();
            glPopMatrix();
        }
        else
        {
            arMatrix4 m1 = _matrix * ar_translationMatrix( 0, 0, -10000);
            arVector3 o1 = ar_ET(m1);
            arVector3 o2 = ar_ET(_matrix);
            glPushMatrix();
                glLineWidth(2.5); 
                glColor3f(1.0, 0.0, 0.0);
                glBegin(GL_LINES);
                glVertex3f( o1[0], o1[1], o1[2] );
                glVertex3f( o2[0], o2[1], o2[2] );
                glEnd();
            glPopMatrix();

        }
    }
}
