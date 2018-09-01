#pragma once

#include "KeyEvent.h"
#include "MouseEvent.h"

namespace csgfx
{
	struct Event
	{
		enum { None, KeyPressed, KeyReleased, MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseWheeled } type;
		union
		{
			KeyEvent key;
			MouseEvent mouse;
		} event;
	};
}