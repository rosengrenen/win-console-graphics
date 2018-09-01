#pragma once

/*
 * TODO:
 * - Add drawing functionality
*/
#define NOMINMAX
#include <Windows.h>

#include <array>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>

#include "Buffer.h"
#include "Colour.h"
#include "Event.h"

namespace csgfx
{
  class Console
  {
  private:
    static Console *sInstance;
    HANDLE inputHandle;
    std::queue<Event> events;
    std::array<bool, 255> keyboard;
    std::array<bool, 3> mouse;
    Buffer frontBuffer;
    Buffer backBuffer;
    Dimension size;
    std::thread inputThread;
    std::mutex eventMutex;
  public:
    Console(int width, int height);

    std::optional<Point> getMousePosition() const;
    bool pollEvent(Event& event);

    std::string getTitle();
    void setTitle(const std::string& title);

    void setSize(int width, int height);
    const Dimension& getSize() const;

    void clear(const Colour& colour);
    void point(const Point& at, const Colour& colour);
    void rect(const std::string& text, const Point& at, const Dimension& size, const Colour& foreground, const Colour& background);
    void fill(const Point& at, const Dimension& size, const Colour& colour);
    void line(const Point& begin, const Point& end, const Colour& colour);
    void display();
  private:
    void pushEvent(const Event& event);
    static LRESULT CALLBACK keyboardInput(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK mouseInput(int nCode, WPARAM wParam, LPARAM lParam);
    static void input();
  };
}