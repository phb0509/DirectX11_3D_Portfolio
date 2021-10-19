#include "Framework.h"

GameManager::GameManager()
{
	player = new Player();

	for (int i = 0; i < 20; i++)
	{
		monster = new Mutant;
		monsters.push_back(monster);
	}

	
	for (int i = 0; i < monsters.size(); i++)
	{
		monstersHitCheck[monsters[i]] = false;
	}
}

GameManager::~GameManager()
{
	delete player;
	delete monster;
}
