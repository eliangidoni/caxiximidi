#include <FreeSixIMU.h>
#include <FIMU_ADXL345.h>
#include <FIMU_ITG3200.h>

/*
CAXIXI XBee
*/
#include "CaxixiConfig.h"

#include "CommunicationUtils.h"
#include "FreeSixIMU.h"
#include <Wire.h>

#include "CxCircularBuffer.h"

CxCircularBuffer accelXBuffer(BUFFER_SIZE);
CxCircularBuffer accelYBuffer(BUFFER_SIZE);
boolean bufferReady = false;
/*
int samples = 0;
int resolution = RESOLUTION;
int iteration = 0;
*/
int	accelXSmooth[filterSamples];
int accelYSmooth[filterSamples];
int smoothAccelX;
int smoothAccelY;

int SensorRead[6] = {0, 0, 0, 0, 0, 0};

int NoteRelease[3] = {
	NOTE_RELEASE_FORWARD,
	NOTE_RELEASE_BACKWARD,
	NOTE_RELEASE_HIT
};

int noteThresholdHit = NOTE_THRESHOLD_HIT;
int canHitDefinition = 1;

int noteOn = NOTE_OFF;

float v[6];
float angles[3];
FreeSixIMU my3IMU = FreeSixIMU();

boolean isUpThreshold, isDownThreshold, isUpThresholdRotated, isDownThresholdRotated;
boolean canHit, canHitRotated;

int currentAccelX, currentAccelY;

int slopeStill;
int accelXForce;
int state = STATE_STILL;
int prevState;

void setup() {
	Serial.begin(9600);
	Wire.begin();
	accelXBuffer.clear();
	accelYBuffer.clear();
	delay(5);
	my3IMU.init();
	my3IMU.acc.setFullResBit(true);
	my3IMU.acc.setRangeSetting(16);
	delay(5);
}

int initialMillis;

void loop() {
	//initialMillis = millis();
	my3IMU.getValues(v);
	SensorRead[SENSOR_ACCEL_X] = (int)v[0];
	SensorRead[SENSOR_ACCEL_Y] = (int)v[1];
	SensorRead[SENSOR_ACCEL_Z] = (int)v[2];
	setCircularBuffer();
	if(bufferReady || isBufferReady()){
		currentAccelX = accelXBuffer.getPreviousElement(1);
		currentAccelY = accelYBuffer.getPreviousElement(1);
		setSlopeStill();
		setAccelXForce();
		prevState = state;
		setState();
		switch (noteOn) {
			case NOTE_FORWARD:
			if(noteReleaseForward()){
				SendNoteOff(CAXIXI_RIGHT_FORWARD_NOTEOFF);
				noteOn = NOTE_OFF;
			}
			break;
			case NOTE_BACKWARD:
			if(noteReleaseBackward()){
				SendNoteOff(CAXIXI_RIGHT_BACKWARD_NOTEOFF);
				noteOn = NOTE_OFF;
			}
			break;
			case NOTE_HIT:
			if(noteReleaseHit()){
				SendNoteOff(CAXIXI_RIGHT_HIT_NOTEOFF);
				noteOn = NOTE_OFF;
			}
			break;
			default:
			break;
		}
		if(noteOn == NOTE_OFF && state == STATE_FORWARD && prevState == STATE_BACKWARD){
			noteOn = NOTE_FORWARD;
			SendNoteOn(CAXIXI_RIGHT_FORWARD_NOTEON);
		}
		
		if(noteOn == NOTE_OFF && state == STATE_BACKWARD && prevState == STATE_FORWARD){
			noteOn = NOTE_BACKWARD;
			SendNoteOn(CAXIXI_RIGHT_BACKWARD_NOTEON);
		}
		
		if(noteOn == NOTE_OFF && currentAccelY > noteThresholdHit){
			noteOn = NOTE_HIT;
			SendNoteOn(CAXIXI_RIGHT_HIT_NOTEON);
		}
	}
	delay(2);
	//int diffMillis = millis() - initialMillis;
	//Serial.print(diffMillis);
	//Serial.println();
}

void SendNoteOn(String note)
{
	Serial.println(note);
}

void SendNoteOff(String note)
{
	Serial.println(note);
}

void setCircularBuffer(){
	smoothAccelX = digitalSmooth(SensorRead[SENSOR_ACCEL_X], accelXSmooth);
	smoothAccelY = digitalSmooth(SensorRead[SENSOR_ACCEL_Y], accelYSmooth);
	accelXBuffer.addValue(smoothAccelX);
	accelYBuffer.addValue(smoothAccelY);
}

boolean isBufferReady(){
	if(accelXBuffer.getCount() < BUFFER_SIZE
	|| accelYBuffer.getCount() < BUFFER_SIZE){
		return false;
	}else{
		bufferReady = true;
		return true;
	}
}

void setSlopeStill()
{
	if(abs(currentAccelX - accelXBuffer.getPreviousElement(8)) < SLOPE_STILL_X_RANGE
	&& abs(currentAccelY - accelYBuffer.getPreviousElement(8)) < SLOPE_STILL_Y_RANGE){
		slopeStill = SLOPE_STILL_STATUS; 
	}else{
		slopeStill = SLOPE_MOVING_STATUS;
	}
}

void setAccelXForce()
{
	if(currentAccelX > FORCE_THRESHOLD_FORWARD || currentAccelY > NOTE_THRESHOLD_HIT){
		accelXForce = FORCE_FORWARD;
	}
	if(currentAccelX < FORCE_THRESHOLD_BACKWARD){
		accelXForce = FORCE_BACKWARD;
	}
	if(currentAccelX < FORCE_STILL_RANGE_FORWARD && currentAccelX > FORCE_STILL_RANGE_BACKWARD){
		accelXForce = FORCE_STILL;
	}
}

void setState()
{
	if(slopeStill == SLOPE_STILL_STATUS && accelXForce == FORCE_STILL){
		state = STATE_STILL;
	}else if(slopeStill == SLOPE_MOVING_STATUS && accelXForce == FORCE_FORWARD){
		state = STATE_FORWARD;
	}else if(slopeStill == SLOPE_MOVING_STATUS && accelXForce == FORCE_BACKWARD){
		state = STATE_BACKWARD;
	}else{
		state = prevState;
	}
}

boolean noteReleaseForward()
{
	if(currentAccelX < NoteRelease[NOTE_FORWARD]){
		return true;
	}else{
		return false;
	}
}

boolean noteReleaseBackward()
{
	if(currentAccelX > NoteRelease[NOTE_BACKWARD]){
		return true;
	}else{
		return false;
	}
}

boolean noteReleaseHit()
{
	if(currentAccelY < NoteRelease[NOTE_HIT]){
		return true;
	}else{
		return false;
	}
}

int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
int j, k, temp, top, bottom;
long total;
static int i;
// static int raw[filterSamples];
static int sorted[filterSamples];
boolean done;

i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

// Serial.print("raw = ");

for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
	sorted[j] = sensSmoothArray[j];
}

done = 0;                // flag to know when we're done sorting              
while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
	done = 1;
	for (j = 0; j < (filterSamples - 1); j++){
		if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
			temp = sorted[j + 1];
			sorted [j+1] =  sorted[j] ;
			sorted [j] = temp;
			done = 0;
		}
	}
}

/*
for (j = 0; j < (filterSamples); j++){    // print the array to debug
Serial.print(sorted[j]); 
Serial.print("   "); 
}
Serial.println();
*/

// throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
bottom = max(((filterSamples * 15)  / 100), 1); 
top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
k = 0;
total = 0;
for ( j = bottom; j< top; j++){
	total += sorted[j];  // total remaining indices
	k++; 
	// Serial.print(sorted[j]); 
	// Serial.print("   "); 
}

//  Serial.println();
//  Serial.print("average = ");
//  Serial.println(total/k);
return total / k;    // divide by number of samples
}