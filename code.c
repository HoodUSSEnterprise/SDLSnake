#include <SDL3/SDL.h>
#include <stdlib.h>
#include <time.h>

#define CELL_SIZE     20
#define X_SIZE        40
#define Y_SIZE        30
#define WINDOW_WIDTH  (CELL_SIZE * X_SIZE)
#define WINDOW_HEIGHT (CELL_SIZE * Y_SIZE)

// dir
#define LEFT  0
#define RIGHT 1
#define UP    2
#define DOWN  3

int dir[4][2] = {
	{-1,  0 },  // LEFT
	{ 1,  0 },  // RIGHT
	{ 0, -1 },  // UP
	{ 0,  1 }   // DOWN
};

// define some basic types
typedef struct Point
{
	int x;
	int y;
} Point;

typedef struct Snake
{
	Point* body;
	int len;
	int dir;
} Snake;

typedef struct Food
{
	Point pos;
} Food;

// some global varibles
Snake snake;
Food food;
Point snake_tail;
SDL_Surface* snake_surface;
SDL_Texture* snake_texture;
SDL_Surface* food_surface;
SDL_Texture* food_texture;
int next_dir;
int can_change_dir;
Uint64 last_time = 0;
Uint64 interval_time = 200;


int IsOnSnake(int x, int y)
{
	for (int i = 0; i < snake.len; i++)
	{
		if (snake.body[i].x == x && snake.body[i].y == y)
		{
			return 1;
		}
	}
	return 0;
}

void CreateFood()
{
	do
	{
		food.pos.x = rand() % X_SIZE;
		food.pos.y = rand() % Y_SIZE;
	} while (IsOnSnake(food.pos.x, food.pos.y));
}

void InitGame(SDL_Renderer *renderer)
{
	// init snake
	snake.body = (Point*)calloc(X_SIZE * Y_SIZE, sizeof(Point));
	snake.len = 3;
	snake.dir = RIGHT;
	next_dir = RIGHT;
	can_change_dir = 1;

	snake.body[0].x = X_SIZE / 2 + 1;
	snake.body[0].y = Y_SIZE / 2;
	snake.body[1].x = X_SIZE / 2;
	snake.body[1].y = Y_SIZE / 2;
	snake.body[2].x = X_SIZE / 2 - 1;
	snake.body[2].y = Y_SIZE / 2;

	// init food
	do
	{
		food.pos.x = rand() % X_SIZE;
		food.pos.y = rand() % Y_SIZE;
	} while (IsOnSnake(food.pos.x, food.pos.y));

	// init surface and texture
	snake_surface = SDL_LoadBMP("snake.bmp");
	snake_texture = SDL_CreateTextureFromSurface(renderer, snake_surface);
	food_surface = SDL_LoadBMP("food.bmp");
	food_texture = SDL_CreateTextureFromSurface(renderer, food_surface);
}

void DrawSnake(SDL_Renderer* renderer)
{
	for (int i = 0; i < snake.len; i++)
	{
		SDL_FRect rect = { snake.body[i].x * CELL_SIZE, snake.body[i].y * CELL_SIZE,
		CELL_SIZE, CELL_SIZE };
		SDL_RenderTexture(renderer, snake_texture, NULL, &rect);
	}
}

void DrawFood(SDL_Renderer* renderer)
{
	SDL_FRect rect = { food.pos.x * CELL_SIZE, food.pos.y * CELL_SIZE,
	CELL_SIZE, CELL_SIZE };
	SDL_RenderTexture(renderer, food_texture, NULL, &rect);
}

void DrawGameBorad(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

	DrawSnake(renderer);
	DrawFood(renderer);

	SDL_RenderPresent(renderer);
}


void SnakeMove()
{
	snake.dir = next_dir;

	snake_tail.x = snake.body[snake.len - 1].x;
	snake_tail.y = snake.body[snake.len - 1].y;

	for (int i = snake.len - 1; i >= 1; i--)
	{
		snake.body[i].x = snake.body[i - 1].x;
		snake.body[i].y = snake.body[i - 1].y;
	}

	snake.body[0].x += dir[snake.dir][0];
	snake.body[0].y += dir[snake.dir][1];
}

int CheckIfOver()
{
	// hit wall
	if (snake.body[0].x < 0 || snake.body[0].x >= X_SIZE || snake.body[0].y < 0 || snake.body[0].y >= Y_SIZE)
	{
		return 1;
	}

	// hit self
	for (int i = 1; i < snake.len; i++)
	{
		if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y)
		{
			return 1;
		}
	}

	return 0;
}

void CheckIfEatFood()
{
	if (snake.body[0].x == food.pos.x && snake.body[0].y == food.pos.y)
	{
		snake.len++;
		snake.body[snake.len - 1].x = snake_tail.x;
		snake.body[snake.len - 1].y = snake_tail.y;
		CreateFood();
	}
}


void KeyBoradEvent(SDL_Event event)
{
	if (can_change_dir == 0)
	{
		return;
	}

	switch (event.key.key)
	{
	case SDLK_LEFT:
	case SDLK_A:
		if (snake.dir != RIGHT)
		{
			next_dir = LEFT;
			can_change_dir = 0;
		}
		break;
	case SDLK_RIGHT:
	case SDLK_D:
		if (snake.dir != LEFT)
		{
			next_dir = RIGHT;
			can_change_dir = 0;
		}
		break;
	case SDLK_UP:
	case SDLK_W:
		if (snake.dir != DOWN)
		{
			next_dir = UP;
			can_change_dir = 0;
		}
		break;
	case SDLK_DOWN:
	case SDLK_S:
		if (snake.dir != UP)
		{
			next_dir = DOWN;
			can_change_dir = 0;
		}
		break;
	default:
		break;
	}
}

int main(int argc, char** argv)
{
	// random seed
	srand((unsigned)time(NULL));

	// init
	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		SDL_Log("Unable init SDL : %s", SDL_GetError());
		exit(1);
	}

	// create window
	SDL_Window* pwindow = SDL_CreateWindow("Snake", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

	if (pwindow == NULL)
	{
		SDL_Log("Unable create window : %s", SDL_GetError());
		exit(1);
	}

	// create renderer
	SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow, NULL);

	if (prenderer == NULL)
	{
		SDL_Log("Unable create renderer : %s", SDL_GetError());
		exit(1);
	}

	// init game
	InitGame(prenderer);

	int running = 1;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			// handle event
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				KeyBoradEvent(event);
			}
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			{
				running = 0;
			}
		}

		// Game loop
		DrawGameBorad(prenderer);

		Uint64 now_time = SDL_GetTicks();
		if (now_time - last_time >= interval_time)
		{
			last_time = now_time;
			SnakeMove();

			can_change_dir = 1;

			if (CheckIfOver())
			{
				running = 0;
			}

			CheckIfEatFood();
		}
	}

	// free snake body
	free(snake.body);

	SDL_DestroyTexture(snake_texture);
	SDL_DestroySurface(snake_surface);
	SDL_DestroyTexture(food_texture);
	SDL_DestroySurface(food_surface);
	SDL_DestroyRenderer(prenderer);
	SDL_DestroyWindow(pwindow);
	SDL_Quit();

	return 0;
}