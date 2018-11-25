#include "MIDIUSB.h"
#include "PitchToNote.h"

//Uncomment the line below to enable the serial port for debugging. Comment it out to disable the serial output. NOTE: MIDI will not output in debugging mode because the serial port interferes with it.
//#define SERIAL_DEBUG (1)

#define SENSOR_IN_PIN (0)
#define SENSOR_VALUE_THRESHOLD (100) //This sensor is actually more of a binary "on/off" reading of 1023 or 0, but this threshold still allows the code below to work.
#define LOOP_SLEEP_MS (100) // Milliseconds to sleep/delay at the end of each loop iteration.

#define DEFAULT_PITCH (pitchC3)
#define DEFAULT_VELOCITY (100)

int velocity = DEFAULT_VELOCITY;
byte pitch = DEFAULT_PITCH;

byte channel = 0; //MIDI channel to output on. I'm not sure what happens if you change this.

int last_value = 0;
int curr_value = 0;

void setup() {
#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  Serial.println("Sensor debug mode (no MIDI will be sent)"); 
#endif
}

void read_value() {
  curr_value = analogRead(SENSOR_IN_PIN);

#ifdef SERIAL_DEBUG
  Serial.print("Sensor read: "); Serial.println(curr_value);
#endif
}

bool state_changed() {
  return (curr_value != last_value);
}

void play_midi_note() {
#ifndef SERIAL_DEBUG //Prevents this following from being executed if in serial debug mode
  midiEventPacket_t noteOn = {0x09, (byte)(0x90 | channel), pitch, (byte)velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();

  delay(50);

  midiEventPacket_t noteOff = {0x08, (byte)(0x80 | channel), pitch, (byte)velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();

#else
  Serial.print("MIDI packet velocity: "); Serial.print((byte)velocity); Serial.print(" , pitch: "); Serial.println((byte)pitch);
#endif
}

void loop() {
  read_value();
  if (state_changed()) {
    play_midi_note();
  }
  last_value = curr_value;
  delay(LOOP_SLEEP_MS);
}

