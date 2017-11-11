#include <Audio.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>
#include <synth_simple_drum.h>

//A simple chord player for teensy + audio which allows
// selection of the most common chords in pop
// (e.g. I, ii, IV, V, vi) with some decorations.

// Hardware setup:
// * 4 potentiometers A0 - A3.  A3 could be logarithmic, as it is for volume.
// * 4 buttons connecting p3-p6 to ground (configures them to use internal pullup resistors)

boolean hasAudioBoard;
// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum1;          //xy=55,64
AudioSynthSimpleDrum     drum2;          //xy=66,101
AudioSynthSimpleDrum     drum3;          //xy=76,139
AudioSynthSimpleDrum     drum4;          //xy=82,173
AudioSynthKarplusStrong  string1;        //xy=95,250
AudioSynthKarplusStrong  string2;        //xy=102,290
AudioSynthKarplusStrong  string3;        //xy=109,328
AudioSynthKarplusStrong  string4;        //xy=115,365
AudioSynthKarplusStrong  string5;        //xy=129,404
AudioSynthKarplusStrong  string6;        //xy=136,441
AudioMixer4              mixerDrums;     //xy=291,139
AudioMixer4              mixerStrings1;  //xy=324,285
AudioMixer4              mixerStrings2;  //xy=331,356
AudioMixer4              mixerFinal;     //xy=534,229
AudioOutputI2S           i2s1;           //xy=679,188
AudioOutputAnalog        dac1;           //xy=713,233
AudioConnection          patchCord1(drum1, 0, mixerDrums, 0);
AudioConnection          patchCord2(drum2, 0, mixerDrums, 1);
AudioConnection          patchCord3(drum3, 0, mixerDrums, 2);
AudioConnection          patchCord4(drum4, 0, mixerDrums, 3);
AudioConnection          patchCord5(string1, 0, mixerStrings1, 0);
AudioConnection          patchCord6(string2, 0, mixerStrings1, 1);
AudioConnection          patchCord7(string3, 0, mixerStrings1, 2);
AudioConnection          patchCord8(string4, 0, mixerStrings1, 3);
AudioConnection          patchCord9(string5, 0, mixerStrings2, 0);
AudioConnection          patchCord10(string6, 0, mixerStrings2, 1);
AudioConnection          patchCord11(mixerDrums, 0, mixerFinal, 0);
AudioConnection          patchCord12(mixerStrings1, 0, mixerFinal, 1);
AudioConnection          patchCord13(mixerStrings2, 0, mixerFinal, 2);
AudioConnection          patchCord14(mixerFinal, 0, i2s1, 0);
AudioConnection          patchCord15(mixerFinal, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=638,468
AudioConnection          patchCord16(mixerFinal, dac1);
// GUItool: end automatically generated code

AudioSynthKarplusStrong *strings [] = { &string1, &string2, &string3, &string4, &string5, &string6 };

int major7[6] = {48, 59, 64, 67, 71, 76};
int minor7[6] = {48, 58, 63, 67, 70, 72};
int dominant7[6] = {48, 58, 64, 67, 70, 76};
int major[6] = {48, 55, 60, 64, 67, 76};
int major6[6] = {48, 55, 60, 64, 67, 69};
int sus4[6] = {48, 55, 60, 65, 67, 76};
int sus2[6] = {48, 55, 62, 64, 67, 76};
int dominant[6] = {48, 55, 60, 64, 67, 76};
int diminished[6] = {48, 54, 60, 63, 66, 72};
int minor[6] = {48, 55, 60, 63, 67, 75};


//int chords[7] = { &major7, &minor7, &minor7, &major7, &dominant7, &minor7, &minor7 };
int offsets[7] = {0, 2, 4, 5, 7, 9, 11};

bool buttonStates[4] = {false, false, false, false};

static uint32_t nextStrum;
static uint32_t nextVolumeUpdate;
int gKeyOffset = 0; //at higher scope to allow periodic logging

//Note: I found that on the teensy 3.2, at 96MHz, notes lower than the 40th one do not
// sound with the correct pitch, at least with the Karplus Strong string synthesis.
const float tune_frequencies2_PGM[128] =
{
  8.1758,    8.6620,    9.1770,    9.7227,    10.3009,    10.9134,    11.5623,    12.2499,
  12.9783,   13.7500,   14.5676,   15.4339,   16.3516,    17.3239,    18.3540,    19.4454,
  20.6017,   21.8268,   23.1247,   24.4997,   25.9565,    27.5000,    29.1352,    30.8677,
  32.7032,   34.6478,   36.7081,   38.8909,   41.2034,    43.6535,    46.2493,    48.9994,
  51.9131,   55.0000,   58.2705,   61.7354,   65.4064,    69.2957,    73.4162,    77.7817,
  82.4069,   87.3071,   92.4986,   97.9989,   103.8262,   110.0000,   116.5409,   123.4708,
  130.8128,  138.5913,  146.8324,  155.5635,  164.8138,   174.6141,   184.9972,   195.9977,
  207.6523,  220.0000,  233.0819,  246.9417,  261.6256,   277.1826,   293.6648,   311.1270,
  329.6276,  349.2282,  369.9944,  391.9954,  415.3047,   440.0000,   466.1638,   493.8833,
  523.2511,  554.3653,  587.3295,  622.2540,  659.2551,   698.4565,   739.9888,   783.9909,
  830.6094,  880.0000,  932.3275,  987.7666,  1046.5023,  1108.7305,  1174.6591,  1244.5079,
  1318.5102, 1396.9129, 1479.9777, 1567.9817, 1661.2188,  1760.0000,  1864.6550,  1975.5332,
  2093.0045, 2217.4610, 2349.3181, 2489.0159, 2637.0205,  2793.8259,  2959.9554,  3135.9635,
  3322.4376, 3520.0000, 3729.3101, 3951.0664, 4186.0090,  4434.9221,  4698.6363,  4978.0317,
  5274.0409, 5587.6517, 5919.9108, 6271.9270, 6644.8752,  7040.0000,  7458.6202,  7902.1328,
  8372.0181, 8869.8442, 9397.2726, 9956.0635, 10548.0818, 11175.3034, 11839.8215, 12543.8540
};

int readKeyOffsetFromAnalog(int pin) {
  int keyAnalog = analogRead(pin);
  int keyOffset = 0;

  //A key-offset lower than -1 will cause IV chord to be too low to be sounded, with current voicings.
  if (keyAnalog > 900) {
    keyOffset = 5;
  } else if (keyAnalog > 800) {
    keyOffset = 4;
  } else if (keyAnalog > 700) {
    keyOffset = 3;
  } else if (keyAnalog > 600) {
    keyOffset = 2;
  } else if (keyAnalog > 500) {
    keyOffset = 1;
  } else if (keyAnalog > 400) {
    keyOffset = 0;
  } else {
    keyOffset = -1;
  }
  return keyOffset;
}

void strumOff() {
  for (int i = 0; i < 6; i++) {
    strings[i] -> noteOff(random(0, 100) / 100.0);
  }
}
void strum() {
  int* chord = major;
  int root = 0;
  gKeyOffset = readKeyOffsetFromAnalog(1);

  if (buttonStates[0] && buttonStates[1]) {
    root = 0;
    chord = sus2;
  } else if (buttonStates[0] && buttonStates[2] && buttonStates[3]) {
    root = 0;
    chord = dominant7;
  } else if (buttonStates[0] && buttonStates[3]) {
    root = 0;
    chord = major7;
  } else if (buttonStates[0] && buttonStates[2]) {
    root = 0;
    chord = sus4;
  } else if (buttonStates[0]) {
    root = 0;
    chord = major;
  } else if (buttonStates[1] && buttonStates[2]) {
    chord = minor7;
    root = -10;
  } else if (buttonStates[1] && buttonStates[3]) {
    chord = minor;
    root = -7;
  } else if (buttonStates[1]) {
    chord = major;
    root = -7;
  } else if (buttonStates[2] && buttonStates[3]) {
    chord = diminished;
    root = -4;
  } else if (buttonStates[2]) {
    chord = dominant;
    root = -5;
  } else if (buttonStates[3]) {
    chord = minor;
    root = -3;
  } else {
    chord = NULL;
    root = 0;
  }

  int delayAnalog = analogRead(0);
  int delayAmt = map(delayAnalog, 0, 1023, 0, 50);
  if (chord != NULL) {
    for (int i = 0; i < 6; i++) {
      int note = gKeyOffset + root + chord[i];
      if (note >= 40) {
        strings[i] -> noteOn(tune_frequencies2_PGM[note], random(20, 40) / 100.0);
      } else {
        //red-flag that a note is being attempted that can't be sounded?
      }
      delay(delayAmt);
    }
  }
}


void setup() {
  hasAudioBoard = true;
  // put your setup code here, to run once:

  Serial.begin(115200);
  
  if (!hasAudioBoard) {
    analogWriteResolution(12); //only needed if we're using the built-in DAC for audio out
  }
  
  for (int i = 3; i < 7; i++) {
    pinMode(i, INPUT_PULLUP);
  }

  // audio library init
  AudioMemory(20);

  nextStrum = millis() + 1000;
  nextVolumeUpdate = millis() + 1000;

  AudioNoInterrupts();

  drum1.frequency(60);
  drum1.length(1500);
  drum1.secondMix(0.0);
  drum1.pitchMod(0.55);

  drum2.frequency(60);
  drum2.length(300);
  drum2.secondMix(0.0);
  drum2.pitchMod(1.0);

  drum3.frequency(60);
  drum3.length(1500);
  drum3.secondMix(0.0);
  drum3.pitchMod(0.60);

  drum4.frequency(600);
  drum4.length(150);
  drum4.secondMix(0.0);
  drum4.pitchMod(0.0);

  
   if (hasAudioBoard) {
     sgtl5000_1.enable();
     sgtl5000_1.volume(0.5);      
   } else {
     mixerFinal.gain(1, 0.3);
     mixerFinal.gain(2, 0.3);
   }
  AudioInterrupts();
}

void showButtonStates() {
  Serial.print("buttons ");
  Serial.print(buttonStates[0]);
  Serial.print(buttonStates[1]);
  Serial.print(buttonStates[2]);
  Serial.println(buttonStates[3]);
}
void loop() {
  // put your main code here, to run repeatedly:

  static uint32_t num = 0;
  unsigned long ms = millis();
  if (ms >= nextVolumeUpdate)
  {
    float volume = map(analogRead(A3), 0, 1023, 0, 60) / 100.0;
    if (hasAudioBoard) {
      sgtl5000_1.volume(volume);
    } else {
      mixerFinal.gain(1, volume/2);
      mixerFinal.gain(2, volume/2);
    }
    nextVolumeUpdate = ms + 50;
  }
  if (ms >= nextStrum)
  {
    for (int bn = 0; bn < 4; bn++) {
      buttonStates[bn] = digitalRead(3 + bn) == LOW;
    }
    //showButtonStates();
    int waitTime = map(analogRead(2), 0, 1023, 300, 700);
    nextStrum = ms + waitTime;
    switch (num % 4)
    {
      case 0:
        //drum1.noteOn();
        //strumOff();
        break;
      case 1:
        //drum2.noteOn();
        strum();
        break;
      case 2:
        //drum1.noteOn();
        //strumOff();
        break;
      case 3:
        //drum2.frequency(random(54, 65));
        //drum2.noteOn();
        strum();
        break;
    }
    num++;

    Serial.println("Teensy Chord Player");
    Serial.print("Key ");
    Serial.println(gKeyOffset);
    Serial.println("Diagnostics (Audio CPU and Audio Mem max usages: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print(" ");
    Serial.println(AudioMemoryUsageMax());
    AudioProcessorUsageMaxReset();
  }

}

