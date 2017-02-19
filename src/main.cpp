#include <iostream>
#include <string>
#include <fstream>
#include "game.hpp"
#include "resource.hpp"

int main(int argc, char *argv[])
{
	std::ios::sync_with_stdio(false);
	game::init_gl();
	resource::init();
	game::init();
	game::loop();
    return 0;
}
