#ifndef __WIIMOTE_H
#define __WIIMOTE_H

//********************************************************
// Syzygy is licensed under the BSD license v2
// see the file SZG_CREDITS for details
//********************************************************

static const int NUM_BUTTONS = 11;
static const int NUM_AXES = 9;
static const int NUM_MATRICES = 2;

#include <list>

#include "arEffector.h"
#include "arGrabCondition.h"

class WiiMote : public arEffector
{
public:
	// Types:
	enum controller_t { CONTROLLER_1 = 0, CONTROLLER_2, MAXCONTROLLERS };
	typedef enum button_t
	{
		  TWO = 0,
		  ONE,
		  B,
		  A,
		  MINUS,
		  HOME,
		  LEFT,
		  RIGHT,
		  DOWN,
		  UP,
		  PLUS,
		  MAX_BUTTONS
	};
	typedef void (*draw_cb)(WiiMote&);
	typedef std::list<button_t> ButtonList;
private:
	
	
	controller_t	controller_;
	int				matrixOverride_;
	draw_cb			draw_;

protected:
	ButtonList		downButtons;
	ButtonList		onButtons;
	ButtonList		upButtons;

public:

	//! \brief Constructor for wiimote
	//! \param[in] controller Controller number (currently only CONTROLLER_1 or CONTROLLER_2)
	WiiMote(controller_t controller);

	//! \brief Contructor for wiimote that takes a matrix override
	//! \param[in] controller Controller number (currently on CONTROLLER_1 or CONTROLLER_2)
	//! \param[in] matrixOverride An override matrix index that this wiimote will use for navigation
	//! \remark This constructor allows us to use a different source for the navigation of this wiimote,
	//!         that way, we'll be able to use another device as this effector's navigation (i.e. Vicons)
	WiiMote(controller_t controller, int matrixOverride);

	//! \brief Sets a draw callback
	//! \param[in] d Takes a draw_cb function pointer, which is called when using draw()
	void setDrawCallback(draw_cb d) { draw_ = d; };

	//! \brief Draws the wiimote's effector object
	//! \remark This function will only work once setDrawCallback is called with a draw_cb function pointer
	void draw() { if(draw_) draw_(*this); }
	
	void rumble(int on_off);

	//! \brief Get the buttons that were just pressed
	const ButtonList& getDownButtons();

	//! \brief Get the buttons that are currently held down
	const ButtonList& getOnButtons();

	//! \brief Get the buttons that were just released
	const ButtonList& getUpButtons();
	
	arGrabCondition getGrabCondition(button_t b);
	
	friend std::ostream& operator<< (std::ostream &out, ButtonList l);
	friend std::ostream& operator<< (std::ostream &out, button_t b);
};

#endif /* __WIIMOTE_H */