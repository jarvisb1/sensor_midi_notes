#include "MIDIUSB.h"
#include "PitchToNote.h"

//Uncomment the line below to enable the serial port for debugging. Comment it out to disable the serial output. NOTE: MIDI will not output in debugging mode because the serial port interferes with it.
//#define SERIAL_DEBUG (1)

//Comment this out if you want the MIDI note offs to be sent. Uncomment it if you do NOT want note offs to be sent.
#define NO_NOTE_OFFS (1)

#define LOOP_SLEEP_MS (100) // Milliseconds to sleep/delay at the end of each loop iteration.

#define DEFAULT_VELOCITY (100)

#define NUM_SENSORS (2)
const byte sensor_pins[NUM_SENSORS] = {0, 1};
const byte pitches[NUM_SENSORS][2] = {{pitchA3, pitchD3}, {pitchA5, pitchD5}}; //Note to be played for each sensor/direction combo. Inner array is hard-coded to 2 for rising vs falling edge

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

#ifndef NO_NOTE_OFFS //Prevents this subsection from running if NO_NOTE_OFFS is defined
  delay(50);

  midiEventPacket_t noteOff = {0x08, (byte)(0x80 | channel), pitch, (byte)velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
#endif

#else
  Serial.print("MIDI packet velocity: "); Serial.print((byte)velocity); Serial.print(" , pitch: "); Serial.println((byte)pitch);
#endif
}

void loop() {
  read_values();
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (state_changed(i)) {
      int dir = 0;
      if ((curr_value[i] - last_value[i]) < 0)
        dir = 1;

      play_midi_note(pitches[i][dir]);
    }
    last_value[i] = curr_value[i];
  }
  delay(LOOP_SLEEP_MS);
}

