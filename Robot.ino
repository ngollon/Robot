#include "Stepper.h"
#include "AngleSensor.h"
#include "PID.h"

Stepper stepper1(0, 1, 2, 3, 4, false);
Stepper stepper2(6, 5, 7, 8, 9, true);
//AngleSensor angleSensor();
PID pid(-10, 0, 0);

#define ANGLE_OFFSET 1.9
#define MAX_SPEED 1600.0
#define DEBUG

void setup() {
    Serial.begin(9600);
    delay(5000);

    // Initialize steppers
    stepper1.setMaxSpeed(MAX_SPEED);
    stepper1.setAcceleration(600.0);
    stepper1.setSpeed(MAX_SPEED);

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

#if 0
void update_speed()
{
    angleSensor.update();

    float currentAngle = angleSensor.getAngle();
    float speedCorrection = pid.calculate(ANGLE_OFFSET, currentAngle)

    // The wheels should at least compensate for the current rotation
    speed = angleSensor.getRate() * 4.239e-3 + speedCorrection;

    // We have fallen, stop struggeling
    float absoluteAngle = abs(currentAngle);
    if (absoluteAngle > 45)
        speed = 0;

    stepper1.setSpeed(speed);
    stepper2.setSpeed(speed);
}
#endif

void loop() {
    // update_speed();

    stepper1.accelerate();
    if(stepper1.getCurrentSpeed() > MAX_SPEED - 20)
        stepper1.setSpeed(-MAX_SPEED);
    if(stepper1.getCurrentSpeed() < -MAX_SPEED + 20)
        stepper1.setSpeed(MAX_SPEED);
}
