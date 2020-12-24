/*
  ATmega328P Maximum Open e-Drum Kit
  
  Buttons Circuit:
    Button for EDIT to digital pin 6
    Button for UP to digital pin 7
    Button for DOWN to digital pin 8
    Button for NEXT to digital pin 9
    Button for BACK to digital pin 10

  MUX 4051
  mux1 ----- Arduino
  S0   ----- 2
  S1   ----- 3
  S2   ----- 4
  Z    ----- A7

  mux2 ----- Arduino
  S0   ----- 5
  S1   ----- 11
  S2   ----- 12
  Z    ----- A6

  i2c oled display
  A4 SDA
  A5 SCL

  FSR HIHAT Pedal
  3.3V --> 220kOhm --> A0 <-- FSR <-- GND
  
   !!! This works with version 0.7.3 of the Hello Drum library !!!
   
*/


//  GitHUb : https://github.com/RyoKosaka/HelloDrum-arduino-Library
//  Blog : https://open-e-drums.tumblr.com/
#include "hellodrum.h"

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

#include <U8x8lib.h>
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE); 
//unsigned long displayDelay;

HelloDrumMUX_4051 mux1(2,3,4,A7); 
HelloDrumMUX_4051 mux2(5,11,12,A6); 

HelloDrum kick(0);
HelloDrum snare(1,2);
HelloDrum tom_1(3);
HelloDrum tom_2(4);
HelloDrum tom_3(5);
HelloDrum tom_4(6,7);
HelloDrum hihat(8);
HelloDrum cym_1(9);
HelloDrum cym_2(10);
HelloDrum ride(11,12); 

#define HIHAT_PEDAL A0
#define PEDAL_MIN  29
#define PEDAL_MAX  450
int pedalAvgValue = 0;
byte pedalEighthOfAvgValue = 0;
byte pedalOldEighthOfAvgValue = 0;
//unsigned long pedalTimeOfChange;
//unsigned long pedalVelocity = 0;

HelloDrumButton button(6, 7, 8, 9, 10); //(EDIT,UP,DOWN,NEXT,BACK)
bool MyEditState = false;

void setup()
{
    //If you use Hairless MIDI, you have to comment out the next line.
    //MIDI.begin(10);
    //And uncomment the next two lines. Please set the baud rate of Hairless to 38400.
    MIDI.begin();
    Serial.begin(38400);

//    displayDelay = millis();
//    pedalTimeOfChange = millis();

    u8x8.begin();
    u8x8.setFont(u8x8_font_7x14B_1x2_r);
    u8x8.clear();
    u8x8.setCursor(1, 2);
    u8x8.print(F("Hello Drum!"));
  
    //Give each pad a name to be displayed on the LCD.
    //It is necessary to make the order in exactly the same order as you named the pad first.
    kick.settingName("KICK");
    snare.settingName("SNARE");
    tom_1.settingName("TOM 1");
    tom_2.settingName("TOM 2");
    tom_3.settingName("TOM 3");
    tom_4.settingName("TOM 4");
    hihat.settingName("HIHAT");
    cym_1.settingName("CYMBAL 1");
    cym_2.settingName("CYMBAL 2");
    ride.settingName("RIDE");

    //Load settings from EEPROM.
    //It is necessary to make the order in exactly the same order as you named the pad first.
    kick.loadMemory();
    snare.loadMemory();
    tom_1.loadMemory();
    tom_2.loadMemory();
    tom_3.loadMemory();
    tom_4.loadMemory();
    hihat.loadMemory();
    cym_1.loadMemory();
    cym_2.loadMemory();
    ride.loadMemory();
}

void loop()
{

    button.readButtonState();

    kick.settingEnable();
    snare.settingEnable();
    tom_1.settingEnable();
    tom_2.settingEnable();
    tom_3.settingEnable();
    tom_4.settingEnable();
    hihat.settingEnable();
    cym_1.settingEnable();
    cym_2.settingEnable();
    ride.settingEnable();

  if (button.GetPushState())
  {
    u8x8.clear();
    u8x8.setFont(u8x8_font_7x14B_1x2_r);
    u8x8.print(button.GetPadName());
    u8x8.setCursor(0, 2);
    u8x8.print(button.GetSettingItem());
    u8x8.setCursor(0, 5);
    u8x8.setFont(u8x8_font_profont29_2x3_r);
    if ( MyEditState) u8x8.setInverseFont(1);
    u8x8.print(button.GetSettingValue());
    u8x8.setInverseFont(0);
    do button.readButtonState(); while (button.GetPushState());
  }
  if (button.GetEditState()){
    MyEditState = true;
    u8x8.setFont(u8x8_font_7x14B_1x2_r);
    u8x8.setCursor(0, 0);
    u8x8.print(button.GetPadName());
    u8x8.setCursor(0, 2);
    u8x8.print(button.GetSettingItem());
    u8x8.setCursor(0, 5);
    u8x8.setFont(u8x8_font_profont29_2x3_r);
    u8x8.setInverseFont(1);
    u8x8.print(button.GetSettingValue());
    u8x8.setInverseFont(0);
  }
  if (button.GetEditdoneState()){
    MyEditState = false ;
    u8x8.setCursor(0, 5);
    u8x8.setInverseFont(0);
    u8x8.print(button.GetSettingValue());
  }
  
//  //show hitted pad name and velocity (This slows down the program. Its use is not recommended.)
//  if ( button.GetDisplayState() && displayDelay < millis())
//  {
//    u8x8.setCursor(0, 6);
//    u8x8.print(F("                "));
//    u8x8.setCursor(0, 6);
//    u8x8.print(button.GetHitPad());
//    u8x8.setCursor(10, 6);
//    u8x8.print(button.GetVelocity());
//    displayDelay = millis()+50;
//  }

  //Sensing each pad.
    mux1.scan();
    mux2.scan();

    kick.singlePiezoMUX();   
    snare.singlePiezoMUX();
//    snare.dualPiezoMUX();    
    tom_1.singlePiezoMUX();  
    tom_2.singlePiezoMUX();  
    tom_3.singlePiezoMUX();  
    tom_4.singlePiezoMUX();  
    hihat.singlePiezoMUX();           
    cym_1.singlePiezoMUX();
    cym_2.singlePiezoMUX();
//    ride.cymbal2zone();
  
    //Sending MIDI signals.
    //KICK//
    if (kick.hit == true)
    {
      MIDI.sendNoteOn(kick.note, kick.velocity, 10); //(note, velocity, channel)
      MIDI.sendNoteOff(kick.note, 0, 10);
    }
   
     //SNARE//
    if (snare.hit == true)
    {
      MIDI.sendNoteOn(snare.note, snare.velocity, 10); //(note, velocity, channel)
      MIDI.sendNoteOff(snare.note, 0, 10);
    }
    else if (snare.hitRim == true)
    {
      if (snare.velocity > 20) //Rim shot
      {
        MIDI.sendNoteOn(snare.noteRim, snare.velocity *10 , 10); //(note, velocity, channel)
        MIDI.sendNoteOff(snare.noteRim, 0, 10);
      }
      else //Side Stick
      {
        MIDI.sendNoteOn(snare.noteCup, snare.velocity * 20, 10); //(note, velocity, channel)
        MIDI.sendNoteOff(snare.noteCup, 0, 10);
      }
    }

    //TOM 1//
    if (tom_1.hit == true)
    {
      MIDI.sendNoteOn(tom_1.note, tom_1.velocity, 10); //(note, velocity, channel)
      MIDI.sendNoteOff(tom_1.note, 0, 10);
    }
  
    //TOM 2//
    if (tom_2.hit == true)
    {
      MIDI.sendNoteOn(tom_2.note, tom_2.velocity, 10); //(note, velocity, channel)
      MIDI.sendNoteOff(tom_2.note, 0, 10);
    }
 
    //TOM 3//
    if (tom_3.hit == true)
    {
      MIDI.sendNoteOn(tom_3.note, tom_3.velocity, 10); //(note, velocity, channel)
      MIDI.sendNoteOff(tom_3.note, 0, 10);
    }
  
    //TOM 4//
    if (tom_4.hit == true)
    {
      MIDI.sendNoteOn(tom_4.note, tom_4.velocity, 10); //(note, velocity, channel)
      MIDI.sendNoteOff(tom_4.note, 0, 10);
    }

    //HIHAT CC only with FSR sensor, SD3 CC direction, AD2 need reverse check box on.//
    if (hihat.hit == true) 
    {
        MIDI.sendNoteOn(hihat.note, hihat.velocity, 10); //(note of close, velocity, channel)
        MIDI.sendNoteOff(hihat.note, 0, 10);
    }

    pedalAvgValue = (pedalAvgValue * 2 + map(analogRead(HIHAT_PEDAL), PEDAL_MIN, PEDAL_MAX, 0, 127)) / 3;
    if (pedalAvgValue < 0) pedalAvgValue = 0;
    if (pedalAvgValue > 127) pedalAvgValue = 127;
    pedalEighthOfAvgValue = map(pedalAvgValue, 0, 127, 7, 0); // reverse here
 //   if (pedalEighthOfAvgValue == 1 || pedalEighthOfAvgValue == 3) pedalTimeOfChange = millis();
    if (pedalEighthOfAvgValue != pedalOldEighthOfAvgValue)
    {
      MIDI.sendControlChange(4, pedalEighthOfAvgValue * 18, 10);
//      pedalVelocity = map(millis() - pedalTimeOfChange, 0, 200, 127, 1);
      if (pedalOldEighthOfAvgValue < 6 && pedalEighthOfAvgValue == 6) // foot close, hihat.note
      {
        MIDI.sendNoteOn(hihat.note, 127, 10); 
        MIDI.sendNoteOff(hihat.note, 0, 10);
      }
      if (pedalOldEighthOfAvgValue > 2 && pedalEighthOfAvgValue == 2) // foot splash, hihat.noteRim
      {
        MIDI.sendNoteOn(hihat.noteRim, 127, 10); 
        MIDI.sendNoteOff(hihat.noteRim, 0, 10);
      }
      pedalOldEighthOfAvgValue = pedalEighthOfAvgValue;  
    }

    //CYMBAL 1//
    if (cym_1.hit == true)
    {
      MIDI.sendNoteOn(cym_1.note, cym_1.velocity, 10); //(note, velocity, channel)
      MIDI.sendNoteOff(cym_1.note, 0, 10);
    }

    //CYMBAL 2//
    if (cym_2.hit == true)
    {
      MIDI.sendNoteOn(cym_2.note, cym_2.velocity, 10); //(note, velocity, channel)
      MIDI.sendNoteOff(cym_2.note, 0, 10);
    }
  
//    //RIDE//
//    //1.bow
//    if (ride.hit == true)
//    {
//      MIDI.sendNoteOn(ride.note, ride.velocity, 10); //(note, velocity, channel)
//      MIDI.sendNoteOff(ride.note, 0, 10);
//    }
//
//    //2.edge
//    else if (ride.hitRim == true)
//    {
//      MIDI.sendNoteOn(ride.noteRim, ride.velocity, 10); //(note, velocity, channel)
//      MIDI.sendNoteOff(ride.noteRim, 0, 10);
//    }
//
//    //3.cup
//    else if (ride.hitCup == true)
//    {
//      MIDI.sendNoteOn(ride.noteCup, ride.velocity, 10); //(note, velocity, channel)
//      MIDI.sendNoteOff(ride.noteCup, 0, 10);
//    }
//
//    //4.choke
//    if (ride.choke == true)
//    {
//      MIDI.sendPolyPressure(ride.note, 127, 10);
//      MIDI.sendPolyPressure(ride.noteRim, 127, 10);
//      MIDI.sendPolyPressure(ride.noteCup, 127, 10);
//      MIDI.sendPolyPressure(ride.note, 0, 10);
//      MIDI.sendPolyPressure(ride.noteRim, 0, 10);
//      MIDI.sendPolyPressure(ride.noteCup, 0, 10);
//    }

}
