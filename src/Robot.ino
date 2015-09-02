#include "Stepper.h"
#include "AngleSensor.h"
#include "PID.h"

// This is for Leonardo
Pin pin0 = { &DDRD, &PORTD, 2 };
Pin pin1 = { &DDRD, &PORTD, 3 };
Pin pin2 = { &DDRD, &PORTD, 1 };
Pin pin3 = { &DDRD, &PORTD, 0 };
Pin pin4 = { &DDRD, &PORTD, 4 };
Pin pin5 = { &DDRC, &PORTC, 6 };
Pin pin6 = { &DDRD, &PORTD, 7 };
Pin pin7 = { &DDRE, &PORTE, 6 };
Pin pin8 = { &DDRB, &PORTB, 4 };
Pin pin9 = { &DDRB, &PORTB, 5 };

Stepper stepper1(600, pin1, pin0, pin4, pin3, pin2, false);
Stepper stepper2(600, pin6, pin5, pin9, pin8, pin7, true);

//AngleSensor angleSensor();
PID pid(-10, 0, 0);

#define ANGLE_OFFSET 1.9
#define MAX_SPEED 1200.0
#define DEBUG

void setup() {
    Serial.begin(9600);
    delay(5000);

    // Initialize steppers
    pinMode(0, OUTPUT);
    pinMode(5, OUTPUT);

    stepper1.setMaxSpeed(MAX_SPEED);
    stepper1.setAcceleration(3200.0);
    stepper1.setSpeed(MAX_SPEED);

    // Initialize timer
    // With 16Mhz Clock speed, prescaling 8 and OSC1A 300
    // the ISR will be called every 150 µs
    cli();
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11);
    OCR1A = 300;
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
