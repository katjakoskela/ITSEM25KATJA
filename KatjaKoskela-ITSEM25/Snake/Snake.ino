#include <TFT_eSPI.h>

#define ROTARY_CLK_PIN 17
#define ROTARY_DT_PIN  18
#define ROTARY_SW_PIN  19

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240
#define GRID_SIZE 10
#define GRID_WIDTH (SCREEN_WIDTH / GRID_SIZE)
#define GRID_HEIGHT (SCREEN_HEIGHT / GRID_SIZE)

TFT_eSPI tft = TFT_eSPI();

enum Direction { UP, RIGHT, DOWN, LEFT };
Direction dir = RIGHT;

struct Point {
  int x;
  int y;
};

Point snake[GRID_WIDTH * GRID_HEIGHT];
int snakeLength = 3;

Point food;

bool gameOver = false;
unsigned long lastMoveTime = 0;
const int moveInterval = 150;

void drawPixel(int x, int y, uint16_t color) {
  tft.fillRect(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE, color);
}

void placeFood() {
  bool placed = false;
  while (!placed) {
    food.x = random(GRID_WIDTH);
    food.y = random(GRID_HEIGHT);
    placed = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        placed = false;
        break;
      }
    }
  }
}

void resetGame() {
  dir = RIGHT;
  snakeLength = 3;
  for (int i = 0; i < snakeLength; i++) {
    snake[i] = {5 - i, 5};
  }
  placeFood();
  gameOver = false;
  lastMoveTime = millis();
  tft.fillScreen(TFT_BLACK);
}

void handleInput() {
  static int lastCLK = digitalRead(ROTARY_CLK_PIN);
  int currentCLK = digitalRead(ROTARY_CLK_PIN);

  if (currentCLK != lastCLK && currentCLK == LOW) {
    int dtState = digitalRead(ROTARY_DT_PIN);
    if (dtState != currentCLK) {
      dir = (Direction)((dir + 1) % 4); // clockwise
    } else {
      dir = (Direction)((dir + 3) % 4); // counter-clockwise
    }
  }
  lastCLK = currentCLK;

  static int lastSwState = HIGH;
  int swState = digitalRead(ROTARY_SW_PIN);
  if (swState == LOW && lastSwState == HIGH) {
    resetGame();
    delay(200);
  }
  lastSwState = swState;
}

void moveSnake() {
  if (millis() - lastMoveTime < moveInterval) return;
  lastMoveTime = millis();

  Point head = snake[0];
  Point newHead = head;

  switch (dir) {
    case UP:    newHead.y--; break;
    case DOWN:  newHead.y++; break;
    case LEFT:  newHead.x--; break;
    case RIGHT: newHead.x++; break;
  }

  if (newHead.x < 0 || newHead.x >= GRID_WIDTH ||
      newHead.y < 0 || newHead.y >= GRID_HEIGHT) {
    gameOver = true;
    return;
  }

  for (int i = 0; i < snakeLength; i++) {
    if (snake[i].x == newHead.x && snake[i].y == newHead.y) {
      gameOver = true;
      return;
    }
  }

  for (int i = snakeLength; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  snake[0] = newHead;

  if (newHead.x == food.x && newHead.y == food.y) {
    snakeLength++;
    placeFood();
  } else {
    drawPixel(snake[snakeLength].x, snake[snakeLength].y, TFT_BLACK);
  }
}

void drawSnake() {
  for (int i = 0; i < snakeLength; i++) {
    drawPixel(snake[i].x, snake[i].y, i == 0 ? TFT_GREEN : TFT_DARKGREEN);
  }
}

void drawFood() {
  drawPixel(food.x, food.y, TFT_RED);
}

void drawGameOver() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(50, SCREEN_HEIGHT / 2 - 20);
  tft.println(" GAME OVER ");
  tft.setCursor(40, SCREEN_HEIGHT / 2 + 10);
  tft.println("Click to Restart");
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  pinMode(ROTARY_CLK_PIN, INPUT_PULLUP);
  pinMode(ROTARY_DT_PIN, INPUT_PULLUP);
  pinMode(ROTARY_SW_PIN, INPUT_PULLUP);
  randomSeed(analogRead(0));
  resetGame();
}

void loop() {
  handleInput();
  if (!gameOver) {
    moveSnake();
    drawSnake();
    drawFood();
  } else {
    drawGameOver();
  }
}
