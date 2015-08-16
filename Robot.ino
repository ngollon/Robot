#include "I2Cdev.h"
#include "MPU6050.h"

#include "Stepper.h"

Stepper stepper1(1, 0, 2, 3, 4, false);

MPU6050 accelgyro;

int ax, ay, az;
int gx, gy, gz;

int g_offset_x = -327;
int g_offset_y = 160;
int g_offset_z = 17;


unsigned long last_time;
unsigned long current_time;
unsigned long diff_time;


double gyro_factor = 0.98;
double accel_factor = 0.02;
double accel_theta = 0;
double gyro_theta = 0;

double raw_theta;
double theta;
double theta_offset = 1.9;

double target_theta;

double error, error_int, error_diff, last_error;

double kp = -10;
double ki = 0;
double kd = 0;

double speed;	// in steps/second

double dts;
int n;

#define DEBUG
//#define DRY

void setup() {
#ifdef DEBUG
	Serial.begin(9600);
#endif

#if 0

	Fastwire::setup(800, true);
	accelgyro.initialize();
	accelgyro.setXGyroOffset(0);
	accelgyro.setYGyroOffset(0);
	accelgyro.setZGyroOffset(0);
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
	current_time = micros();

	if (last_time == 0 || current_time < last_time)
	{
		last_time = current_time;
		return;
	}

	diff_time = (current_time - last_time) * 1e-6;	// in seconds

	// read raw accel/gyro measurements from device
	accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);


	gx -= g_offset_x;
	gy -= g_offset_y;
	gz -= g_offset_z;


	accel_theta = -atan((double)ax / az) * 57.29578;

	if (accel_factor > 0 && abs(accel_theta) > 80)
		accel_theta = abs(accel_theta) * ((ax < 0) ? -1 : 1);

	gyro_theta = raw_theta + (gy * 7.62939453125e-6) * diff_time;   // 250 / 32768 / 1000000

	raw_theta = gyro_theta * gyro_factor + accel_theta * accel_factor;
	theta = raw_theta - theta_offset;
#ifdef SERIAL
	/*Serial.print("gyro_theta: "); Serial.print(gyro_theta); Serial.print("\t");
	Serial.print("accel_theta: "); Serial.print(accel_theta); Serial.println("\t");	*/
#endif

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
