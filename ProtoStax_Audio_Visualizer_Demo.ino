/*************************************************** 
  ProtoStax Audio Visualizer Demo

  This is a example sketch for an Audio Visualizer using Arduino,  
   
  SparkFun Spectrum Shield --> https://www.sparkfun.com/products/13116 ,
  Adafruit NeoPixel Shield --> https://www.adafruit.com/product/1430 ,
  and 
  ProtoStax for Arduino --> https://www.protostax.com/products/protostax-for-arduino

  It analyzes the frequency spectrum of the audio input and visualizes it using 
  an RGB LED matrix with different schemes. 
 
  Written by Sridhar Rajagopal for ProtoStax
  BSD license. All text above must be included in any redistribution
 */


#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

//Declare Spectrum Shield pin connections
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1 

//Define spectrum variables
int freq_amp;
int Frequencies_One[7];
int Frequencies_Two[7]; 

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


// Example for NeoPixel Shield used in Audio Visualizer Demo.
// In this application we'd like to use it
// as a 5x8 tall matrix, with the USB port positioned at the top of the
// Arduino.  When held that way, the first physical pixel is at the top right, and
// lines are arranged in columns, progressive order.  The shield uses
// 800 KHz (v2) pixels that expect GRB color data.
// See https://www.hackster.io/sridhar-rajagopal/rgb-matrix-audio-visualizer-with-arduino-845062
// for more details on how to choose the parameters


#define NEO_MATRIX_WIDTH 5
#define NEO_MATRIX_HEIGHT 8

#define NEOPIXEL_PIN 6 // Shield maps it to pin 6

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(NEO_MATRIX_WIDTH, NEO_MATRIX_HEIGHT, NEOPIXEL_PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

enum RANGE {
  BASS = 0,
  MID_RANGE = 1,
  TREBLE = 2,
  ALL = 3
};

enum SCHEME {
  MAGNITUDE_HUE = 0,
  MAGNITUDE_HUE_2 = 1,
  HSV_COLOR_WHEEL = 2
};

/********************Setup *************************/
void setup() {
  Serial.begin(9600);
  Serial.println("ProtoStax Audio Visualizer Demo");
  Serial.println("**************************************************");

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.fillScreen(0);
  matrix.show();
  
  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);  
  digitalWrite(STROBE, HIGH);
  digitalWrite(RESET, HIGH);
  
  //Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, HIGH);
  delay(1);
  digitalWrite(STROBE, HIGH);
  delay(1);
  digitalWrite(STROBE, LOW);
  delay(1);
  digitalWrite(RESET, LOW);
}


/************************** Loop*****************************/
void loop() {
  static int scheme = 0;
  while (Serial.available() > 0) {
    scheme = Serial.parseInt();
  }
  
  Read_Frequencies();
  Graph_Frequencies(ALL, scheme);
  // Print_Frequencies();
  delay(50);
 
}

int max_bass_freq = 0;
int max_mid_freq = 0;
int max_treble_freq = 0;

/*******************Pull frquencies from Spectrum Shield********************/
void Read_Frequencies(){
  max_bass_freq = 0;
  max_mid_freq = 0;
  max_treble_freq = 0;

  //Read frequencies for each band
  for (freq_amp = 0; freq_amp<7; freq_amp++)
  {
    Frequencies_One[freq_amp] = (analogRead(DC_One) + analogRead(DC_One) ) >> 1 ;
    Frequencies_Two[freq_amp] = (analogRead(DC_Two) + analogRead(DC_Two) ) >> 1; 

    if (freq_amp >= 0 && freq_amp < 2) {
        if (Frequencies_One[freq_amp] > max_bass_freq) 
          max_bass_freq = Frequencies_One[freq_amp];
        if (Frequencies_Two[freq_amp] > max_bass_freq) 
          max_bass_freq = Frequencies_Two[freq_amp];  
    }
    else if (freq_amp >= 2 && freq_amp < 5) {
        if (Frequencies_One[freq_amp] > max_mid_freq) 
          max_mid_freq = Frequencies_One[freq_amp];
        if (Frequencies_Two[freq_amp] > max_mid_freq) 
          max_mid_freq = Frequencies_Two[freq_amp];  
    }
    else if (freq_amp >= 5 && freq_amp < 7) {
        if (Frequencies_One[freq_amp] > max_treble_freq) 
          max_treble_freq = Frequencies_One[freq_amp];
        if (Frequencies_Two[freq_amp] > max_treble_freq) 
          max_treble_freq = Frequencies_Two[freq_amp];  
    }    

    digitalWrite(STROBE, HIGH);
    digitalWrite(STROBE, LOW);
  }
}

int FREQ_DIV_FACTOR = 204;

/*******************Light LEDs based on frequencies*****************************/
void Graph_Frequencies(RANGE r, SCHEME s){
   int from = 0;
   int to = 0;

   switch(r) {
    case BASS:
      from = 0;
      to = 2;
      break;
    case MID_RANGE:
      from = 2; 
      to = 5;
      break;
    case TREBLE:
      from = 5;
      to = 7;
      break;
    case ALL:
      from = 0;
      to = 7;
      break;
    default:
      break;
   }
  
   // Serial.print("max freq is "); Serial.println(max_freq);
   // FREQ_DIV_FACTOR = max_bass_freq/4;
   // Serial.print("FREQ_DIV_FACTOR is "); Serial.println(FREQ_DIV_FACTOR); 
   
   static uint16_t hue = 0; //21845 22250 to -250 
   uint16_t hueDelta = 200;
   hue += hueDelta;

    
   uint16_t bassHue = 22250; 
   uint16_t midHue = 22250; //54613 
   uint16_t trebleHue = 22250; //43690

      
   matrix.fillScreen(0);
   uint32_t rgbcolor;
   for(int row= from; row<to; row++)
   {

     int freq = (Frequencies_Two[row] > Frequencies_One[row])?Frequencies_Two[row]:Frequencies_One[row]; 

     
     int numCol = (freq/FREQ_DIV_FACTOR);
     if (numCol > 5) numCol = 5;
    
     for (int col = 0 ; col < numCol ; col++) {
       switch(s) {
        case MAGNITUDE_HUE:
          bassHue = 22250; 
          midHue = 22250; //54613 
          trebleHue = 22250; //43690
          if (row >= 0 && row < 2) {
            rgbcolor = matrix.ColorHSV(bassHue - (7416 * col) );      
          } else if (row >= 2 && row < 5) {
            rgbcolor = matrix.ColorHSV(midHue - (7416 * col) );      
            
          } else if (row >= 5 && row < 7) {
            rgbcolor = matrix.ColorHSV(trebleHue - (7416 * col) );      
          } 
          break;          
        case MAGNITUDE_HUE_2:
          bassHue = 54613; 
          midHue = 54613; //54613 
          trebleHue = 54613; //43690        
          if (row >= 0 && row < 2) {
            rgbcolor = matrix.ColorHSV(bassHue - (7416 * col) );      
          } else if (row >= 2 && row < 5) {
            rgbcolor = matrix.ColorHSV(midHue - (7416 * col) );      
            
          } else if (row >= 5 && row < 7) {
            rgbcolor = matrix.ColorHSV(trebleHue - (7416 * col) );      
          }        
          break;
        case HSV_COLOR_WHEEL:
          rgbcolor = matrix.ColorHSV(hue);
          break;
       }

      
        matrix.setPassThruColor(rgbcolor);      
        matrix.drawPixel(col, row, (uint16_t)0); // color does not matter here 
        matrix.setPassThruColor();
        
        //matrix.show();
     
     }
     matrix.show();
   }
}

// Used for debugging 
void Print_Frequencies() {
  for (int i = 0; i < 7; i++) {
    Serial.print("FreqOne["); Serial.print(i); Serial.print( "]:"); Serial.println((Frequencies_One[i]/FREQ_DIV_FACTOR)%5); 
    Serial.print("FreqTwo["); Serial.print(i); Serial.print( "]:"); Serial.println((Frequencies_Two[i]/FREQ_DIV_FACTOR)%5); 

  }
}



