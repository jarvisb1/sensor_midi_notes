#include "MIDIUSB.h"
#include "PitchToNote.h"

//Uncomment the line below to enable the serial port for debugging. Comment it out to disable the serial output. NOTE: MIDI will not output in debugging mode because the serial port interferes with it.
//#define SERIAL_DEBUG (1)

#define SENSOR_VALUE_THRESHOLD (100) //This sensor is actually more of a binary "on/off" reading of 1023 or 0, but this threshold still allows the code below to work.
#define LOOP_SLEEP_MS (100) // Milliseconds to sleep/delay at the end of each loop iteration.

#define DEFAULT_VELOCITY (100)

#define NUM_SENSORS (2)
const byte sensor_pins[NUM_SENSORS] = {0, 1};
const byte pitches[NUM_SENSORS] = {pitchC3, pitchC5}; //Used in case the sensors read different values for the same light level

int velocity = DEFAULT_VELOCITY;

byte channel = 0; //MIDI channel to output on. I'm not sure what happens if you change this.

int last_value[NUM_SENSORS] = {0, 0};
int curr_value[NUM_SENSORS] = {0, 0};

void setup() {
#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  Serial.println("Sensor debug mode (no MIDI will be sent)"); 
#endif
}

void read_values() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    curr_value[i] = analogRead(sensor_pins[i]);
#ifdef SERIAL_DEBUG
    Serial.print("Pin "); Serial.print(sensor_pins[i]); Serial.print(" sensor read: "); Serial.println(curr_value[i]);
#endif
  }
}

bool state_changed(int sensor_num) {
  return (curr_value[sensor_num] != last_value[sensor_num]);
}

void play_midi_note(byte pitch) {
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
  read_values();
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (state_changed(i)) {
      play_midi_note(pitches[i]);
    }
    last_value[i] = curr_value[i];
  }
  delay(LOOP_SLEEP_MS);
}

