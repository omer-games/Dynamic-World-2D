#include <iostream>
#include <thread>
#include <Windows.h>



// Features
const bool ArrayDisp = false;
const bool randomWorld = false;

// Keys
char jumpKey = ' ';// space
char leftKey = 'A';
char rightKey = 'D';
char placeKey = 'X';
char spikeKey = 'P';
char mineKey = 'M';



const int width = 15;
const int height = 8;
const int center = (int)width / 2 - 1;

int seed = 28171548; // can be changed


bool screenChanged = false;




class Object
{
public:
	int value;
	Object() {}
	virtual ~Object() {}
};

class Player : public Object
{
public:
	Player()
	{
		value = 2;
	}
};

class Grass : public Object
{
public:
	Grass()
	{
		value = 1;
	}
};

class Block : public Object
{
public:
	bool sticked;
	Block()
	{
		value = 3;
		sticked = false;
	}
};

class Spike : public Object
{
public:
	int motion;
	Spike()
	{
		value = 4;
		motion = 0;
	}
};




class Logic
{
public:
	static int useSeed(int& seed, int min, int max, bool isRight = true)
	{
		const long long c = 10132143904223;
		const long long m = 2147483647;

		long long seedEval = static_cast<long long>(seed);

		if (isRight || randomWorld)
		{
			seedEval = (seedEval + c) % m;
			if (seedEval < 0)
				seedEval += m;

			seed = static_cast<int>(seedEval);
		}
		else
		{
			seedEval = static_cast<long long>(seed);
			seedEval = (seedEval - c) % m;
			if (seedEval < 0)
				seedEval += m;

			seed = static_cast<int>(seedEval);

			for (int i = 0; i < width - 2; i++)
			{
				seedEval = (seedEval - c) % m;
				if (seedEval < 0)
					seedEval += m;
			}
		}

		int random_number = min + (seedEval % (max - min + 1));

		return random_number;
	}

	static void DisplayWorld(Object*** array, int rows = height * 2, int cols = width * 2)
	{
		if (ArrayDisp)
		{
			for (int i = 0; i < rows; i++)
			{
				std::cout << "\033[F";
				std::cout << "\033[k";
			}
			for (int i = 0; i < rows; ++i)
			{
				for (int j = 0; j < cols; ++j)
				{
					if (array[i][j] == NULL)
						std::cout << "0" << " ";
					else if (array[i][j]->value == 1)
						std::cout << "\033[32m1\033[0m" << " ";
					else if (array[i][j]->value == 2)
						std::cout << "\033[33m2\033[0m" << " ";
					else if (array[i][j]->value == 3)
						std::cout << "\033[31m3\033[0m" << " ";
					else if (array[i][j]->value == 4)
						std::cout << "\033[31m4\033[0m" << " ";
					else
						std::cout << "\033[34m5\033[0m" << " ";
				}
				std::cout << std::endl;
			}
		}
		else
		{
			for (int i = 0; i < rows + 1; i++)
			{
				std::cout << "\033[F";
				std::cout << "\033[k";
			}
			for (int i = 0; i < rows; ++i)
			{
				std::cout << "|";
				for (int j = 0; j < cols; ++j)
				{
					if (array[i][j] == NULL)
						std::cout << " " << " ";
					else if (array[i][j]->value == 1)
						std::cout << "\033[32m#\033[0m" << " ";
					else if (array[i][j]->value == 2)
						std::cout << "\033[33mS\033[0m" << " ";
					else if (array[i][j]->value == 3)
						std::cout << "\033[31mX\033[0m" << " ";
					else if (array[i][j]->value == 4)
						std::cout << "\033[31m~\033[0m" << " ";
					else
						std::cout << "\033[34mE\033[0m" << " ";

				}
				std::cout << "|" << std::endl;
			}
			for (int i = 0; i < rows; ++i)
				std::cout << " _ _";
		}
	}

	static void powerBy2(int**& newWorld, int** world, int height, int width)
	{
		newWorld = new int* [height * 2];
		for (int i = 0; i < height * 2; i++) {
			newWorld[i] = new int[width * 2];
		}
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				newWorld[i * 2][j * 2] = world[i][j];
				newWorld[i * 2][j * 2 + 1] = world[i][j];
				newWorld[i * 2 + 1][j * 2] = world[i][j];
				newWorld[i * 2 + 1][j * 2 + 1] = world[i][j];
			}
		}
	}
};

class Game
{
public:
	bool isHalf = false;
	Object*** world = new Object * *[height * 2];
	int playerPos;

	Game()
	{
		for (int i = 0; i < height * 2; i++)
		{
			world[i] = new Object * [width * 2];
		}

		for (int i = 0; i < height * 2; ++i) {
			for (int j = 0; j < width * 2; ++j) {
				world[i][j] = NULL;
			}
		}

		Initialize_World();
	}
	~Game()
	{
		for (int i = 0; i < height * 2; ++i)
		{
			for (int j = 0; j < width * 2; ++j)
			{
				delete world[i][j];
			}
			delete[] world[i];
		}
		delete[] world;
	}


	int HeightAt(int Pos) const
	{
		for (int i = 0; i < height; i++)
		{
			if (CheckValue(i, Pos) == 1)
			{
				return i;
			}
		}
		throw "Error";
	}

	void Initialize_World(int lastMotion = 0, int lastPos = 0)
	{
		/*
		0 - air
		1 - Grass
		2 - player
		3 - block
		4 - spike
		*/
		if (lastMotion == 0)
		{
			lastPos = Logic::useSeed(seed, 3, height - 1);
			for (int i = lastPos; i < height; i++)
			{
				ChangeWorld(i, 0, 1);
			}
		}

		for (int i = 1; i < width; i++)
		{
			fillRow(i, lastMotion, lastPos, seed);
		}

		playerPos = HeightAt((int)width / 2) * 2;
		ChangeWorld(playerPos / 2 - 1, (int)width / 2, 2);
	}

	void MoveLeft() // <-
	{
		if (world[playerPos - 2][center * 2 + 1] != NULL || world[playerPos - 1][center * 2 + 1] != NULL)
			return;
		int lastPos;
		int lastMotion;

		if (isHalf)
		{
			for (int i = 0; i < height; i++)
			{
				if (world[i * 2][width * 2 - 1] != NULL && (world[i * 2][width * 2 - 1]->value == 3 || world[i * 2][width * 2 - 1]->value == 4))
					if (world[i * 2][width * 2 - 2] == world[i * 2][width * 2 - 1])
						continue;
				delete world[i * 2][width * 2 - 1];
			}
			for (int i = 0; i < height; i++)
			{
				if (world[i * 2][width * 2 - 1] != NULL && (world[i * 2][width * 2 - 1]->value == 3 || world[i * 2][width * 2 - 1]->value == 4))
				{
					if (world[i * 2][width * 2 - 2] == world[i * 2][width * 2 - 1])
					{
						delete world[i * 2][width * 2 - 1];
						world[i * 2][width * 2 - 2] = NULL;
						world[i * 2 + 1][width * 2 - 2] = NULL;
					}
				}
			}
		}

		lastPos = HeightAt(0);
		lastMotion = HeightAt(1) - lastPos;
		for (int i = 1; i < height * 2; i++)
		{
			for (int j = width * 2 - 1; j > 0; j--)
			{
				if (j == 1 && isHalf && world[i][j] != NULL && world[i][j] == world[i][j - 1])
				{
					world[i][j - 1] = NULL;
				}
				else if (world[i][j - 1] != NULL && world[i][j - 1]->value == 2)
				{
					if (world[i][j] == NULL || world[i][j]->value != 2)
						world[i][j] = NULL;
				}
				else if (world[i][j] == NULL || world[i][j]->value != 2)
					world[i][j] = world[i][j - 1];
			}
		}
		if (!isHalf)
		{
			fillRow(0, lastMotion, lastPos, seed, true);
			isHalf = true;
		}
		else
		{
			isHalf = false;
			Logic::useSeed(seed, 1, 15, false);
		}
		screenChanged = true;
	}

	void MoveRight() // ->
	{
		if ((world[playerPos - 2][center * 2 + 4] != NULL) || (world[playerPos - 1][center * 2 + 4] != NULL))
			return;
		int lastPos;
		int lastMotion;

		if (isHalf)
		{
			for (int i = 0; i < height; i++)
			{
				if (world[i * 2][0] != NULL && (world[i * 2][0]->value == 3 || world[i * 2][0]->value == 4))
					if (world[i * 2][1] == world[i * 2][0])
						continue;
				delete world[i * 2][0];
			}
			for (int i = 0; i < height; i++)
			{
				if (world[i * 2][0] != NULL && (world[i * 2][0]->value == 3 || world[i * 2][0]->value == 4))
				{
					if (world[i * 2][1] == world[i * 2][0])
					{
						delete world[i * 2][0];
						world[i * 2][1] = NULL;
						world[i * 2 + 1][1] = NULL;
					}
				}
			}
		}

		lastPos = HeightAt(width - 1);
		lastMotion = lastPos - HeightAt(width - 2);

		for (int i = height * 2 - 1; i >= 0; i--)
		{
			for (int j = 1; j < width * 2; j++)
			{
				if (j == width * 2 - 2 && isHalf && world[i][j] != NULL && world[i][j] == world[i][j + 1]) // PROBLEM HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				{
					world[i][j + 1] = NULL;
				}
				else if (world[i][j] != NULL && world[i][j]->value == 2)
				{
					if (world[i][j - 1] == NULL || world[i][j - 1]->value != 2)
						world[i][j - 1] = NULL;
				}
				else if (world[i][j - 1] == NULL || world[i][j - 1]->value != 2)
					world[i][j - 1] = world[i][j];
			}
		}
		if (!isHalf)
		{
			fillRow(width - 1, lastMotion, lastPos, seed, true);
			isHalf = true;
		}
		else
		{
			isHalf = false;
			Logic::useSeed(seed, 1, 15, true);
		}

		screenChanged = true;

	}

	void ChangeWorld(int i, int j, int value_)
	{
		Object* value = nullptr;
		if (value_ == 0 || value == NULL)
		{
			value = NULL;
		}
		if (value_ == 1)
		{
			value = new Grass();
		}
		if (value_ == 2)
		{
			value = new Player();
		}
		if (value_ == 3)
			value = new Block();
		if (value_ == 4)
			value = new Spike();

		world[i * 2][j * 2] = value;
		world[i * 2][j * 2 + 1] = value;
		world[i * 2 + 1][j * 2] = value;
		world[i * 2 + 1][j * 2 + 1] = value;
	}

	int CheckValue(int i, int j) const
	{
		if (world[i * 2][j * 2] == NULL)
			return NULL;
		return world[i * 2][j * 2]->value;
	}

	void fillRow(int rowPos, int& lastMotion, int& lastPos, int& seed_, bool animated = false)
	{
		int b = Logic::useSeed(seed_, 0, 15, (rowPos == 0) ? false : true);
		if (lastMotion == 1)
		{
			if ((b == 0 || b == 1 || b == 4) && lastPos < height - 1)
			{
				lastPos += 1;
				lastMotion = 1;
			}
			else if (b == 2 && lastPos > 3)
			{
				lastPos -= 1;
				lastMotion = -1;
			}
			else
			{
				lastMotion = 0;
			}
		}
		else if (lastMotion == -1)
		{
			if ((b == 0 || b == 1 || b == 4) && lastPos > 3)
			{
				lastPos -= 1;
				lastMotion = -1;
			}
			else if (b == 2 && lastPos < height - 1)
			{
				lastPos += 1;
				lastMotion = 1;
			}
			else
			{
				lastMotion = 0;
			}
		}
		else
		{
			if ((b == 0 || b == 1 || b == 4) && lastPos < height - 1)
			{
				lastPos += 1;
				lastMotion = 1;
			}
			else if ((b == 2 || b == 3) && lastPos > 3)
			{
				lastPos -= 1;
				lastMotion = -1;
			}
			else
			{
				lastMotion = 1;
			}
		}
		if (animated)
		{

			for (int j = height * 2 - 1; j >= 0; j--)
			{
				if (j >= lastPos * 2)
				{
					if (rowPos == 0)
					{

						Object* value = new Grass();
						world[j][rowPos * 2] = value;
						world[j - 1][rowPos * 2] = value;
					}

					else
					{
						Object* value = new Grass();
						world[j][rowPos * 2 + 1] = value;
						world[j - 1][rowPos * 2 + 1] = value;
					}
				}
				else
				{
					if (rowPos == 0)
						world[j][rowPos * 2] = NULL;
					else
						world[j][rowPos * 2 + 1] = NULL;
				}
			}


		}

		else
		{

			for (int j = lastPos; j < height; j++)
				ChangeWorld(j, rowPos, 1);
		}

	}
	void Gravity(bool playerMoves = true)
	{
		bool changed = false;
		if (playerMoves) // Player
		{
			if (world[playerPos][center * 2 + 3] == NULL && world[playerPos][center * 2 + 2] == NULL)
			{
				world[playerPos][center * 2 + 3] = world[playerPos - 2][center * 2 + 3];
				world[playerPos][center * 2 + 2] = world[playerPos - 2][center * 2 + 2];
				world[playerPos - 2][center * 2 + 3] = NULL;
				world[playerPos - 2][center * 2 + 2] = NULL;
				playerPos += 1;
				changed = true;
			}
		}

		for (int i = height * 2 - 2; i > 0; i--) // Block
		{
			for (int j = width * 2 - 3; j > 0; j--)
			{
				if ((world[i][j] != NULL && world[i][j]->value == 3) && world[i][j + 1] != NULL && world[i][j + 1]->value == 3 && world[i - 1][j] != NULL && world[i - 1][j]->value == 3)
				{
					if (world[i + 1][j] == NULL && world[i + 1][j + 1] == NULL)
					{
						if (!(world[i][j - 1] != NULL && world[i][j - 1]->value == 3 && world[i - 1][j - 1] != NULL && world[i - 1][j - 1]->value == 3 || world[i + 1][j] != NULL && world[i + 1][j]->value == 3 && world[i + 1][j + 1] != NULL && world[i + 1][j + 1]->value == 3 || world[i][j + 2] != NULL && world[i][j + 2]->value == 3 && world[i - 1][j + 2] != NULL && world[i - 1][j + 2]->value == 3 || world[i - 2][j] != NULL && world[i - 2][j]->value == 3 && world[i - 2][j + 1] != NULL && world[i - 2][j + 1]->value == 3))
						{
							world[i + 1][j + 1] = world[i - 1][j];
							world[i + 1][j] = world[i - 1][j + 1];
							world[i - 1][j] = NULL;
							world[i - 1][j + 1] = NULL;
							changed = true;
						}
					}
				}
			}
		}

		for (int i = height * 2 - 2; i > 0; i--) // Spike
		{
			for (int j = width * 2 - 3; j > 0; j--)
			{
				if (world[i][j] != NULL && world[i][j]->value == 4 && world[i][j + 1] != NULL && world[i][j + 1]->value == 4 && world[i - 1][j] != NULL && world[i - 1][j]->value == 4)
				{
					if (world[i + 1][j] == NULL && world[i + 1][j + 1] == NULL)
					{
						world[i + 1][j + 1] = world[i - 1][j];
						world[i + 1][j] = world[i - 1][j + 1];
						world[i - 1][j] = NULL;
						world[i - 1][j + 1] = NULL;
						changed = true;
					}
				}
			}
		}

		if (changed)
			screenChanged = true;
	}

	void Jump(int& jumped)
	{
		if (world[playerPos - 3][center * 2 + 3] == NULL && world[playerPos - 3][center * 2 + 2] == NULL)
			if (playerPos > 3)
			{
				world[playerPos - 3][center * 2 + 3] = world[playerPos - 1][center * 2 + 3];
				world[playerPos - 3][center * 2 + 2] = world[playerPos - 1][center * 2 + 2];
				world[playerPos - 1][center * 2 + 3] = NULL;
				world[playerPos - 1][center * 2 + 2] = NULL;
				playerPos -= 1;
				screenChanged = true;
			}
			else
			{
				jumped = 0;
			}

	}

	bool canJump() const
	{
		if (world[playerPos][center * 2 + 3] != NULL || world[playerPos][center * 2 + 2] != NULL)
			if (world[playerPos - 3][center * 2 + 3] == NULL && world[playerPos - 3][center * 2 + 2] == NULL)
				return true;
		return false;
	}

	bool PlaceBlock()
	{
		if (world[playerPos][center * 2 + 3] == NULL && world[playerPos][center * 2 + 2] == NULL && world[playerPos + 1][center * 2 + 3] == NULL && world[playerPos + 1][center * 2 + 2] == NULL)
		{
			Object* value = new Block();
			world[playerPos][center * 2 + 3] = value;
			world[playerPos][center * 2 + 2] = value;
			world[playerPos + 1][center * 2 + 3] = value;
			world[playerPos + 1][center * 2 + 2] = value;
			screenChanged = true;
			return true;
		}
		return false;
	}

	void GenerateSpike()
	{
		ChangeWorld(playerPos / 2 - 2, (int)width / 2, 4);
		Spike* spike = dynamic_cast<Spike*>(world[(playerPos / 2 - 2) * 2][(int)width]);
		spike->motion = 0;
		screenChanged = true;
	}

	void SpikeLogic()
	{
		bool changed = false;
		for (int i = height * 2 - 1; i > 0; i--)
		{
			for (int j = width * 2 - 2; j >= 0; j--)
			{
				if (world[i][j] != NULL && world[i][j]->value == 4 && world[i - 1][j] == world[i][j])
				{
					if (world[i][j + 1] != world[i][j] && world[i][j - 1] != world[i][j])
					{
						delete world[i][j];
						world[i - 1][j] = NULL;
						world[i][j] = NULL;
					}
					else if (world[i][j + 1] == world[i][j])
					{

						changed = true;
						Spike* spike = dynamic_cast<Spike*>(world[i][j]);
						switch (spike->motion)
						{
						case 1:
							if (j == width * 2 - 2)
							{
								world[i][j] = NULL;
								world[i - 1][j] = NULL;
								world[i][j + 1] = NULL;
								world[i - 1][j + 1] = NULL;
							}
							else if (world[i][j + 2] == NULL && world[i - 1][j + 2] == NULL)
							{
								world[i][j + 2] = world[i][j];
								world[i - 1][j + 2] = world[i][j];
								world[i][j] = NULL;
								world[i - 1][j] = NULL;
							}
							else
							{
								spike->motion = -1;
								changed = false;
							}
							break;
						case -1:
							if (j == 0)
							{
								world[i][j] = NULL;
								world[i - 1][j] = NULL;
								world[i][j + 1] = NULL;
								world[i - 1][j + 1] = NULL;
							}
							else if (world[i][j - 1] == NULL && world[i - 1][j - 1] == NULL)
							{
								world[i][j - 1] = world[i][j];
								world[i - 1][j - 1] = world[i][j];
								world[i][j + 1] = NULL;
								world[i - 1][j + 1] = NULL;
								j--;
							}
							else
							{
								spike->motion = 1;
								changed = false;
							}
							break;
						default:
							if (world[i + 1][j] != NULL || world[i + 1][j + 1] != NULL)
							{
								int FakeSeed = seed - 10;
								spike->motion = Logic::useSeed(FakeSeed, 0, 1) == 1 ? 1 : -1;
							}
							break;

						}

					}
				}
			}
		}
		if (changed)
			screenChanged = true;
	}

	int Mine()
	{

		if (world[playerPos][center * 2 + 3] != NULL)
		{
			if ((world[playerPos][center * 2 + 3]->value != 1 || playerPos <= height * 2 - 6) && world[playerPos][center * 2 + 3] == world[playerPos][center * 2 + 2] && world[playerPos][center * 2 + 2] == world[playerPos + 1][center * 2 + 2] && world[playerPos + 1][center * 2 + 2] == world[playerPos + 1][center * 2 + 3] || (world[playerPos + 1][center * 2 + 3] != NULL && world[playerPos][center * 2 + 3] != NULL && world[playerPos + 1][center * 2 + 2] != NULL && world[playerPos][center * 2 + 2] != NULL && world[playerPos + 1][center * 2 + 3]->value == 1 && world[playerPos + 1][center * 2 + 2]->value == 1 && world[playerPos][center * 2 + 2]->value == 1 && world[playerPos][center * 2 + 3]->value == 1 && playerPos <= height * 2 - 6))
			{
				world[playerPos][center * 2 + 3] = NULL;
				world[playerPos][center * 2 + 2] = NULL;
				world[playerPos + 1][center * 2 + 3] = NULL;
				world[playerPos + 1][center * 2 + 2] = NULL;
				screenChanged = true;
				return 1;
			}
		}

		return 0;
	}

};


std::atomic<bool> jump(false);
std::atomic<bool> moveLeft(false);
std::atomic<bool> moveRight(false);
std::atomic<bool> place(false);
std::atomic<bool> stop(false);
std::atomic<bool> placeSpike(false);
std::atomic<bool> mineBlock(false);



void inputHandler()
{
	while (true)
	{
		jump = GetAsyncKeyState(jump == ' ' ? VK_SPACE : jumpKey) & 0x8000;
		moveLeft = GetAsyncKeyState(leftKey) & 0x8000;
		moveRight = GetAsyncKeyState(rightKey) & 0x8000;
		place = GetAsyncKeyState(placeKey) & 0x8000;
		placeSpike = GetAsyncKeyState(spikeKey) & 0x8000;
		mineBlock = GetAsyncKeyState(mineKey) & 0x8000;
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}
int main()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);

	Game game;

	std::thread inputThread(inputHandler);

	Logic::DisplayWorld(game.world, height * 2, width * 2);


	int jumped = -1;
	bool lastMove = NULL;
	bool resetMove = false;
	int blocks = 0;
	while (!stop)
	{
		if (lastMove != NULL)
			resetMove = true;
		if (place && blocks > 0)
		{
			if (game.PlaceBlock())
				blocks--;
		}
		if (jumped == -1 && jump)
		{
			if (game.canJump())
				jumped = 3;
		}
		if (moveRight)
		{
			if (lastMove == NULL || lastMove == true)
				game.MoveRight();
			lastMove = true;
		}
		if (moveLeft)
		{
			if (lastMove == NULL || lastMove == false)
				game.MoveLeft();
			lastMove = false;
		}
		game.SpikeLogic();
		if (jumped > 0)
		{
			game.Jump(jumped);
			--jumped;
			game.Gravity(false);
		}
		else if (jumped == 0)
		{
			--jumped;
			game.Gravity(false);
		}
		else
			game.Gravity();
		if (placeSpike)
			game.GenerateSpike();
		if (mineBlock)
			blocks += game.Mine();
		jump = false;
		moveLeft = false;
		moveRight = false;
		if (resetMove)
			lastMove = NULL;
		if (screenChanged)
		{
			Logic::DisplayWorld(game.world, height * 2, width * 2);
			screenChanged = false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	Logic::DisplayWorld(game.world, height * 2, width * 2);
	inputThread.join();

}