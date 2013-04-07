//********************************************************
// Syzygy is licensed under the BSD license v2
// see the file SZG_CREDITS for details
//********************************************************

// precompiled header include MUST appear as the first non-comment line
#include "arPrecompiled.h"
// MUST come before other szg includes. See arCallingConventions.h for details.
#define SZG_DO_NOT_EXPORT

#include "WiiMote.h"
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
	const char* vers = wiiuse_startup(WIIUSE_PATH);
	wiimotes_ = wiiuse_init(2, wiimote_ids_, NULL, NULL, NULL);
	
	const int found = wiiuse_find(wiimotes_, 2, 5);
	
	connected_ = wiiuse_connect( wiimotes_, 2 );
}

WiiMote::WiiMote(controller_t controller) : 
	arEffector( controller+1, NUM_BUTTONS, (controller*NUM_BUTTONS), 0, 0),
	controller_(controller), matrixOverride_(-1), draw_(standardDraw)
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
	controller_(controller), matrixOverride_(matrixOverride), draw_(standardDraw)
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
    float lastTipDistance = tipDistance;
    tipDistance = scalar_distance(primary.getMatrix(), secondary.getMatrix());

/*
    if (tipDistance != lastTipDistance)
        cout <<  "New tip distance: " << tipDistance << endl;
*/
}
