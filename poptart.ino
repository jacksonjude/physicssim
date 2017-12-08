#include <SPI.h>
#include <EEPROM.h>
#include <Arduboy2.h>
//#include "bitmaps.h"

Arduboy2 arduboy;

#define FRAMES_PER_SECOND 30  // The update and refresh speed

#define BALL_RADIUS 4
#define BALL_Y_START ((HEIGHT / 2) - 1) // The height Floaty begins at
#define PIXEL_TO_METER 1.00

float xBounceMultiplier = 0.5;
float yBounceMultiplier = 0.5;

const byte PROGMEM arrow [] = {0xE0, 0xF8, 0xFC, 0xF8, 0xE0, 0x00};

float ballY = 10;      // Floaty's height
float ballX = 10;

int ballFlapper = BALL_RADIUS; // Floaty's wing length
int gameState = 0;
int frameCount = 0;

float yBallVelocity = 0;
float xBallVelocity = 0;

float gForce = -9.81;
float normalForce = 0.0;
float yAddForce = 0.0;

bool reversedYDirection = false;
bool reversedXDirection = false;

bool statNamesOn = true;

int choiceSelected = 0;
int pageOn = 1;

bool koCode = false;
String enteredCode = "";
size_t codeSize = 0;

bool saveFileMenu = false;
int fileSelected = 1;

bool justSavedFile = false;
int savedFileFrame = 0;

float lineX1 = -618;
float lineX2 = 618;
float lineY1 = -618;
float lineY2 = -618;

float lineX1s = 0;
float lineX2s = WIDTH-30;
float lineY1s = HEIGHT-30;
float lineY2s = 0;

float wasJustOnLine = false;

float files[][7] = {
{30, 5, 0, 1, 80, 5, 1},
{30, 500, 0, 1, 80, 500, 1},
{30, 5, 100, 1, 80, 5, 1},
};

bool hideLine = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  arduboy.begin();
  arduboy.setFrameRate(FRAMES_PER_SECOND);
  
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!arduboy.nextFrame())
    return;

  frameCount++;

  arduboy.clear();

  if (justSavedFile)
  {
    savedFileFrame++;
    if (savedFileFrame > 60)
    {
      justSavedFile = false;
      savedFileFrame = 0;
    }

    arduboy.setCursor(WIDTH-37, 0);
    arduboy.print("KOCODE");
  }

  if (!saveFileMenu)
  {
    if ((frameCount % (1)) == 0)
    {
      switch (gameState)
      {
        case 0:
          moveFloaty();
          drawFloaty();      
      }
    }
  
    int xPosToPrintValues = 0;
  
    if (pageOn != 0)
    {
      arduboy.drawSlowXYBitmap(0,(choiceSelected*10) + 1,arrow,8,6,10);
      xPosToPrintValues = 10;
    }
  
    switch (pageOn)
    {
      case 1:
      arduboy.setCursor(xPosToPrintValues, 0);
      printStat(HEIGHT - ballY, "YP");
    
      arduboy.setCursor(xPosToPrintValues, 10);
      printStat(yBallVelocity, "YV");
    
      arduboy.setCursor(xPosToPrintValues, 20);
      printStat(getYNetForce(), "YA");
  
      arduboy.setCursor(xPosToPrintValues, 30);
      printStat(yBounceMultiplier, "YB");
      break;
      case 2:
      arduboy.setCursor(xPosToPrintValues, 0);
      printStat(ballX, "XP");
    
      arduboy.setCursor(xPosToPrintValues, 10);
      printStat(xBallVelocity, "XV");
  
      arduboy.setCursor(xPosToPrintValues, 20);
      printStat(xBounceMultiplier, "XB");
      break;
      case 3:
      arduboy.setCursor(xPosToPrintValues, 0);
      printStat(lineX1, "X1");
    
      arduboy.setCursor(xPosToPrintValues, 10);
      printStat(lineY1, "Y1");
    
      arduboy.setCursor(xPosToPrintValues, 20);
      printStat(lineX2, "X2");
  
      arduboy.setCursor(xPosToPrintValues, 30);
      printStat(lineY2, "Y2");

      arduboy.setCursor(xPosToPrintValues, 40);
      printStat(618, "Roll");
      
      arduboy.setCursor(xPosToPrintValues, 50);
      printStat(618, "Toggl");
      break;
    }
  
    bool buttonsPressed = false;
    bool enteredCodeThisTime = false;
  
    if (arduboy.pressed(UP_BUTTON))
    {
      buttonsPressed = true;
  
      if (!enteredCodeThisTime)
      {
        enteredCode += (String)UP_BUTTON;
        enteredCodeThisTime = true;
      }
  
      choiceSelected--;
    }
  
    if (arduboy.pressed(DOWN_BUTTON))
    {
      buttonsPressed = true;
  
      if (!enteredCodeThisTime)
      {
        enteredCode += (String)DOWN_BUTTON;
        enteredCodeThisTime = true;
      }
      choiceSelected++;
    }
    
    if (choiceSelected < 0)
    {
      switch (pageOn)
      {
        case 0:
        choiceSelected = 0;
        break;
        case 1:
        choiceSelected = 3;
        break;
        case 2:
        choiceSelected = 2;
        break;
        case 3:
        choiceSelected = 5;
      }
    }
  
    switch (pageOn)
    {
      case 0:
      if (choiceSelected > 0)
      {
        choiceSelected = 0;
      }
      break;
      case 1:
      if (choiceSelected > 3)
      {
        choiceSelected = 0;
      }
      break;
      case 2:
      if (choiceSelected > 2)
      {
        choiceSelected = 0;
      }
      break;
      case 3:
      if (choiceSelected > 5)
      {
        choiceSelected = 0;
      }
      break;
    }

    bool changedBounceMultiplier = false;
  
    while (arduboy.buttonsState() & (RIGHT_BUTTON | LEFT_BUTTON))
    {
      arduboy.clear();

      float valueToDisplay = 0.0;
      switch (pageOn)
      {
        case 1:
        switch (choiceSelected)
        {
          case 0:
            valueToDisplay = HEIGHT - ballY;
            break;
          case 1:
            valueToDisplay = yBallVelocity;
            break;
          case 2:
            valueToDisplay = yAddForce;
            break;
          case 3:
            valueToDisplay = yBounceMultiplier;
            break;
        }
        break;
        case 2:
        switch (choiceSelected)
        {
          case 0:
            valueToDisplay = ballX;
            break;
          case 1:
            valueToDisplay = xBallVelocity;
            break;
          case 2:
            valueToDisplay = xBounceMultiplier;
            break;
        }
        break;
        case 3:
        switch (choiceSelected)
        {
          case 0:
            valueToDisplay = lineX1;
            break;
          case 1:
            valueToDisplay = lineY1;
            break;
          case 2:
            valueToDisplay = lineX2;
            break;
          case 3:
            valueToDisplay = lineY2;
            break;
          case 5:
            valueToDisplay = hideLine;
            break;
        }
        break;
      }
      
      arduboy.setCursor(10, (choiceSelected*10));
      if (valueToDisplay < 0)
      {
        arduboy.print("   " + (String)valueToDisplay);
      }
      else
      {
        arduboy.print("    " + (String)valueToDisplay);
      }
      arduboy.display();
  
      if (arduboy.pressed(RIGHT_BUTTON))
      {
        switch (pageOn)
        {
          case 1:
          switch (choiceSelected)
          {
            case 0:
              ballY -= 0.1;
              break;
            case 1:
              yBallVelocity += 0.01;
              break;
            case 2:
              yAddForce += 0.01;
              break;
            case 3:
              yBounceMultiplier += 0.001;
              changedBounceMultiplier = true;
              break;
          }
          break;
          case 2:
          switch (choiceSelected)
          {
            case 0:
              ballX += 0.1;
              break;
            case 1:
              xBallVelocity += 0.01;
              break;
            case 2:
              xBounceMultiplier += 0.001;
              changedBounceMultiplier = true;
              break;
          }
          break;
          case 3:
          switch (choiceSelected)
          {
            case 0:
              lineX1 += 0.1;
              break;
            case 1:
              lineY1 += 0.1;
              break;
            case 2:
              lineX2 += 0.1;
              break;
            case 3:
              lineY2 += 0.1;
              break;
            case 4:
              debounceButtons();
              rollBall();
              break;
            case 5:
              debounceButtons();
              hideLine = !hideLine;
              if (!hideLine)
              {
                lineX1 = lineX1s;
                lineX2 = lineX2s;
                lineY1 = lineY1s;
                lineY2 = lineY2s;
              }
              else
              {
                lineX1s = lineX1;
                lineX2s = lineX2;
                lineY1s = lineY1;
                lineY2s = lineY2;

                lineX1 = -618;
                lineX2 = 618;
                lineY1 = -618;
                lineY2 = -618;
              }
              break;
          }
          break;
        }
        
        if (!enteredCodeThisTime)
        {
          enteredCode += (String)LEFT_BUTTON;
          enteredCodeThisTime = true;
        }
      }
  
      if (arduboy.pressed(LEFT_BUTTON))
      {
        switch (pageOn)
        {
          case 1:
          switch (choiceSelected)
          {
            case 0:
              ballY += 0.1;
              break;
            case 1:
              yBallVelocity -= 0.01;
              break;
            case 2:
              yAddForce -= 0.01;
              break;
            case 3:
              yBounceMultiplier -= 0.001;
              changedBounceMultiplier = true;
          }
          break;
          case 2:
          switch (choiceSelected)
          {
            case 0:
              ballX -= 0.1;
              break;
            case 1:
              xBallVelocity -= 0.01;
              break;
            case 2:
              xBounceMultiplier -= 0.001;
              changedBounceMultiplier = true;
              break;
          }
          break;
          case 3:
          switch (choiceSelected)
          {
            case 0:
              lineX1 -= 0.1;
              break;
            case 1:
              lineY1 -= 0.1;
              break;
            case 2:
              lineX2 -= 0.1;
              break;
            case 3:
              lineY2 -= 0.1;
              break;
          }
          break;
        }
        
        if (!enteredCodeThisTime)
        {
          enteredCode += (String)RIGHT_BUTTON;
          enteredCodeThisTime = true;
        }
      }
      
      buttonsPressed = true;
    }

    if (changedBounceMultiplier)
    {
      xBounceMultiplier = round(xBounceMultiplier*100.0)/100.0;
      yBounceMultiplier = round(yBounceMultiplier*100.0)/100.0;
    }

    bool aButtonPressed = false;
    int timePressed = 0;
    while (arduboy.buttonsState() & (A_BUTTON))
    {
      aButtonPressed = true;
      buttonsPressed = true;
  
      if (!enteredCodeThisTime)
      {
        enteredCode += (String)A_BUTTON;
        enteredCodeThisTime = true;
      }

      timePressed++;

      delay(1);
    }

    if (aButtonPressed)
    {
      if (timePressed < 750)
      {
        pageOn++;
  
        if (pageOn > 3)
        {
          pageOn = 1;
        }
      }
      else
      {
        saveCurrentData(fileSelected);
      }
    }
  
    if (arduboy.buttonsState() & (B_BUTTON))
    {
      buttonsPressed = true;
  
      if (!enteredCodeThisTime)
      {
        enteredCode += (String)B_BUTTON;
        enteredCodeThisTime = true;
      }
    
      switch (pageOn)
      {
        case 1:
        switch (choiceSelected)
        {
          case 0:
            ballY = HEIGHT;
            break;
          case 1:
            yBallVelocity = 0;
            break;
          case 2:
            yAddForce = 0;
            break;
          case 3:
            yBounceMultiplier = 0.9;
            break;
        }
        break;
        case 2:
        switch (choiceSelected)
        {
          case 0:
            ballX = 0;
            break;
          case 1:
            xBallVelocity = 0;
            break;
          case 2:
            xBounceMultiplier = 0.9;
            break;
        }
        break;
        case 3:
        switch (choiceSelected)
        {
          case 0:
            lineX1 = 0;
            break;
          case 1:
            lineY1 = 0;
            break;
          case 2:
            lineX2 = 0;
            break;
          case 3:
            lineY2 = 0;
            break;
        }
        break;
      }
    }
    
    if (buttonsPressed)
    {
      debounceButtons();
    }
    
    if (enteredCode == "12812816166432643248")
    {
      koCode = true;
    }
  
    if (koCode)
    {
      for (int i = 0; i < 6; i++)
      {
        arduboy.clear();
        if (i % 2 == 0)
        {
          arduboy.setCursor(WIDTH - 37, 0);
          arduboy.print("KOCODE!");
        }
        arduboy.display();
  
        if (i % 3 == 0)
        {
          digitalWrite(RED_LED, LOW);
        }
  
        if (i % 3 == 1)
        {
          digitalWrite(GREEN_LED, LOW);
        }
  
        if (i % 3 == 2)
        {
          digitalWrite(BLUE_LED, LOW);
        }
  
        delay(250);
  
        digitalWrite(BLUE_LED, HIGH);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(RED_LED, HIGH);
      }
  
      koCode = false;
      enteredCode = "";
  
      saveFileMenu = true;
    }
  
    String codeToOutput = "";
    for (int i = 0; sizeof(enteredCode) > i; i++)
    {
      codeToOutput += (String)enteredCode[i] + ", ";
    }

    arduboy.drawLine(lineX1, HEIGHT-lineY1, lineX2, HEIGHT-lineY2, WHITE);
  }
  else
  {
    drawSaveFileMenu();

    bool buttonsPressed = false;

    if (arduboy.pressed(UP_BUTTON))
    {
      buttonsPressed = true;
      
      fileSelected--;
    }
    
    if (arduboy.pressed(DOWN_BUTTON))
    {
      buttonsPressed = true;
      
      fileSelected++;
    }

    if (arduboy.pressed(A_BUTTON))
    {
      loadFromArray(files[fileSelected-1]);
      debounceButtons();
      saveFileMenu = false;
    }

    if (fileSelected < 1)
    {
      fileSelected = 3;
    }

    if (fileSelected > 3)
    {
      fileSelected = 1;
    }

    if (buttonsPressed)
    {
      debounceButtons();
    }
  }
  
  //Serial.println(pointIsOnLine(2, 8, 0, 2, 4, 14));
  //Serial.println(calculateSlopeAngle(0, 2, 4, 14));
  
  arduboy.display();
}

void drawFloaty() {
  ballFlapper--;
  if (ballFlapper < 0) { ballFlapper = BALL_RADIUS; }  // Flapper starts at the top of the ball
  arduboy.drawCircle(ballX, (int)ballY, BALL_RADIUS, BLACK);  // Black out behind the ball
  arduboy.drawCircle(ballX, (int)ballY, BALL_RADIUS, WHITE);  // Draw outline
  arduboy.drawLine(ballX, (int)ballY, ballX - (BALL_RADIUS+1), (int)ballY - ballFlapper, WHITE);  // Draw wing
  arduboy.drawPixel(ballX - (BALL_RADIUS+1), (int)ballY - ballFlapper + 1, WHITE);  // Dot the wing
  arduboy.drawPixel(ballX + 1, (int)ballY - 2, WHITE);  // Eye
}

void moveFloaty()
{
  if (pointIsOnLine(ballX, ballY, lineX1, lineY1, lineX2, lineY2))
  {
    if (!wasJustOnLine)
    {
      wasJustOnLine = true;
      yBallVelocity = 0;
      xBallVelocity = 0;
    }
  }
  else
  {
    wasJustOnLine = false;
  }
  
  float yNetForce = getYNetForce();
  
  yBallVelocity += yNetForce / (float)FRAMES_PER_SECOND;

  ballY += (yBallVelocity / PIXEL_TO_METER * (-1.00));

  Serial.println((String)yNetForce + ":" + (String)yBallVelocity + ":" + (String)((float)HEIGHT - ballY));
  
  if ((ballY >= (float)HEIGHT || ballY <= 0) && !reversedYDirection)
  {
    if (yBallVelocity != 0.0)
    {
       yBallVelocity *= -(yBounceMultiplier);
       reversedYDirection = true;
    }
  }

  if (ballY > (float)0 && ballY < (float)HEIGHT && reversedYDirection)
  {
    reversedYDirection = false;
  }

  float xNetForce = getXNetForce();
  
  xBallVelocity += xNetForce / (float)FRAMES_PER_SECOND;
  
  ballX += (xBallVelocity / PIXEL_TO_METER);

  if ((ballX > (float)WIDTH || ballX < 0) && !reversedXDirection)
  {
    if (xBallVelocity != 0.0)
    {
       xBallVelocity *= -(xBounceMultiplier);
       reversedXDirection = true;
    }
  }

  if (ballX > (float)0 && ballX < (float)WIDTH && reversedXDirection)
  {
    reversedXDirection = false;
  }
}

float getYNetForce()
{
  float sumForces1 = gForce + yAddForce;
  float directionForceSum = 0.0;

  if (sumForces1 > 0.0)
  {
    directionForceSum = -sumForces1;
  }

  if (sumForces1 < 0.0)
  {
    directionForceSum = sumForces1;
  }
  
  if ((ballY >= (float)HEIGHT && (round(yBallVelocity*100.0)/100.0) <= 0.16))
  {
    normalForce = -directionForceSum;

    ballY = (float)HEIGHT;

    yBallVelocity = 0;
  }
  else if ((ballY <= 0.0 && (round(yBallVelocity*100.0)/100.0) >= 0.16))
  {
    normalForce = directionForceSum;

    ballY = (float)0;

    yBallVelocity = 0;
  }
  else
  {
    normalForce = 0;
  }

  float yNetForce = sumForces1 + normalForce;

  if (pointIsOnLine(ballX, ballY, lineX1, lineY1, lineX2, lineY2))
  {
    float trigTmp = sin(calculateSlopeAngle(lineX1, lineY1, lineX2, lineY2));
    yNetForce = pow(trigTmp,2)*sumForces1;
    Serial.println(pow(trigTmp,2));
  }

  if (!reversedYDirection)
  {
    return yNetForce;
  }
  else
  {
    return 0;
  }
}

float getXNetForce()
{
  if ((ballX >= (float)WIDTH && (round(xBallVelocity*100.0)/100.0) <= 0.26) && !reversedXDirection)
  {
    ballX = (float)WIDTH;

    xBallVelocity = 0;
  }
  else if ((ballX <= 0.0 && (round(xBallVelocity*100.0)/100.0) >= 0.26) && !reversedXDirection)
  {
    ballX = (float)0;

    xBallVelocity = 0;
  }
  
  float xNetForce = 0;
  
  if (pointIsOnLine(ballX, ballY, lineX1, lineY1, lineX2, lineY2))
  {
    float trigTmp1 = sin(calculateSlopeAngle(lineX1, lineY1, lineX2, lineY2));
    float trigTmp2 = cos(calculateSlopeAngle(lineX1, lineY1, lineX2, lineY2));
    xNetForce = trigTmp1*trigTmp2*gForce;
  }

  return xNetForce;
}

float absoluteVal(float value)
{
  if (value < 0)
  {
    return value * -1;
  }
  else
  {
    return value;
  }
}

void debounceButtons() {
  delay(100);
  while (arduboy.buttonsState()) { }  // Wait for all keys released
  delay(100);
}

void printStat(float statValue, String statName)
{
  if ((statValue) < 0)
  {
    if (statNamesOn)
    {
      arduboy.print(statName + "=" + (String)statValue);
    }
    else
    {
      arduboy.print((String)statValue);
    }
  }
  else
  {
    if (statNamesOn)
    {
      arduboy.print(statName + "= " + (String)(statValue));
    }
    else
    {
      arduboy.print(" " + (String)(statValue));
    }
  }
}

void drawSaveFileMenu()
{
  arduboy.setCursor(6, 3);
  arduboy.print("Files");
  arduboy.drawFastHLine(5, 11, 31, WHITE);

  printFile(1, "File 1");
  printFile(2, "File 2");
  printFile(3, "File 3");
  
  arduboy.drawSlowXYBitmap(0,(fileSelected*10) + 7,arrow,8,6,10);
}

void printFile(int fileNumber, String fileName)
{
  arduboy.setCursor(16, fileNumber*10+6);
  arduboy.print(fileName);
}

void saveCurrentData(int fileNumber)
{
  Serial.println("Something....");
  enteredCode = "";
  justSavedFile = true;
}

void loadFromArray(float fileArray[7])
{
  ballY = fileArray[0];
  yBallVelocity = fileArray[1];
  yAddForce = fileArray[2];
  yBounceMultiplier = fileArray[3];

  ballX = fileArray[4];
  xBallVelocity = fileArray[5];
  xBounceMultiplier = fileArray[6];
}

bool pointIsOnLine(float px, float py, float x1, float y1, float x2, float y2)
{
  float slope = (y2 - y1) / (x2 - x1);
  float yIntercept = (slope * x1) + y1;

  float lineResult = (slope * px) + yIntercept;

  Serial.println(String(round(lineResult)) + " == " + String(round(HEIGHT-py)));
  
  if (-3 < (round(lineResult) - round(HEIGHT-py)) && (round(lineResult) - round(HEIGHT-py)) < 3)
  {
    return true;
  }
  else
  {
    return false;
  }
}

float calculateSlopeAngle(float x1, float y1, float x2, float y2)
{
  float yDiff = (y2 - y1);
  float xDiff = (x2 - x1);

  return atan(yDiff / xDiff);

  //*4068/71
}

float rollBall()
{
  ballX = lineX1;
  ballY = 1;

  yBallVelocity = 0;
  xBallVelocity = 0;
}

