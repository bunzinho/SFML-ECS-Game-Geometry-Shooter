#include <SFML/Graphics.hpp>

#include "Game.h"

int main()
{
	std::cout << std::boolalpha;
	static Game g("config.txt");
	g.run();
}
