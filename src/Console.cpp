#include "Console.h"

csgfx::Console *csgfx::Console::sInstance = nullptr;

csgfx::Console::Console(int width, int height)
	: inputHandle(GetStdHandle(STD_INPUT_HANDLE)),
	frontBuffer(width, height),
	backBuffer(width, height)
{
	if (sInstance)
		throw "Class already intanciated";
	sInstance = this;
	this->inputThread = std::thread(input);
	//TODO: Abstract the modes?
	SetConsoleMode(this->inputHandle, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
	this->frontBuffer.activate();
	this->size = this->backBuffer.getSize();
}

std::string csgfx::Console::getTitle()
{
	char buffer[128];
	unsigned long length = GetConsoleTitle(buffer, 128);
	return std::string(buffer, length);
}

void csgfx::Console::setTitle(const std::string& title)
{
	SetConsoleTitle(title.c_str());
}

LRESULT CALLBACK csgfx::Console::keyboardInput(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		KBDLLHOOKSTRUCT *keyboard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
		Event e {};
		e.type = keyboard->flags & LLKHF_UP ? Event::KeyReleased : Event::KeyPressed;
		e.event.key.code = static_cast<Keyboard>(keyboard->vkCode);
		sInstance->pushEvent(e);
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

LRESULT CALLBACK csgfx::Console::mouseInput(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		MSLLHOOKSTRUCT *mouse = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
		ScreenToClient(GetConsoleWindow(), &mouse->pt);
		RECT consoleRect { };
		GetClientRect(GetConsoleWindow(), &consoleRect);

		if (!(mouse->pt.x < 0 || 
			mouse->pt.y < 0 || 
			mouse->pt.x > consoleRect.right || 
			mouse->pt.y > consoleRect.bottom))
		{
			Dimension charSize = sInstance->backBuffer.getCharSize();
			Event e {};
			e.event.mouse.position = { static_cast<short>(mouse->pt.x / charSize.width), static_cast<short>(mouse->pt.y / charSize.height) };
			switch (wParam)
			{
				case WM_LBUTTONDOWN:
					{
						e.type = Event::MouseButtonPressed;
						e.event.mouse.button = Mouse::Left;
					} break;
				case WM_LBUTTONUP:
					{
						e.type = Event::MouseButtonReleased;
						e.event.mouse.button = Mouse::Left;
					} break;
				case WM_MOUSEMOVE:
					{
						e.type = Event::MouseMoved;
					} break;
				case WM_MOUSEWHEEL:
				case WM_MOUSEHWHEEL:
					{
						e.type = Event::MouseWheeled;
					} break;
				case WM_RBUTTONDOWN:
					{
						e.type = Event::MouseButtonPressed;
						e.event.mouse.button = Mouse::Right;
					} break;
				case WM_RBUTTONUP:
					{
						e.type = Event::MouseButtonReleased;
						e.event.mouse.button = Mouse::Right;
					} break;
			}
			sInstance->pushEvent(e);
		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);
}

void csgfx::Console::input()
{
	SetWindowsHookEx(WH_MOUSE_LL, mouseInput, 0, 0);
	SetWindowsHookEx(WH_KEYBOARD_LL, keyboardInput, 0, 0);

	MSG message {};
	while (GetMessage(&message, 0, 0, 0))
	{
	}
}

void csgfx::Console::pushEvent(const Event& event)
{
	std::lock_guard<std::mutex> sync(this->eventMutex);
	this->events.push(event);
}

// install mutex? and how? start and check with bool and end when empty, or local guard
// fps limiit, look at the benny box's engine
bool csgfx::Console::pollEvent(Event& event)
{
	std::lock_guard<std::mutex> sync(this->eventMutex);
	if (this->events.empty())
	{
		return false;
	}

	event = this->events.front();
	this->events.pop();
	return true;
}

std::optional<csgfx::Point> csgfx::Console::getMousePosition() const
{
	POINT cursorPosition { };
	GetCursorPos(&cursorPosition);
	ScreenToClient(GetConsoleWindow(), &cursorPosition);
	RECT consoleRect {};
	GetClientRect(GetConsoleWindow(), &consoleRect);
	if (cursorPosition.x < 0 || cursorPosition.y < 0 || cursorPosition.x > consoleRect.right || cursorPosition.y > consoleRect.bottom)
	{
		return {};
	}
	Dimension charSize = this->backBuffer.getCharSize();
	return std::optional<Point>({ static_cast<short>(cursorPosition.x / charSize.width), static_cast<short>(cursorPosition.y / charSize.height) });
}

void csgfx::Console::setSize(int width, int height)
{
	this->backBuffer.setSize(width, height);
	this->frontBuffer.setSize(width, height);
	this->size = this->backBuffer.getSize();
}

const csgfx::Dimension& csgfx::Console::getSize() const
{
	return this->backBuffer.getSize();
}

void csgfx::Console::clear(const Colour& colour)
{
	this->backBuffer.clear(colour);
}

void csgfx::Console::point(const Point& at, const Colour& colour)
{
	this->rect(" ", at, { 1, 1 }, colour, colour);
}

void csgfx::Console::rect(const std::string& text, const Point& at, const Dimension& size, const Colour& foreground, const Colour& background)
{
	if (text.size() < size.width * size.height)
	{
		//TODO: Error handling
		unsigned long error = GetLastError();
		throw 0;
	}
	std::vector<CHAR_INFO> buffer(text.size());
	for (int i = 0; i < text.size(); ++i)
	{
		buffer.at(i).Attributes = static_cast<int>(foreground) + 16 * static_cast<int>(background);
		buffer.at(i).Char.AsciiChar = text.at(i);
	}
	this->backBuffer.draw(buffer, at, size);
}

void csgfx::Console::fill(const Point& at, const Dimension& size, const Colour& colour)
{
	std::vector<CHAR_INFO> buffer(size.width * size.height);
	for (int i = 0; i < size.width * size.height; ++i)
	{
		buffer.at(i).Attributes = 16 * static_cast<int>(colour);
		buffer.at(i).Char.AsciiChar = ' ';
	}
	this->backBuffer.draw(buffer, at, size);
}

void csgfx::Console::line(const Point& begin, const Point& end, const Colour& colour)
{

	Dimension size;
	Point at { std::min(begin.x, end.x), std::min(begin.y, end.y) };
	size.width = begin.x > end.x ? begin.x - end.x + 1 : end.x - begin.x + 1;
	size.height = begin.y > end.y ? begin.y - end.y + 1 : end.y - begin.y + 1;
	std::vector<CHAR_INFO> buffer = this->backBuffer.readRect(at, size);
	if (size.width == 1 || size.height == 1)
	{
		for (int i = 0; i < size.width * size.height; ++i)
		{
			buffer.at(i).Attributes = 16 * static_cast<int>(colour);
			buffer.at(i).Char.AsciiChar = ' ';
		}
	}
	else
	{
		float m = static_cast<float>(size.height - 1) / static_cast<float>(size.width - 1);
		if (begin.x > end.x ^ begin.y > end.y)
		{
			m *= -1;
		}
		if (m < 0)
		{
			if (m > -1)
			{
				for (int x = 0; x < size.width; ++x)
				{
					int rounded = std::round(x * m);
					int y = (size.height - 1) + rounded;
					int index = y * size.width + x;
					buffer.at(index).Attributes = 16 * static_cast<int>(colour);
					buffer.at(index).Char.AsciiChar = ' ';
				}
			}
			else
			{
				for (int y = 0; y < size.height; ++y)
				{
					// y = mx --> y /m
					int rounded = std::round(y / m);
					int x = (size.width - 1) + rounded;
					int index = y * size.width + x;
					buffer.at(index).Attributes = 16 * static_cast<int>(colour);
					buffer.at(index).Char.AsciiChar = ' ';
				}
			}
		}
		else
		{
			if (m < 1)
			{
				for (int x = 0; x < size.width; ++x)
				{
					int y = std::round(x * m);
					int index = y * size.width + x;
					buffer.at(index).Attributes = 16 * static_cast<int>(colour);
					buffer.at(index).Char.AsciiChar = ' ';
				}
			}
			else
			{
				for (int y = 0; y < size.height; ++y)
				{
					int x = std::round(y / m);
					int index = y * size.width + x;
					buffer.at(index).Attributes = 16 * static_cast<int>(colour);
					buffer.at(index).Char.AsciiChar = ' ';
				}
			}
		}
	}
	this->backBuffer.draw(buffer,
		at,
		size);
}

void csgfx::Console::display()
{
	std::swap(this->frontBuffer, this->backBuffer);
	this->backBuffer.setSize(size);
	this->frontBuffer.setCursorVisibility(false);
	this->frontBuffer.activate();
}