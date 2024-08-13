#include <deque>
#include <raylib.h>
#include <raymath.h>

Color green     = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

int cellSize  = 30;
int cellCount = 25;
int offset    = 75;

double lastUpdateTime = 0;
bool   eventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

bool ElementInDeque(Vector2 element, std::deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++) {
        if (Vector2Equals(deque[i], element))
            return true;
    }
    return false;
}

class Food
{
public:
    Vector2 position;
    Texture texture;
    Food(std::deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Graphics/food.png");
        texture     = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }
    ~Food() { UnloadTexture(texture); }

    void Draw()
    {
        DrawTexture(texture, position.x * cellSize + offset, position.y * cellSize + offset, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }
    Vector2 GenerateRandomPos(std::deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody)) {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Snake
{
public:
    std::deque<Vector2> body       = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2             direction  = {1, 0};
    bool                addSegment = false;

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++) {
            float     x        = body[i].x;
            float     y        = body[i].y;
            Rectangle segement = Rectangle{
                x * cellSize + offset, y * cellSize + offset, float(cellSize), float(cellSize)};
            DrawRectangleRounded(segement, 0.5, 6, darkGreen);
        }
    }

    void update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true) {
            addSegment = false;
        }
        else {
            body.pop_back();
        }
    }
    void Reset()
    {
        body      = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Game
{
public:
    Snake snake   = Snake();
    Food  food    = Food(snake.body);
    bool  running = true;
    int   score   = 0;
    Sound eatSound;
    Sound wallSound;

    Game()
    {
        InitAudioDevice();
        eatSound  = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }
    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
    }
    void Update()
    {
        if (running) {
            snake.update();
            CheckCollisionWithFood();
            CheckCollisonWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position)) {
            food.position    = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score += 1;
            PlaySound(eatSound);
        }
    }

    void CheckCollisonWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1) {
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1) {
            GameOver();
        }
    }

    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running       = false;
        score         = 0;
        PlaySound(wallSound);
    }
    void CheckCollisionWithTail()
    {
        std::deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody)) {
            GameOver();
        }
    }
};

int main()
{

    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);
    Game game = Game();

    while (!WindowShouldClose()) {
        BeginDrawing();
        if (eventTriggered(0.1)) {
            game.Update();
        }

        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = {0, -1};
            game.running         = true;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = {0, 1};
            game.running         = true;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = {-1, 0};
            game.running         = true;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = {1, 0};
            game.running         = true;
        }
        DrawText("snake", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("%i", game.score),
                 offset - 5,
                 offset + cellSize * cellCount + 10,
                 40,
                 darkGreen);
        game.Draw();
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{float(offset - 5),
                                       float(offset - 5),
                                       float(cellSize * cellCount + 10),
                                       float(cellSize * cellCount + 10)},
                             5,
                             darkGreen);
        EndDrawing();
    }
    CloseWindow();
}
