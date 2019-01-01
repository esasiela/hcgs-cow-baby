#include <HC_BouncyButton.h>

#include <MIDI.h>
#include <FastLED.h>
#include <Servo.h>


#define PIN_BUTTON_GREEN 2
#define PIN_BUTTON_RED 3
#define PIN_BUTTON_BLUE 4



#define PIN_PIXEL 6

#define PIN_SERVO_RIGHT 7
#define PIN_SERVO_LEFT 8

#define PIN_SERIAL_RX 10
#define PIN_SERIAL_TX 11

#define PIN_POT_ORANGE A0
#define PIN_POT_RED A1
#define PIN_POT_YELLOW A2

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, midiCtl);


// hardcode compatibility with HC Midi Mud
#define MIDI_MUD_BLUE 40
#define MIDI_MUD_GREEN 36


BouncyButton btnBlue = BouncyButton(PIN_BUTTON_BLUE);
BouncyButton btnRed = BouncyButton(PIN_BUTTON_RED);
BouncyButton btnGreen = BouncyButton(PIN_BUTTON_GREEN);


#define PIXEL_COUNT 3

#define PIXEL_IDX_LEFT 2
#define PIXEL_IDX_RIGHT 1
#define PIXEL_IDX_UTIL 0

uint8_t brightness;
CRGB fastled[PIXEL_COUNT];

CRGB channelModeColors[17] = {
  CRGB::Purple,
  CRGB::Yellow,
  CRGB::Cyan,
  CRGB::Red,
  CRGB::Blue,
  CRGB::Green,
  CRGB::Purple,
  CRGB::Yellow,
  CRGB::Cyan,
  CRGB::Red,
  CRGB::Blue,
  CRGB::Green,
  CRGB::Purple,
  CRGB::Yellow,
  CRGB::Cyan,
  CRGB::Red,
  CRGB::Blue
};

uint8_t myMidiChannel;
uint8_t channelSelectMode;



uint16_t nowMillis;

uint16_t leftStrikeMillis;
uint16_t rightStrikeMillis;

uint16_t leftStrikeDuration;
uint16_t rightStrikeDuration;


uint16_t redDownMillis;
uint16_t redDownDuration;




uint8_t noteLeft;
uint8_t noteRight;

Servo servoLeft;
Servo servoRight;

uint8_t servoRightRestPos;
uint8_t servoRightStrikePos;

uint8_t servoLeftRestPos;
uint8_t servoLeftStrikePos;





/**********************************************************
 * setup()
 **********************************************************/
void setup() {

  // TODO - move brightness to eeprom
  brightness = 100;

  FastLED.addLeds<NEOPIXEL, PIN_PIXEL>(fastled, PIXEL_COUNT);
  FastLED.setBrightness(brightness);

  for (int i=0; i<PIXEL_COUNT; i++) {
    fastled[i] = CRGB::Black;
  }
  FastLED.show();



  // TODO - move servo positions to eeprom
  // 115 is just about touching for right/small drum (reasonable values: rest=110, strike=135, delay=100)
  // right/small drum, higher number moves the hammer lower
  /*
  servoRightRestPos = 100;
  servoRightStrikePos = 130;
  rightStrikeDuration = 100; 
  */
  servoRightRestPos = 100;
  servoRightStrikePos = 140;
  rightStrikeDuration = 100; 


  // left/big drum, higher number moves the hammer up higher (reasonable values: rest=80, strike=63, delay=100)
  servoLeftRestPos = 80;
  servoLeftStrikePos = 63;
  leftStrikeDuration = 100;


  servoRight.attach(PIN_SERVO_RIGHT);
  servoRight.write(servoRightRestPos);
  
  servoLeft.attach(PIN_SERVO_LEFT);
  servoLeft.write(servoLeftRestPos);



  // TODO - move midi channel to eeprom
  myMidiChannel = MIDI_CHANNEL_OMNI;

  midiCtl.setHandleNoteOn(noteOn);
  midiCtl.setHandleNoteOff(noteOff);
  midiCtl.begin(myMidiChannel);


  // TODO - move note mappings to eeprom (midi-mud uses 36, 40)
  noteLeft = 93;
  noteRight = 96;




  pinMode(PIN_BUTTON_BLUE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_RED, INPUT_PULLUP);
  pinMode(PIN_BUTTON_GREEN, INPUT_PULLUP);

  btnBlue.init();
  btnRed.init();
  btnGreen.init();


  redDownDuration=5000;

}


/**********************************************************
 * loop()
 **********************************************************/
void loop() {

  nowMillis = millis();

  midiCtl.read();


  if (leftStrikeMillis>0 && ((nowMillis - leftStrikeMillis)>leftStrikeDuration)) {
    // it has been long enough since a strike, move to rest position
    restLeft();
  }

  if (rightStrikeMillis>0 && ((nowMillis - rightStrikeMillis)>rightStrikeDuration)) {
    // it has been long enough since a strike, move to rest position
    restRight();
  }


  if (redDownMillis>0 && ((nowMillis - redDownMillis)>redDownDuration)) {
    fastled[PIXEL_IDX_UTIL] = CRGB::Yellow;
    
    channelSelectMode=1;
    
    FastLED.show();
  }

  if (channelSelectMode==1) {

    // check the red knob.  there are 17 zones for OMNI plus 16 midi channels.  if we've entered a new zone, change the midi channel.
    //int redVal = map(analogRead(PIN_POT_RED), 0, 1023, 0, 16);
    int redVal = analogRead(PIN_POT_RED);
/*
    if (redVal < 60) {
      // 0
      fastled[PIXEL_IDX_UTIL] = CRGB::Purple;
    } else if (yellowVal < 120) {
      // 1
      fastled[PIXEL_IDX_UTIL] = CRGB::Green;
    } else if (yellowVal < 180) {
      // 2
      fastled[PIXEL_IDX_UTIL] = CRGB::Orange;
    } else {
      fastled[PIXEL_IDX_UTIL] = CRGB::Blue;
    }
*/
    redVal = map(redVal, 0, 1023, 0, 17);
    fastled[PIXEL_IDX_UTIL] = channelModeColors[redVal];

    myMidiChannel = redVal;
    
    /*
    switch (redVal) {
      case 0 : 
        fastled[PIXEL_IDX_UTIL] = CRGB::Purple;
        break;
      case 1 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Yellow;
        break;
      case 2 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Pink;
        break;
      case 3 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 4 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 5 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 6 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 7 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 8 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 9 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 10 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 3 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 3 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 3 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 3 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 3 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      case 3 :
        fastled[PIXEL_IDX_UTIL] = CRGB::Green;
        break;
      default :
        fastled[PIXEL_IDX_UTIL] = CRGB::Blue;
        break;
    }
    */
    FastLED.show();

    
  }



  if (btnBlue.update()) {
    if (!btnBlue.getState()) {
      // press event
      strikeLeft();
    }
  }

  if (btnGreen.update()) {
    if (!btnGreen.getState()) {
      // press event
      strikeRight();
    }
  }


  if (btnRed.update()) {
    if (btnRed.getState()) {
      // release event
      if (channelSelectMode!=1) {
        fastled[PIXEL_IDX_UTIL] = CRGB::Black;
      }
        

      redDownMillis=0;
  
      
    } else {
      // press event
      fastled[PIXEL_IDX_UTIL] = CRGB::Red;
      channelSelectMode=0;
      redDownMillis = nowMillis;
    }
    FastLED.show();
  }



//  servoRight.write(map(analogRead(PIN_POT_YELLOW), 0, 1023, 180, 0));
//  servoLeft.write(map(analogRead(PIN_POT_ORANGE), 0, 1023, 0, 180));



}

/***********************************************
 * strikeLeft()
 ***********************************************/
void strikeLeft() {

  leftStrikeMillis = millis();

  servoLeft.write(servoLeftStrikePos);

  fastled[PIXEL_IDX_LEFT] = CRGB::Blue;
  // CRGB(red, green, blue)
  FastLED.show();
  
}

/***********************************************
 * restLeft()
 ***********************************************/
void restLeft() {
  leftStrikeMillis=0;

  servoLeft.write(servoLeftRestPos);
  
  fastled[PIXEL_IDX_LEFT] = CRGB::Black;
  FastLED.show();
  
}



/***********************************************
 * strikeRight()
 ***********************************************/
void strikeRight() {

  rightStrikeMillis = millis();

  servoRight.write(servoRightStrikePos);

  fastled[PIXEL_IDX_RIGHT] = CRGB::Green;
  // CRGB(red, green, blue)
  FastLED.show();
 
}


/***********************************************
 * restRight()
 ***********************************************/
void restRight() {
  rightStrikeMillis=0;

  servoRight.write(servoRightRestPos);
  
  fastled[PIXEL_IDX_RIGHT] = CRGB::Black;
  FastLED.show();
  
}




/***********************************************
 * noteOn()
 ***********************************************/
void noteOn(byte channel, byte note, byte velocity) {
  
  if (velocity==0) {
    noteOff(channel, note, velocity);
  } else {
    if (myMidiChannel==MIDI_CHANNEL_OMNI || myMidiChannel==channel) {

      if (note == noteLeft || note == MIDI_MUD_BLUE) {
        // left
        strikeLeft();
      
      } else if (note==noteRight || note == MIDI_MUD_GREEN) {
        // right
        strikeRight();
      }
    }
    FastLED.show();
  }
}

/***********************************************
 * noteOff()
 ***********************************************/
void noteOff(byte channel, byte note, byte velocity) {

  if (myMidiChannel==MIDI_CHANNEL_OMNI || myMidiChannel==channel) {

    if (note == noteLeft || note == MIDI_MUD_BLUE) {
      // left
      //restLeft();
      
    } else if (note==noteRight || note == MIDI_MUD_GREEN) {
      // right
      //fastled[PIXEL_IDX_RIGHT] = CRGB::Black;
    }
    FastLED.show();
  }
} 
