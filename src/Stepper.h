#pragma once

#define MAX_MICROSTEPPING_MODE 4

struct Pin
{
    volatile unsigned char* ddr;
    volatile unsigned char* port;
    unsigned char index;
};

class Stepper
{
public:
    Stepper(unsigned long minUsPerStep,
            Pin dirPin,
            Pin stepPin,
            Pin ms1Pin,
            Pin ms2Pin,
            Pin ms3Pin,
            bool reverse);

    void    setMaxSpeed(float speed);
    void    setAcceleration(float acceleration);
    void    setSpeed(float speed);
    float   getCurrentSpeed();

    void    run();
    void    accelerate();

private:
    Pin        _dirPin;
    Pin        _stepPin;
    Pin        _ms1Pin;
    Pin        _ms2Pin;
    Pin        _ms3Pin;
    bool       _reverse;         // Changes wich direction is considered forward
    float      _desiredSpeed;    // The desired speed in steps per second
    float      _currentSpeed;    // The current motos speed in steps per second
    float      _maxSpeed;        // The maximum permitted speed in steps per second. Must be > 0.
    float      _acceleration;    // Acceleration in steps per second squared

	unsigned char  _desiredMicrosteppingMode;	 // The microstepping mode the motor SHOULD run at.
	unsigned char  _currentMicrosteppingMode;	 // The microstepping mode the motor runs at.
	unsigned char  _stepCounter;				 // Counts microsteps done.

    unsigned long  _minUsPerStep;    // Given the current max speed, how fast can steps be done?
    unsigned long  _lastUpdateTime;  // The last time updateSpeed was called
    unsigned long  _lastStepTime;    // The last step time in microseconds
    unsigned long  _usPerFullStep;   // The current interval between steps in microseconds. 0 means stopped.

    void setDirection(bool forward);
    void updateMicrosteppingMode();
    void setMicrosteppingMode(int microsteppingMode);
};
