#include <iostream>
#include "Game.hpp"
#include "Resource.hpp"

int main(int argc, char *argv[])
{
	std::ios::sync_with_stdio(false);

	Game::Init();
	Resource::Init();

	try {
		Game::Loop();
	}
	catch(const std::exception &exception) {
		std::cout << exception.what() << std::endl;
	}

	Game::Terminate();
    return 0;
}
