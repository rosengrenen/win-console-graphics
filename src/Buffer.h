#pragma once

#include <vector>
#include <Windows.h>

#include "Colour.h"
#include "Dimension.h"
#include "Point.h"

namespace csgfx
{
	class Buffer
	{
	private:
		HANDLE handle;
		Dimension size;
		Dimension maxSize;
	public:
		Buffer(int width, int height);

		std::vector<CHAR_INFO> readRect(const Point& at, const Dimension& size) const;
		std::vector<char> readRectChar(const Point& at, const Dimension& size) const;
		std::vector<std::pair<Colour, Colour>> readRectAttrib(const Point& at, const Dimension& size) const;

		void clear(const Colour& colour);
		void draw(std::vector<CHAR_INFO>& buffer, const Point& at, const Dimension& size);
		void activate();

		Dimension getCharSize() const;

		const Dimension& getMaxSize() const;
		Dimension getSize() const;
		void setSize(const Dimension& size);
		void setSize(short width, short height);

		void setCursorVisibility(bool visible);
	private:
		Dimension getBufferSize() const;
		void setBufferSize(const Dimension& size);
		void setBufferSize(short width, short height);

		Dimension getWindowSize() const;
		void setWindowSize(const Dimension& size);
		void setWindowSize(short width, short height);

	};
}