#include "I2Cdev.h"
#include "MPU6050.h"

#include "Stepper.h"

Stepper stepper1(1, 0, 2, 3, 4, false);

double theta_offset = 1.9;
double target_theta;

double error, error_int, error_diff, last_error;
double speed;	// in steps/second

double dts;
int n;

#define DEBUG
//#define DRY

void setup() {
#ifdef DEBUG
	Serial.begin(9600);
#endif
        delay(5000);
	// Initialize steppers
	stepper1.setMaxSpeed(400.0);
	stepper1.setAcceleration(10.0);
	stepper1.setSpeed(400);

	// Initialize timer
	// With 16Mhz Clock speed, prescaling 8 and OSC1A 1024
	// the ISR will be called 2048 times a second. More than enough for the 400 max speed.
	cli();
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS11);
	OCR1A = 1024;
	TIMSK1 = (1 << OCIE1A);
	sei();
}

ISR(TIMER1_COMPA_vect)
{
	stepper1.run();
}

int loopCounter = 0;

#if 0
void update_speed(){

	error = target_theta - theta;
	error_int += error * diff_time;
	error_diff = (last_error - error) / diff_time;

#ifdef SERIAL
	//Serial.print("error: "); Serial.print(error); Serial.print("\t");
	//Serial.print("error_int: "); Serial.print(error_int); Serial.print("\t");
	//Serial.print("error_diff: "); Serial.print(error_diff); Serial.println("\t");
#endif
	// gy is the current rotation in degrees per second
	// The wheels should at least compensate for that
	speed = gy * 4.239e-3;
	speed += kp * error + ki * error_int + kd * error_diff;

	float absTheta = abs(theta);
	if (absTheta > 45)
		speed = 0;

	last_error = error;
	last_time = current_time;


#ifndef DRY
	stepper1.setSpeed(speed);
	stepper2.setSpeed(speed);
#endif


#ifdef SERIAL
	Serial.print("theta: "); Serial.print(theta); Serial.print("\t");
	Serial.print("speed: "); Serial.println(speed);
#endif
}
#endif

void loop() {
	stepper1.accelerate();
	if(stepper1.getCurrentSpeed() > 380)
		stepper1.setSpeed(0);
	if(stepper1.getCurrentSpeed() < 2)
		stepper1.setSpeed(400);
}
