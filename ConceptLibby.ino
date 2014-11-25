/* Copyright Concept Libby 
 * Authors: Vaibhav Bhandari (vaibhavb@gmail.com), and Chris Ryan (chrisr98008@yahoo.com)
 * 
*/

/* Routines performed by this version 
   1. Start or stop lights based on a switch 
   2. Start or stop seahawks flash based on touch
   3. Start or stop rising lights based on ambient light
   4. Start of stop lights based on temperature

   To do
   1. Integrate Lightblue to enable bluetooth support
   2. Refactor the code to organize routines
*/

// Definition of interrupt names
#include <avr/io.h>
// ISR interrupt service routine
#include <avr/interrupt.h>
#include <Adafruit_NeoPixel.h>

//low light level to wait to go over to start
#define LowLightLevel 650

#define PIN 9
const int pinButton = 4;             // pin of button define here
int speakerPin = 6;                  // Grove Buzzer connect to D4
const int pinLight = A0;             // Light Sensor
const int pinTemp = A1;              // pin of temperature sensor
enum
{
  D0=1,
  D1,
  D2,
  D3,
  D4,
  D5,
  D6,
  D7
};
 
volatile int event = 0;
volatile int bBreak = 0;


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);


// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


void setup() {
  Serial.begin(9600);
  Serial.println("setup WaitInTheDark");
  Serial.println("setup begin");
  setup_ambient_sensors();
  lights_setup();
  button_setup();
  speaker_setup();
  InitialiseIO();
  InitialiseInterrupt();
  WaitInTheDark(LowLightLevel);
}


/** main eventloop **/
void loop() {
  Serial.println(event);
  int swVal = event;
  switch(swVal)
  {
    case D2:
      Serial.println("PC D2");
      seahawks_flash();
      break;
    case D3:
      Serial.println("PC D3");
      rainbowCycle(15);
      break;
    case D4:
      Serial.println("PC D4");
      speaker_loop();
      break;
    case D5:
      Serial.println("PC D5");
      break;
    default:
      lights_wakeup_blue();     
     break;
  }
  process_ambient_sensors();
}


/************ I/O and Interupts ***/
void InitialiseIO()
{
  pinMode(2, INPUT);    
  digitalWrite(2, HIGH);   
  pinMode(3, INPUT);    
  digitalWrite(3, HIGH);  
  pinMode(4, INPUT);  
  digitalWrite(4, HIGH);  
  pinMode(4, INPUT);  
  digitalWrite(4, HIGH);  
  pinMode(5, INPUT);  
  digitalWrite(5, HIGH);  
}


void InitialiseInterrupt()
{
  cli();  // switch interrupts off while messing with their settings  
  //Endable Interrupts for PCIE2 Arduino Pins (D0-7)
  PCICR |= (1<<PCIE2);
 
  //Setup pins 2,3,4,5
  PCMSK2 |= (1<<PCINT18);
  PCMSK2 |= (1<<PCINT19);
  PCMSK2 |= (1<<PCINT20);
  PCMSK2 |= (1<<PCINT21);
 
  //Trigger Interrupt on rising edge
  MCUCR = (1<<ISC01) | (1<<ISC01);
  sei();  // turn interrupts back on
}
 
ISR(PCINT2_vect) 
{ 
  if (digitalRead(2)==0)  {
    switch(event) {
      case D2:
      case D3:
      case D4:
      case D5:
       event++;
       break;
      default:
       event = D2;
    }
  }
  if (digitalRead(3)==0)  event=D3; else
  if (digitalRead(4)==0)  event=D4; else
  if (digitalRead(5)==0)  event=D5;
  if(event!=0) bBreak = 1;
}


/************ SENSORS *************/
void setup_ambient_sensors(){
}


void WaitInTheDark(int triggerLevel)
{
  int light = analogRead(pinLight);
  while(light<triggerLevel)
  {
    Serial.println(light);
    delay(300);
    light = analogRead(pinLight);
    if (bBreak) return;
  }
}

void process_ambient_sensors() {
  int light = analogRead(pinLight);
  int B=3975;                  // B value of the thermistor
  int val = analogRead(pinTemp);                               // get analog value
  float resistance=(float)(1023-val)*10000/val;                      // get resistance
  float temperature=1/(log(resistance/10000)/B+1/298.15)-273.15;     // calc temperature
  Serial.println(temperature);
  Serial.println(light);
}


/************ BUTTON **************/
void button_setup()
{
  pinMode(pinButton, INPUT);
}




/************ LIGHTS **************/
void lights_setup()
{
  strip.begin();
  strip.show(); // All Pixels are 'off'
}

void seahawks_flash()
{
  uint16_t g, b;
  Serial.println("seahawks_flash");
  g = strip.Color(0, 255, 0); // Green
  b = strip.Color(0, 0, 255); // Blue
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, g);
      strip.setPixelColor(i*2, b);
      strip.show();
      if (bBreak) { 
        Serial.println("Break");
        bBreak = 0;
        Serial.println(event);
        return;
      }
      delay(15);
  }
}

void lights_wakeup_blue()
{
  uint16_t g;
  Serial.println("lights_wakeup_blue");
  g = strip.Color(0, 0, 0); // dark
  for(uint16_t j=0; j < 256;){
   for(uint16_t i=0; i<strip.numPixels(); i++) {
      g = strip.Color(0, 0, j); // dar
      strip.setPixelColor(i, g);
      strip.setPixelColor(i*2, g);
      strip.show();
      if (bBreak) { 
        Serial.println("Break");
        bBreak = 0;
        Serial.println(event);
        return;
      }
    }
    delay(5);
    j = j + 2;
  }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
    Serial.println("colorWipe");
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      if (bBreak) { 
        Serial.println("Break");
        bBreak = 0;
        Serial.println(event);
        return;
      }
      delay(wait);
  }
}


void rainbow(uint8_t wait) {
  uint16_t i, j;
    Serial.println("rainbow");

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
      if (bBreak) { 
        Serial.println("Break");
        bBreak = 0;
        Serial.println(event);
        return;
      }
    delay(wait);
  }
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

    Serial.println("rainbowCycle");
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
      if (bBreak) { 
        Serial.println("Break");
        bBreak = 0;
        Serial.println(event);
        return;
      }
    delay(wait);
  }
}


//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  Serial.println("theaterChase");
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      if (bBreak) { 
        Serial.println("Break");
        bBreak = 0;
        Serial.println(event);
        return;
      }
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}


//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
          Serial.println("theaterChaseRainbow");
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
      if (bBreak) { 
        Serial.println("Break");
        bBreak = 0;
        Serial.println(event);
        return;
      }
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}




/****** MUSIC ******/
/* Melody
 * (cleft) 2005 D. Cuartielles for K3
 *
 * This example uses a piezo speaker to play melodies.  It sends
 * a square wave of the appropriate frequency to the piezo, generating
 * the corresponding tone.
 *
 * The calculation of the tones is made following the mathematical
 * operation:
 *
 *       timeHigh = period / 2 = 1 / (2 * toneFrequency)
 *
 * where the different tones are described as in the table:
 *
 * note  frequency  period  timeHigh
 * c          261 Hz          3830  1915
 * d          294 Hz          3400  1700
 * e          329 Hz          3038  1519
 * f          349 Hz          2864  1432
 * g          392 Hz          2550  1275
 * a          440 Hz          2272  1136
 * b          493 Hz          2028 1014
 * C         523 Hz         1912  956
 *
 * http://www.arduino.cc/en/Tutorial/Melody
 */


int length = 15; // the number of notes
char notes[] = "ccggaagffeeddc "; // a space represents a rest
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;


void playTone(int tone, int duration) {
    for (long i = 0; i < duration * 1000L; i += tone * 2) {
        digitalWrite(speakerPin, HIGH);
        delayMicroseconds(tone);
        digitalWrite(speakerPin, LOW);
        delayMicroseconds(tone);
    }
}


void playNote(char note, int duration) {
    char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
    int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };


    // play the tone corresponding to the note name
    for (int i = 0; i < 8; i++) {
        if (names[i] == note) {
            playTone(tones[i], duration);
        }
    }
}


void speaker_setup()
{
    pinMode(speakerPin, OUTPUT);
}


void speaker_loop() 
{
    for (int i = 0; i < length; i++) 
    {
        if (notes[i] == ' ')
        {
            delay(beats[i] * tempo); // rest
        }
        else
        {
            playNote(notes[i], beats[i] * tempo);
            /*
            for (int i=0; i < strip.numPixels(); i=i+3) {
              strip.setPixelColor(i, notes[i]/4);        //turn every third pixel off
            }
            */
        }
        if (bBreak) { bBreak=0; return;}
        // pause between notes
        delay(tempo / 2);
    }
}

