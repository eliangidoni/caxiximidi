#define MIDI_CHANNEL 1

#define NotaAdelante 0
#define NotaAtras 1
#define NotaAdelanteRotada 2
#define NotaAtrasRotada 3

// SENSOR_BEGIN
#define SENSOR_ACCEL_X 0
#define SENSOR_ACCEL_Y 1
#define SENSOR_ACCEL_Z 2
#define SENSOR_GYRO_Y 3
#define SENSOR_GYRO_X 4
#define SENSOR_GYRO_Z 5

#define BUFFER_SIZE 8

#define RESOLUTION 1

// SmoothFilter Samples
#define filterSamples 11

#define SENSOR_NOTE_ADELANTE 43
#define SENSOR_NOTE_ATRAS 45
#define SENSOR_NOTE_ADELANTE_ROTADA 47
#define SENSOR_NOTE_ATRAS_ROTADA 49

#define NOTE_RELEASE_ADELANTE 100
#define NOTE_RELEASE_ATRAS -100
#define NOTE_RELEASE_ADELANTE_ROTADA 100
#define NOTE_RELEASE_ATRAS_ROTADA -100

#define NOTE_THRESHOLD_ADELANTE 600
#define NOTE_THRESHOLD_ATRAS -400
#define NOTE_THRESHOLD_ADELANTE_ROTADA 550
#define NOTE_THRESHOLD_ATRAS_ROTADA -6000

#define HITS_THRESOLD 30

////////PIEZO///////
// SENSOR_BEGIN
#define A_NotaAdelante 0
#define A_NotaAtras 1
#define A_PIEZO_ADELANTE 0
#define A_PIEZO_ATRAS 1
// SENSOR_END
#define SMOOTH_SAMPLES 15
#define PIEZO_SMOOTH_SAMPLES 10