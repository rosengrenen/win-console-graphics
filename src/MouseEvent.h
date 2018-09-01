#pragma once

#include "Mouse.h"
#include "Point.h"

namespace csgfx
{
	struct MouseEvent
	{
		Point position;
		Mouse button;
	};
}