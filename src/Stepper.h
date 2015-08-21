#pragma once

#define MAX_MICROSTEPPING_MODE 4

class Stepper
{
public:
    Stepper(int dirPin, int stepPin, int ms1Pin, int ms2Pin, int ms3Pin, bool reverse);

    void    setMaxSpeed(float speed);
    void    setAcceleration(float acceleration);
    void    setSpeed(float speed);
    float   getCurrentSpeed();

    void    run();
    void    accelerate();

private:
    int        _dirPin;
    int        _stepPin;
    int        _ms1Pin;
    int        _ms2Pin;
    int        _ms3Pin;
    bool       _reverse;         // Changes wich direction is considered forward
    float      _desiredSpeed;    // The desired speed in steps per second
    float      _currentSpeed;    // The current motos speed in steps per second
    float      _maxSpeed;        // The maximum permitted speed in steps per second. Must be > 0.
    float      _acceleration;    // Acceleration in steps per second squared

	int _desiredMicrosteppingMode;	 // The microstepping mode the motor SHOULD run at.
	int _currentMicrosteppingMode;	 // The microstepping mode the motor runs at. 
	int _stepCounter;				 // Counts microsteps done. 

    unsigned long  _minUsPerStep;    // Given the current max speed, how fast can steps be done?
    unsigned long  _lastUpdateTime;  // The last time updateSpeed was called
    unsigned long  _lastStepTime;    // The last step time in microseconds
    unsigned long  _usPerFullStep;   // The current interval between steps in microseconds. 0 means stopped.    

    void setDirection(bool forward);
    void updateMicrosteppingMode();
    void setMicrosteppingMode(int microsteppingMode);
};
