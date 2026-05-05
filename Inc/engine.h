#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

typedef enum {
  NOTE_REST = 0,
  NOTE_C0,
  NOTE_CS0,
  NOTE_D0,
  NOTE_DS0,
  NOTE_E0,
  NOTE_F0,
  NOTE_FS0,
  NOTE_G0,
  NOTE_GS0,
  NOTE_A0,
  NOTE_AS0,
  NOTE_B0,
  NOTE_C1,
  NOTE_CS1,
  NOTE_D1,
  NOTE_DS1,
  NOTE_E1,
  NOTE_F1,
  NOTE_FS1,
  NOTE_G1,
  NOTE_GS1,
  NOTE_A1,
  NOTE_AS1,
  NOTE_B1,
  NOTE_C2,
  NOTE_CS2,
  NOTE_D2,
  NOTE_DS2,
  NOTE_E2,
  NOTE_F2,
  NOTE_FS2,
  NOTE_G2,
  NOTE_GS2,
  NOTE_A2,
  NOTE_AS2,
  NOTE_B2,
  NOTE_C3,
  NOTE_CS3,
  NOTE_D3,
  NOTE_DS3,
  NOTE_E3,
  NOTE_F3,
  NOTE_FS3,
  NOTE_G3,
  NOTE_GS3,
  NOTE_A3,
  NOTE_AS3,
  NOTE_B3,
  NOTE_C4,
  NOTE_CS4,
  NOTE_D4,
  NOTE_DS4,
  NOTE_E4,
  NOTE_F4,
  NOTE_FS4,
  NOTE_G4,
  NOTE_GS4,
  NOTE_A4,
  NOTE_AS4,
  NOTE_B4,
  NOTE_C5,
  NOTE_CS5,
  NOTE_D5,
  NOTE_DS5,
  NOTE_E5,
  NOTE_F5,
  NOTE_FS5,
  NOTE_G5,
  NOTE_GS5,
  NOTE_A5,
  NOTE_AS5,
  NOTE_B5,
  NOTE_C6,
  NOTE_CS6,
  NOTE_D6,
  NOTE_DS6,
  NOTE_E6,
  NOTE_F6,
  NOTE_FS6,
  NOTE_G6,
  NOTE_GS6,
  NOTE_A6,
  NOTE_AS6,
  NOTE_B6,
  NOTE_C7,
  NOTE_CS7,
  NOTE_D7,
  NOTE_DS7,
  NOTE_E7,
  NOTE_F7,
  NOTE_FS7,
  NOTE_G7,
  NOTE_GS7,
  NOTE_A7,
  NOTE_AS7,
  NOTE_B7,
  NOTE_COUNT
} MusicNote;

static const int NOTE_FREQUENCIES[NOTE_COUNT] = {
    16,   17,   18,   19,   21,   22,
    23,   24,   26,   28,   29,   31, // C0-B0
    33,   35,   37,   39,   41,   44,
    46,   49,   52,   55,   58,   62, // C1-B1
    65,   69,   73,   78,   82,   87,
    92,   98,   104,  110,  117,  123, // C2-B2
    131,  139,  147,  156,  165,  175,
    185,  196,  208,  220,  233,  247, // C3-B3
    262,  277,  294,  311,  330,  349,
    370,  392,  415,  440,  466,  494, // C4-B4
    523,  554,  587,  622,  659,  698,
    740,  784,  831,  880,  932,  988, // C5-B5
    1047, 1109, 1175, 1245, 1319, 1397,
    1480, 1568, 1661, 1760, 1865, 1976, // C6-B6
    2093, 2217, 2349, 2489, 2637, 2794,
    2960, 3136, 3322, 3520, 3729, 3951, // C7-B7
};

typedef enum {
  WAVE_SQUARE,
  WAVE_SAW,
  WAVE_TRIANGLE,
  WAVE_SINE,
} WaveType;

typedef enum {
  FILTER_LOW_PASS,
} FilterType;

typedef struct filter {
  float filter_state;
  float cutoff;
  float filter_p;
  FilterType filter;
} Filter;

typedef struct engine {
  int num_steps;
  int steps[32];
  int bpm;
  int sample_idx;
  Filter filter;
  WaveType wave;

} Engine;

#define SAMPLE_RATE 22050
#define AMPLITUDE 8000
Engine engine_init(void);
int16_t engine_next_sample(Engine *e);
void engine_set_bpm(Engine *e, int bpm);
void engine_set_steps(Engine *e, int maxSteps, MusicNote *n);

#endif
