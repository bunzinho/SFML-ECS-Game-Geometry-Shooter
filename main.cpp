#include <SFML/Graphics.hpp>

#include "Game.h"

int main()
{
	static Game g("config.txt");
	g.run();
}