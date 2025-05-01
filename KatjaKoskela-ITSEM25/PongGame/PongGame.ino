#include <TFT_eSPI.h>

#define ROTARY_CLK_PIN 17
#define ROTARY_DT_PIN  18
#define ROTARY_SW_PIN  19

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

#define PADDLE_WIDTH  10
#define PADDLE_HEIGHT 40
#define BALL_SIZE     10

TFT_eSPI tft = TFT_eSPI();

int playerY = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
int ballX = SCREEN_WIDTH / 2;
int ballY = SCREEN_HEIGHT / 2;
int ballSpeedX = 3;
int ballSpeedY = 2;
int score = 0;

void drawPaddle() {
  tft.fillRect(10, playerY, PADDLE_WIDTH, PADDLE_HEIGHT, TFT_GREEN);
}

void drawBall() {
  tft.fillRect(ballX, ballY, BALL_SIZE, BALL_SIZE, TFT_RED);
}

void drawScore() {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(SCREEN_WIDTH / 2 - 40, 10);
  tft.print("Score: ");
  tft.print(score);
}

void resetGame() {
  playerY = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  ballSpeedX = 3;
  ballSpeedY = 2;
  score = 0;
}

void handleInput() {
  static int lastStateCLK = digitalRead(ROTARY_CLK_PIN);
  int currentStateCLK = digitalRead(ROTARY_CLK_PIN);

  if (currentStateCLK != lastStateCLK && currentStateCLK == LOW) {
    int dtState = digitalRead(ROTARY_DT_PIN);
    if (dtState != currentStateCLK) {
      playerY += 5; // alas
    } else {
      playerY -= 5; // ylös
    }
    playerY = constrain(playerY, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  }
  lastStateCLK = currentStateCLK;

  static int lastSwState = HIGH;
  int swState = digitalRead(ROTARY_SW_PIN);
  if (swState == LOW && lastSwState == HIGH) {
    resetGame();
    delay(200);
  }
  lastSwState = swState;
}

void updateBall() {
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  if (ballY <= 0 || ballY >= SCREEN_HEIGHT - BALL_SIZE) {
    ballSpeedY *= -1;
  }

  if (ballX <= 10 + PADDLE_WIDTH &&
      ballY + BALL_SIZE >= playerY &&
      ballY <= playerY + PADDLE_HEIGHT) {
    ballSpeedX *= -1;
    ballX = 10 + PADDLE_WIDTH + 1;
    score++;
  }

  if (ballX > SCREEN_WIDTH) {
    resetGame();
  }
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);  // For round screens, usually rotation 0 is correct
  tft.fillScreen(TFT_BLACK);

  pinMode(ROTARY_CLK_PIN, INPUT_PULLUP);
  pinMode(ROTARY_DT_PIN, INPUT_PULLUP);
  pinMode(ROTARY_SW_PIN, INPUT_PULLUP);

  resetGame();
}

void loop() {
  handleInput();
  updateBall();

  tft.fillScreen(TFT_BLACK);
  drawPaddle();
  drawBall();
  drawScore();

  delay(30);
}
