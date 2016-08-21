#include <FastLED.h>

#define LEDPIN   13
#define REDPIN   6
#define GREENPIN 20
#define BLUEPIN  21
#define WHITEPIN 5




CHSV gColor = {0,255,255};
float gWhite = 0;
float gBreathe = 0;  // low intensity version of gWhite (gWhite wins when it's not zero)


// This version is more robust
void SerialCommand()
{
  int count = 0;
  int num = 0;
  int command = 0;
  
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == 'h' || c == 'b' || c == 's' || c == 'w')
    {
      num = 0;  // always begin anew when command received
      count = 0;
      command = c;
    }
    else
    {
      if (c >= '0' && c <= '9')
      {
        num = (10 * num) + (c - '0') ; // convert digits to a number
        count++;
      }
    }
  }

  switch (command)
  {
    case 'h':
        gColor.h = num;
        Serial.print("Color: ");
        Serial.println(num);
        break;

    case 'b':
        gColor.v = num;
        Serial.print("Brightness: ");
        Serial.println(num);
        break;

    case 's':
        gColor.s = num;
        Serial.print("Saturation: ");
        Serial.println(num);
        break;

    case 'w':
        gWhite = (float)num/100.0;
        Serial.print("White: ");
        Serial.println(num);
        break;
        
    default:
        break;
  }
}





void SetLEDs()
{
    const float kBreatheMax = 0.2;
    
    float white = gWhite;
    if (gWhite == 0)
        white = kBreatheMax * gBreathe;

    const int low = 800;  // lowest raw value where white leds turn off
    const int high = 1700; // highest value we allow for white leds
    
    int rawOutput = white * (high-low) + low;
    if (white == 0)
        rawOutput = 0;
    analogWrite(WHITEPIN, rawOutput);

    CHSV hsv(gColor);
    if (gBreathe > 0)
    {
        uint8_t byteBreate = 255.0 * gBreathe;
        Serial.println(gBreathe);
        Serial.println(byteBreate);
        hsv.s = min(255-byteBreate, hsv.s);
    }
    
    CRGB rgb(hsv);
    analogWrite(REDPIN,   (255-rgb.r) * 16);
    analogWrite(GREENPIN, (255-rgb.g) * 16);
    analogWrite(BLUEPIN,  (255-rgb.b) * 16);
}

// Flash the LEDs
void TestPattern()
{
  for (int i = 0; i < 8; i++)
  {
    gWhite = (i == 0) ? 0 : 1.0;
    SetLEDs();
    delay( 20 );
  }
  gWhite = 0;
}



int gFrame = 0;
int gFrameDelay = 1;


void BreatheWhite()
{
    const int kPeriod = 30000; // frames between breaths
    const int kUpFrame = 4000;
    const int kDownFrame = 6000;;
    int localFrame = gFrame % kPeriod;
    static float sFactor = 1.0;

    if (localFrame < kUpFrame)
        gBreathe = (float)localFrame / kUpFrame;
    else if (localFrame < kDownFrame)
        gBreathe = 1.0 - (float)(localFrame - kUpFrame) / (kDownFrame - kUpFrame);
    else if (localFrame < (kUpFrame + kDownFrame))
        gBreathe = (float)(localFrame - kDownFrame) / kUpFrame;
    else if (localFrame < (kDownFrame*2))
        gBreathe = 1.0 - (float)(localFrame - (kDownFrame + kUpFrame)) / (kDownFrame - kUpFrame);
    else
    {
        gBreathe = 0;
        sFactor = (float)random(50, 100)/100.0;
    }

    gBreathe = gBreathe * sFactor;
}

void WoopWoop()
{
    static CHSV beginWoopColor = gColor; // previous color
    static CHSV endWoopColor = gColor; // target color

    static int beginWoopFrame = gFrame; // frame where the fade started
    static int endWoopFrame = gFrame; // intended frame with full fade

    // Time to select next target?
    if (gFrame >= endWoopFrame)
    {
        beginWoopFrame = gFrame;
        endWoopFrame = beginWoopFrame + ( random16( 1000, 2000 ) / gFrameDelay );
        beginWoopColor = endWoopColor;
        endWoopColor.h = 232;
        endWoopColor.s = 255;
        endWoopColor.v = 255;
        if ( beginWoopColor.v == 255 )
        {
            endWoopColor.v = 0;//random8(0, 128);
        }
    }

    gColor = blend( beginWoopColor, endWoopColor, fract8(((gFrame - beginWoopFrame) * 255) / (endWoopFrame - beginWoopFrame)) );
}

void RandomColorFades()
{
    static CHSV beginColor = gColor; // previous color
    static CHSV endColor = gColor; // target color

    static int beginFrame = gFrame; // frame where the fade started
    static int endFrame = gFrame; // intended frame with full fade

    // Time to select next target?
    if (gFrame >= endFrame)
    {
        beginFrame = gFrame;
        endFrame = beginFrame + ( random16( 1000, 5000 ) / gFrameDelay );
        beginColor = endColor;
        endColor.h = random8();
        endColor.s = 255;
        endColor.v = 255;
        if ( beginColor.s == 255 )
        {
            endColor.s = random8(128, 255);
            endColor.v = 0;//random8(0, 128);
        }
    }

    gColor = blend( beginColor, endColor, fract8(((gFrame - beginFrame) * 255) / (endFrame - beginFrame)) );
}



void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(0));
    
  pinMode( WHITEPIN, OUTPUT );
  pinMode( REDPIN,   OUTPUT );
  pinMode( GREENPIN, OUTPUT );
  pinMode( BLUEPIN,  OUTPUT );

  //analogWriteFrequency(WHITEPIN, 11718.75);
  analogWriteResolution(12);

  TestPattern();
}

void loop() 
{
   gFrame++;

   //SerialCommand();

    const int kPeriod = 100000; // frames between breaths
    int localFrame = gFrame % kPeriod;

    if (localFrame < kPeriod/2)
        RandomColorFades();
    else if (localFrame < kPeriod)
        WoopWoop();
   
   BreatheWhite();
 

  SetLEDs();
  delay(gFrameDelay);
}


