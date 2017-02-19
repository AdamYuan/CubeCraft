#pragma once
#include "world.hpp"
#include "player.hpp"
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
namespace game
{
extern sf::Window win;
extern sf::Event evt;
extern int width,height;
extern world wld;
extern player gamePlayer;

extern void init_gl();
extern void init();
extern void loop();
extern void keyControl();
extern void procEvent();
extern void render();
};
