//======================== intro ============================================
//
//      Space Shooter, basically knock-off Space Invaders
//             and also maybe a bit of Galaga
//   Written by Tyler Edwards for the Hackerbox #0020 badge
//  But should work on any ESP32 and Adafruit ILI9341 display
//
//        I am sorry for the messy code, you'll just
//                  have to live with it
//
//      Tyler on GitHub: https://github.com/HailTheBDFL/
//
//          Hackerboxes: http://hackerboxes.com/
//

//=========================== setup ==============================================
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

//Display pins (current is default for HB0020 Badge)
#define TFT_CS    19
#define TFT_DC    22
#define TFT_MOSI  23
#define TFT_CLK   26
#define TFT_RST   21
#define TFT_MISO  25

// NeoPixel Values
#define PIXELPIN   5
#define NUMPIXELS  5
#define pixlux    20  //saturation level for NeoPixels colors

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

//============================= game variables =================================
unsigned long offsetM = 0;
unsigned long offsetT = 0;
unsigned long offsetF = 0;
unsigned long offsetB = 0;
unsigned long offsetA = 0;
unsigned long offsetAF = 0;
unsigned long offsetAB = 0;
unsigned long offsetS = 0;

int threshold = 40;

boolean startPrinted = false;
boolean beginGame = false;
boolean beginGame2 = true;
boolean play = false;
int score = 0;
int scoreInc = 10;
int level = 1;

//---------------------Player-----------------------------------------------
int shipX = 147;
int shipY = 190;
int oldShipX = 0;
int oldShipY = 0;
int changeShipX = 0;
int changeShipY = 0;
int shipSpeed = 50;
boolean doSplode = false;
boolean fire = false;

int fFireX[5] = {0, 0, 0, 0, 0};
int fFireY[5] = {0, 0, 0, 0, 0};
int fFireAge[5] = {0, 0, 0, 0, 0};


//--------------------------Aliens-----------------------------------------------

boolean alienLive[18];
int alienLiveCount = 18;
int alienX = 7;
int alienY = 25;
int oldAlienX = 7;
int oldAlienY = 25;
int changeAlienX = 6;
int changeAlienY = 0;
int alienSpeed = 200;
int oldAlienSpeed;

int aFireX[5];
int aFireY[5];
boolean aFireAge[5];
int chanceOfFire = 2;

//================================ bitmaps ===================================
//your starship
const int shipImgW = 14;
const int shipImgH = 16;
char shipImg[] = "ZZZZZZWWZZZZZZZZZZZYWWYZZZZZZZZZZWWWWZZZZZZZZZZWWWWZZZZZZZZZWWWWWWZZZZZZZZWWWWWWZZZZZYZZWWWWWWZZYZZYZZWWWWWWZZYZWWZZWWBBWWZZWWWWZZWBBBBWZZWWWWZWWBBBBWWZWWWWZWWWWWWWWZWWWWWWWWWWWWWWWWWRWWWWWWWWWWRWZZWWWWWWWWWWZZZZZWRRWWRRWZZZ";

//flames
const int flamesImgW = 12;
const int flamesImgH = 6;
char flamesImg[] = "RZZZZZZZZZZRRZRYYRRYYRZRRZRYYRRYYRZRZZRYRZZRYRZZZZZRZZZZRZZZZZZRZZZZRZZZ";

//alien
const int alienImgW = 14;
const int alienImgH = 11;
char alienImg[] = "GGGZZZZZZZZGGGZZZGZZZZZZGZZZZZGGGGGGGGGGZZZGGGGGGGGGGGGZGGGZGGGGGGZGGGGGGZZGGGGZZGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGZZZGGZZGGZZZGZGGZGGZZGGZGGZZZZZZGZZGZZZZZ";

//ship 'sploded
const int splodedImgW = 14;
const int splodedImgH = 16;
char splodedImg[] = "ZZZZZZWWZZZZZZZZZZRYWWYRZZZYZZZRRWWRRRRZRWYZRRRRRYYRRRZWYZRYRYYRYYRRRZWWRYYYRYYYYYRZWWRYYRYRYYYYRRWWRYYYRWRYBRRZRRRYRRWWWRYRWZZRYYRRBBWRYRWWZZRYYBBBRRYBWWRZZRYYYRRYYZZWZRRWRYYRBYRZZWZZRYBRYYYYYRRZZRWWYYYWWRRRZZZZWRRWWRRRWZZZ";

//=============================== setup and loop =============================
void setup() {
  memset(alienLive, true, 18);
  memset(aFireX, 0, 5);
  memset(aFireY, 0, 5);
  memset(aFireAge, 0, 5);
  
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(0x5E85);
  tft.setTextSize(4);

  randomSeed(analogRead(6));
  
  touchAttachInterrupt(27, left, threshold);  //Touch input
  touchAttachInterrupt(12, right, threshold);
  touchAttachInterrupt(13, down, threshold);
  touchAttachInterrupt(14, up, threshold);
  touchAttachInterrupt(15, select, threshold);
}

void loop() {
  //-------------Start Screen--------------
  if (millis() - offsetS >= 900 and !beginGame) {
    if (!startPrinted) {
      tft.setCursor(77, 105);
      tft.print(">START<");
      startPrinted = true;
      offsetS = millis();
    }
    else {
      tft.fillRect(77, 105, 244, 32, ILI9341_BLACK);
      startPrinted = false;
      offsetS = millis();
    }
  }
  if (beginGame and beginGame2) {
    tft.fillRect(77, 105, 244, 32, ILI9341_BLACK);
    beginGame2 = false;
    play = true;
  }
  //-------------Player--------------------
  if (millis() - offsetM >= shipSpeed and play) {
    moveShip();
    offsetM = millis();
  }
  if (oldShipX != shipX or oldShipY != shipY) {
    tft.fillRect(oldShipX, oldShipY, 28, 44, ILI9341_BLACK);
    oldShipX = shipX;
    oldShipY = shipY;
    drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
  }
  
  if (fire and play) {
    fireDaLazer();
  }
  if (millis() - offsetB >= 50) {
    for (int i = 0; i < 5; i++) {
      if (fFireAge[i] < 20 and fFireAge[i] > 0) {
        keepFirinDaLazer(i);
      }
      if (fFireAge[i] == 20) {
        stopFirinDaLazer(i);
      }
    }
    offsetB = millis();
  }
  
  if (millis() - offsetT > 50) {
    changeShipX = 0;
    changeShipY = 0;
  }

  //---------------Aliens-----------------------------------
  if (millis() - offsetA >= alienSpeed and play) {
    moveAliens();
    offsetA = millis();
  }
  if (findAlienX(5) >= 294) {
    changeAlienX = -3;
    changeAlienY = 7;
  }
  if (alienX <= 6) {
    changeAlienX = 3;
    changeAlienY = 7;
  }

  alienLiveCount = 0;
  for (int i = 0; i < 18; i++) {
    if (alienLive[i]) {
      alienLiveCount += 1;
      if (alienShot(i)) {
        tft.fillRect(findOldAlienX(i), findOldAlienY(i), 28, 22, ILI9341_BLACK);
        alienLiveCount -= 1;
        alienLive[i] = false;
        score += scoreInc;
      }
      if (onPlayer(i) or exceedBoundary(i)) {
        gameOver();
      }
    }
  }
  if (alienLiveCount == 1) {
    oldAlienSpeed = alienSpeed;
    if (alienSpeed > 50) {
      alienSpeed -= 10;
    }
    else {
      alienSpeed = 20;
    }
  }
  if (alienLiveCount == 0) {
    levelUp();
  }
}

//================================ functions ==================================================================
void gameOver() {
  play = false;
  if (doSplode) {
    drawBitmap(splodedImg, splodedImgW, splodedImgH, shipX, shipY, 2);
  }
  tft.fillScreen(ILI9341_BLACK);
  drawScore(false);
  delay(1000);
  tft.setCursor(17, 168);
  tft.setTextSize(2);
  tft.print("(Reset device to replay)");
  while (true) {}
}

void drawScore(boolean win) {
  tft.setCursor(53, 40);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(4);
  if (win) {
    tft.print("LEVEL UP!");
  }
  else {
    tft.print("GAME OVER");
  }
  for (;millis() - offsetM <= 1000;)
  tft.setCursor(59, 89);
  tft.setTextSize(3);
  tft.print("Score: ");
  tft.print(score);
  offsetM = millis();

  for (;millis() - offsetM <= 1000;) {
  }
  tft.setCursor(71, 128);
  tft.print("Level: ");
  tft.print(level);
}

void levelUp() {
  play = false;
  
  memset(alienLive, true, 18);
  memset(aFireX, 0, 5);
  memset(aFireY, 0, 5);
  memset(aFireAge, 0, 5);
  
  alienX = 7;
  alienY = 25;
  oldAlienX = 7;
  oldAlienY = 25;
  alienSpeed = oldAlienSpeed;
  if (alienSpeed > 100) {
    alienSpeed -= 10;
    chanceOfFire -= 10;
  }
  else if (alienSpeed > 50) {
    alienSpeed -= 10;
    chanceOfFire -=5;
  }
  else if (alienSpeed > 25) {
    alienSpeed -= 5;
    chanceOfFire -=1;
  }
  
  score += 50;
  scoreInc += 5;

  changeShipX = 0;
  changeShipY = 0;
  
  for (unsigned long i = millis(); millis() - i <= 1600;) {
    if (millis() - offsetM >= 20) {
      tft.fillRect(oldShipX, oldShipY, 28, 44, ILI9341_BLACK);
      drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
      drawBitmap(flamesImg, flamesImgW, flamesImgH, shipX + 1, shipY + 32, 2);
      oldShipX = shipX;
      oldShipY = shipY;
      shipY -= 6;
      offsetM = millis();
    }
  }

  drawScore(true);
  level += 1;
  shipX = 147;
  shipY = 190;
  for (; millis() - offsetM <= 4000;) {
  }
  tft.fillScreen(ILI9341_BLACK);
  offsetM = millis();
  play = true;
}


boolean alienShot(int num) {
  for (int i; i < 5; i++) {
    if (fFireAge[i] < 20 and fFireAge[i] > 0) {
      if (fFireX[i] > findAlienX(num) - 4 and fFireX[i] < findAlienX(num) + 28 and fFireY[i] < findAlienY(num) + 22 and fFireY[i] > findAlienY(num) + 4) {
        fFireAge[i] = 20;
        return true;
      }
    }
  }
  return false;
}

boolean onPlayer(int num) {
  if (findAlienX(num) - shipX < 24 and findAlienX(num) - shipX > -28 and findAlienY(num) - shipY < 32 and findAlienY(num) - shipY > -22) {
    doSplode = true;
    return true;
  }
  else {
    return false;
  }
}

boolean exceedBoundary(int num) {
  if (findAlienY(num) > 218) {
    return true;
  }
  else {
    return false;
  }
}

void moveAliens() {
  for (int i = 0; i < 18; i++) {
    if (alienLive[i]) {
      tft.fillRect(findOldAlienX(i), findOldAlienY(i), 28, 22, ILI9341_BLACK);
      drawBitmap(alienImg, alienImgW, alienImgH, findAlienX(i), findAlienY(i), 2);
    }
  }
  oldAlienX = alienX;
  oldAlienY = alienY;
  alienX += changeAlienX;
  alienY += changeAlienY;
  if (changeAlienY != 0) {
    changeAlienY = 0; 
  }
}

int findAlienX(int num) {
  return alienX + 42*(num % 6);
}

int findAlienY(int num) {
  return alienY + 33*(num / 6);
}

int findOldAlienX(int num) {
  return oldAlienX + 42*(num % 6);
}

int findOldAlienY(int num) {
  return oldAlienY + 33*(num / 6);
}

//---------------------------Player----------------------------------------
void fireDaLazer() {
  int bulletNo = -1;
  for (int i = 0; i < 4; i++) {
    if (fFireAge[i] == 0) {
      bulletNo = i;
    }
  }
  if (bulletNo != -1) {
    fFireAge[bulletNo] = 1;
    fFireX[bulletNo] = shipX + 13;
    fFireY[bulletNo] = shipY - 4;
    tft.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 3, ILI9341_MAGENTA);
  }
  fire = false;
}

void keepFirinDaLazer(int bulletNo) {
  tft.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, ILI9341_BLACK);
  fFireY[bulletNo] -= 8;
  tft.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, ILI9341_MAGENTA);
  fFireAge[bulletNo] += 1;
}

void stopFirinDaLazer(int bulletNo) {
  tft.fillRect(fFireX[bulletNo], fFireY[bulletNo], 4, 4, ILI9341_BLACK);
  fFireAge[bulletNo] = 0;
}

void moveShip() {
  if (shipX + changeShipX < 288 and shipX + changeShipX > 6 and changeShipX != 0) {
    shipX += changeShipX;
  }
  if (shipY + changeShipY > 24 and shipY + changeShipY < 192 and changeShipY != 0) {
    shipY += changeShipY;
  }
  if (oldShipX != shipX or oldShipY != shipY) {
    tft.fillRect(oldShipX, oldShipY, 28, 44, ILI9341_BLACK);
    oldShipX = shipX;
    oldShipY = shipY;
    drawBitmap(shipImg, shipImgW, shipImgH, shipX, shipY, 2);
  }
}

void drawBitmap(char img[], int imgW, int imgH, int x, int y, int scale) {
  uint16_t cellColor;
  char curPix;
  for (int i = 0; i < imgW*imgH; i++) {
    curPix = img[i];
    if (curPix == 'W') {
      cellColor = ILI9341_WHITE;
    }
    else if (curPix == 'Y') {
      cellColor = ILI9341_YELLOW;
    }
    else if (curPix == 'B') {
      cellColor = ILI9341_BLUE;
    }
    else if (curPix == 'R') {
      cellColor = ILI9341_RED;
    }
    else if (curPix == 'G') {
      cellColor = 0x5E85;
    }
    if (curPix != 'Z' and scale == 1) {
      tft.drawPixel(x + i % imgW, y + i / imgW, cellColor);
    }
    else if (curPix != 'Z' and scale > 1) {
      tft.fillRect(x + scale*(i%imgW), y + scale*(i/imgW), scale, scale, cellColor);
    }
  }
}

//=========================== button functions ===================================
void up() {
  if (millis() - offsetT >= 50 and play) {
    changeShipX = 0;
    changeShipY = -6;
    offsetT = millis();
  }
}

void down() {
  if (millis() - offsetT >= 50 and play) {
    changeShipX = 0;
    changeShipY = 6;
    offsetT = millis();
  }
}

void left() {
  if (millis() - offsetT >= 50 and play) {
    changeShipX = -6;
    changeShipY = 0;
    offsetT = millis();
  }
}

void right() {
  if (millis() - offsetT >= 50 and play) {
    changeShipX = 6;
    changeShipY = 0;
    offsetT = millis();
  }
}

void select() {
  if (millis() - offsetF >= 500 and play) {
    fire = true;
    offsetF = millis();
  }
  if (!beginGame) {
    beginGame = true;
  }
}
