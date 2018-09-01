#include "Buffer.h"

csgfx::Buffer::Buffer(int width, int height)
{
	this->handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, 0, CONSOLE_TEXTMODE_BUFFER, 0);
	if (!this->handle)
	{
		unsigned long error = GetLastError();
		throw 0;
	}
	if (!SetConsoleScreenBufferSize(this->handle, { 1000, 1000 }))
	{
		unsigned long error = GetLastError();
		throw 0;
	}
	//TODO: Error handling, how does this throw?
	COORD maxSize = GetLargestConsoleWindowSize(this->handle);
	this->maxSize = { maxSize.X, maxSize.Y };
	this->setSize(width, height);
}

std::vector<CHAR_INFO> csgfx::Buffer::readRect(const Point& at, const Dimension& size) const
{
	std::vector<CHAR_INFO> buffer(size.width * size.height);
	SMALL_RECT region { at.x, at.y, at.x + size.width - 1, at.y + size.height - 1 };
	if (!ReadConsoleOutput(this->handle, buffer.data(), { size.width, size.height }, {}, &region))
	{
		unsigned long error = GetLastError();
		throw 0;
	}
	if (region.Left != at.x && region.Top != at.y && region.Right != at.x + size.width - 1 && region.Bottom != at.y + size.height - 1)
	{
		throw 0;
	}
	return buffer;
}

std::vector<char> csgfx::Buffer::readRectChar(const Point& at, const Dimension& size) const
{
	std::vector<CHAR_INFO> buffer(size.width * size.height);
	SMALL_RECT region { at.x, at.y, at.x + size.width - 1, at.y + size.height - 1 };
	if (!ReadConsoleOutput(this->handle, buffer.data(), { size.width, size.height }, { }, &region))
	{
		unsigned long error = GetLastError();
		throw 0;
	}
	if (region.Left != at.x && region.Top != at.y && region.Right != at.x + size.width - 1 && region.Bottom != at.y + size.height - 1)
	{
		throw 0;
	}
	std::vector<char> characters(size.width * size.height);
	for (int i = 0; i < characters.size(); ++i)
	{
		characters.at(i) = buffer.at(i).Char.AsciiChar;
	}
	return characters;
}

std::vector<std::pair<csgfx::Colour, csgfx::Colour>> csgfx::Buffer::readRectAttrib(const Point& at, const Dimension& size) const
{
	std::vector<CHAR_INFO> buffer(size.width * size.height);
	SMALL_RECT region { at.x, at.y, at.x + size.width - 1, at.y + size.height - 1 };
	if (!ReadConsoleOutput(this->handle, buffer.data(), { size.width, size.height }, { }, &region))
	{
		unsigned long error = GetLastError();
		throw 0;
	}
	if (region.Left != at.x && region.Top != at.y && region.Right != at.x + size.width - 1 && region.Bottom != at.y + size.height - 1)
	{
		throw 0;
	}
	std::vector<std::pair<Colour, Colour>> attributes(size.width * size.height);
	for (int i = 0; i < attributes.size(); ++i)
	{
		attributes.at(i).first = static_cast<Colour>(buffer.at(i).Attributes % 16);
		attributes.at(i).second = static_cast<Colour>(buffer.at(i).Attributes / 16);
	}
	return attributes;
}

void csgfx::Buffer::clear(const Colour& _colour)
{
	DWORD written;
	if (!FillConsoleOutputAttribute(this->handle, static_cast<int>(_colour) + 16 * static_cast<int>(_colour), this->size.width * this->size.height, { 0, 0 }, &written))
	{
		unsigned long error = GetLastError();
		throw 0;
	}
}

void csgfx::Buffer::draw(std::vector<CHAR_INFO>& buffer, const Point& at, const Dimension& size)
{
	if (buffer.size() < size.width * size.height)
	{
		//TODO: Error handling
		unsigned long error = GetLastError();
		//throw 0;
	}
	SMALL_RECT region { at.x, at.y, at.x + size.width, at.y + size.height };
	if (!WriteConsoleOutput(this->handle, buffer.data(), { size.width, size.height }, { 0, 0 }, &region))
	{
		//TODO: Error handling
		unsigned long error = GetLastError();
		throw 0;;
	}
}

void csgfx::Buffer::setSize(const Dimension& size)
{
	this->setSize(size.width, size.height);
}

void csgfx::Buffer::setSize(short width, short height)
{
	if (width < 1 || height < 1)
	{
		//TODO: Error handling
		unsigned long error = GetLastError();
		throw 0;
	}
	Dimension currentBufferSize = this->getBufferSize();
	Dimension currentWindowSize = this->getWindowSize();
	if (currentBufferSize.width == currentWindowSize.width &&
		currentBufferSize.height == currentWindowSize.height &&
		currentBufferSize.width == width &&
		currentBufferSize.height == height)
		return;
	Dimension windowMaxSize = this->getMaxSize();
	Dimension targetSize {
	  width > windowMaxSize.width ? windowMaxSize.width : width,
	  height > windowMaxSize.height ? windowMaxSize.height : height
	};
	this->activate();
	if (targetSize.width > currentWindowSize.width && targetSize.height > currentWindowSize.height)
	{
		this->setBufferSize(targetSize);
		this->setWindowSize(targetSize);
	}
	else if (targetSize.width < currentWindowSize.width && targetSize.height > currentWindowSize.height)
	{
		this->setWindowSize(targetSize.width, currentWindowSize.height);
		this->setBufferSize(targetSize);
		this->setWindowSize(targetSize);
	}
	else if (targetSize.width > currentWindowSize.width && targetSize.height < currentWindowSize.height)
	{
		this->setWindowSize(currentWindowSize.width, targetSize.height);
		this->setBufferSize(targetSize);
		this->setWindowSize(targetSize);
	}
	else
	{
		this->setWindowSize(targetSize);
		this->setBufferSize(targetSize);
	}
	this->size = targetSize;
}

csgfx::Dimension csgfx::Buffer::getSize() const
{
	return this->size;
}

void csgfx::Buffer::activate()
{
	if (!SetConsoleActiveScreenBuffer(this->handle))
	{
		//TODO: Error handling
		unsigned long error = GetLastError();
		throw 0;
	}
}

csgfx::Dimension csgfx::Buffer::getCharSize() const
{
	CONSOLE_FONT_INFO font {};
	GetCurrentConsoleFont(this->handle, false, &font);
	return { font.dwFontSize.X, font.dwFontSize.Y };
}

const csgfx::Dimension& csgfx::Buffer::getMaxSize() const
{
	return this->maxSize;
}

void csgfx::Buffer::setCursorVisibility(bool visible)
{
	CONSOLE_CURSOR_INFO cursorInfo;
	if (!GetConsoleCursorInfo(this->handle, &cursorInfo))
	{
		//TODO: Error handling
		unsigned long error = GetLastError();
		throw 0;
	}
	cursorInfo.bVisible = visible;
	if (!SetConsoleCursorInfo(this->handle, &cursorInfo))
	{
		//TODO: Error handling
		unsigned long error = GetLastError();
		throw 0;
	}
}

csgfx::Dimension csgfx::Buffer::getBufferSize() const
{
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo {};
	GetConsoleScreenBufferInfo(this->handle, &bufferInfo);
	return { bufferInfo.dwSize.X, bufferInfo.dwSize.Y };
}

void csgfx::Buffer::setBufferSize(const Dimension& size)
{
	this->setBufferSize(size.width, size.height);
}

void csgfx::Buffer::setBufferSize(short width, short height)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(this->handle, &info);
	// Error handling
	if (!SetConsoleScreenBufferSize(this->handle, { width, height }))
	{
		//TODO: Error handling
		DWORD error = GetLastError();
		OutputDebugString("SetConsoleScreenBufferSize\n");
	}
}

csgfx::Dimension csgfx::Buffer::getWindowSize() const
{
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo {};
	GetConsoleScreenBufferInfo(this->handle, &bufferInfo);
	return { bufferInfo.srWindow.Right + 1, bufferInfo.srWindow.Bottom + 1 };
}

void csgfx::Buffer::setWindowSize(const Dimension& size)
{
	this->setWindowSize(size.width, size.height);
}

void csgfx::Buffer::setWindowSize(short width, short height)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(this->handle, &info);
	SMALL_RECT size { 0, 0, width - 1, height - 1 };
	if (!SetConsoleWindowInfo(this->handle, true, &size))
	{
		//TODO: Error handling
		DWORD error = GetLastError();
		OutputDebugString("SetConsoleWindowInfo\n");
	}
}
