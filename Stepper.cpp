#include "Stepper.h"
#include "Arduino.h"

Stepper::Stepper(int dirPin, int stepPin, int ms1Pin, int ms2Pin, int ms3Pin, bool reverse)
{
	// Set some consistent default values
	_maxSpeed = 1.0;
	_minUsPerStep = 1e6;
	_acceleration = 1.0;
	_desiredSpeed = 0.0;
	_currentSpeed = 0.0;
	_lastStepTime = 0;
	_usPerFullStep = 0;
	_stepCounter = 0;
	_currentMicrosteppingMode = 1;

	// Store pin layout
	_reverse = reverse;
	_dirPin = dirPin;
	_stepPin = stepPin;
	_ms1Pin = ms1Pin;
	_ms2Pin = ms2Pin;
	_ms3Pin = ms3Pin;

	// Configure pins
	pinMode(_dirPin, OUTPUT);
	pinMode(_stepPin, OUTPUT);
	pinMode(_ms1Pin, OUTPUT);
	pinMode(_ms2Pin, OUTPUT);
	pinMode(_ms3Pin, OUTPUT);
}

void Stepper::setMaxSpeed(float speed)
{
	_maxSpeed = speed;
	_minUsPerStep = (unsigned long)(1e6 / _maxSpeed);
	if (_desiredSpeed > _maxSpeed)
		_desiredSpeed = _maxSpeed;
	if (_desiredSpeed < -_maxSpeed)
		_desiredSpeed = -_maxSpeed;
	updateMicrosteppingMode();
}

void Stepper::setAcceleration(float acceleration)
{
	_acceleration = acceleration;
}

void Stepper::setSpeed(float speed)
{
	_desiredSpeed = speed;
	if (_desiredSpeed > _maxSpeed)
		_desiredSpeed = _maxSpeed;
	if (_desiredSpeed < -_maxSpeed)
		_desiredSpeed = -_maxSpeed;
}

float Stepper::getCurrentSpeed()
{
	return _currentSpeed;
}

void Stepper::run()
{
	unsigned long currentTime = micros();

	if (_lastStepTime == 0 || _desiredMicrosteppingMode == -1)
	{
		_lastStepTime = currentTime;
	}
	else		
	{
		// First, check what the fastest possible microstep is with the current state of stepCounter
		// We need to be sure that e.g. a quarter step is only done, if the stepCounter is divisible by 4.

		// We try to be fast here and avoid division since this is run in an ISR.
		// See https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightParallel
		int nextStepSize = 1 << MAX_MICROSTEPPING_MODE;
		if(_stepCounter > 0)
			nextStepSize = _stepCounter & -_stepCounter;

		int desiredStepSize = 1 << (MAX_MICROSTEPPING_MODE - _desiredMicrosteppingMode);

		if(nextStepSize > desiredStepSize)
			nextStepSize = desiredStepSize;

		int nextStepMode = MAX_MICROSTEPPING_MODE;
		int tempStepSize = nextStepSize;
		while(tempStepSize > 1)
		{
			tempStepSize >> 1;
			nextStepMode--;
		}		

		// Check if a step needs to be done yet
		if(_lastStepTime  + (_usPerFullStep >> nextStepMode < currentTime)
		{
			setMicrosteppingMode(nextStepMode);			

			// Do a step
			digitalWrite(_stepPin, 1);
			delayMicroseconds(5);
			digitalWrite(_stepPin, 0);

			_lastStepTime += _usPerMicroStep;
			_stepCounter += nextStepSize;
		}
	}
}

void Stepper::accelerate()
{
	long currentTime = micros();
	float timeSinceLastUpdate = (currentTime - _lastUpdateTime) * 1e-6;
	_lastUpdateTime = currentTime;

	if (_currentSpeed == _desiredSpeed)
		return;

	bool decelerating = _currentSpeed > _desiredSpeed;
	if (decelerating)
	{
		_currentSpeed -= timeSinceLastUpdate * _acceleration;
		if (_currentSpeed < _desiredSpeed)
			_currentSpeed = _desiredSpeed;
	}
	else
	{
		_currentSpeed += timeSinceLastUpdate * _acceleration;
		if (_currentSpeed > _desiredSpeed)
			_currentSpeed = _desiredSpeed;
	}

	_usPerFullStep = 1e6 / abs(_currentSpeed);

	setDirection(_currentSpeed > 0 ? 1 : 0);
	updateMicrosteppingMode();
}

void Stepper::setDirection(bool direction)
{
	if (direction ^ _reverse)
		digitalWrite(_dirPin, 1);
	else
		digitalWrite(_dirPin, 0);
}

void Stepper::updateMicrosteppingMode()
{
	if (_currentSpeed == 0)
	{
		_currentMicrosteppingMode = -1;
	}
	else
	{
		int maxMicroStepsPerFullStep = _usPerFullStep / _minUsPerStep;
		int microsteppingMode = 0;

		while(maxMicroStepsPerFullStep > 1)
		{
			maxMicroStepsPerFullStep >>= 1;
			microsteppingMode += 1;
		}

		if (microsteppingMode > MAX_MICROSTEPPING_MODE)
			microsteppingMode = MAX_MICROSTEPPING_MODE;		

		_desiredMicrosteppingMode = microsteppingMode;		
	}
}

void Stepper::setMicrosteppingMode(int microsteppingMode)
{
	if(microsteppingMode == _currentMicrosteppingMode)
		return;

	_currentMicrosteppingMode = microsteppingMode;	
	switch (microsteppingMode)
	{
	case 0:
		digitalWrite(_ms1Pin, 0);
		digitalWrite(_ms2Pin, 0);
		digitalWrite(_ms3Pin, 0);
		break;

	case 1:
		digitalWrite(_ms1Pin, 1);
		digitalWrite(_ms2Pin, 0);
		digitalWrite(_ms3Pin, 0);
		break;

	case 2:
		digitalWrite(_ms1Pin, 0);
		digitalWrite(_ms2Pin, 1);
		digitalWrite(_ms3Pin, 0);
		break;

	case 3:
		digitalWrite(_ms1Pin, 0);
		digitalWrite(_ms2Pin, 0);
		digitalWrite(_ms3Pin, 1);
		break;

	case 4:
		digitalWrite(_ms1Pin, 1);
		digitalWrite(_ms2Pin, 1);
		digitalWrite(_ms3Pin, 1);
		break;
	}
}
