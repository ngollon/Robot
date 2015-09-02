#include "Stepper.h"
#include "Arduino.h"

#define SET(p) *p.port |= (1 << p.index)
#define CLR(p) *p.port &= ~(1 << p.index)
#define CFG_OUT(p) *p.ddr |= (1 << p.index)

Stepper::Stepper(	unsigned long minUsPerStep,
					Pin dirPin,
					Pin stepPin,
					Pin ms1Pin,
					Pin ms2Pin,
					Pin ms3Pin,
					bool reverse)
{
	// Set some consistent default values
	_maxSpeed = 1.0;
	_minUsPerStep = minUsPerStep;
	_acceleration = 1.0;
	_desiredSpeed = 0.0;
	_currentSpeed = 0.0;
	_lastStepTime = 0;
	_usPerFullStep = 0;
	_stepCounter = 0;
	_currentMicrosteppingMode = -1;

	// Store pin layout
	_reverse = reverse;
	_dirPin = dirPin;
	_stepPin = stepPin;
	_ms1Pin = ms1Pin;
	_ms2Pin = ms2Pin;
	_ms3Pin = ms3Pin;

	// Configure pins
	CFG_OUT(_dirPin);
	CFG_OUT(_stepPin);
	CFG_OUT(_ms1Pin);
	CFG_OUT(_ms2Pin);
	CFG_OUT(_ms3Pin);
}

void Stepper::setMaxSpeed(float speed)
{
	_maxSpeed = speed;
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

	// If we run for the first time, or the motor is stopped, do nothing.
	if (_lastStepTime == 0 || _desiredMicrosteppingMode == -1)
	{
		_lastStepTime = currentTime;
		return;
	}

	int nextStepSize;
	int nextStepMode;

	// If we have not changed the microstepping mode, we continue as usual
	if(_currentMicrosteppingMode == _desiredMicrosteppingMode)
	{
		nextStepMode = _currentMicrosteppingMode;
		nextStepSize = (1 << (MAX_MICROSTEPPING_MODE - _currentMicrosteppingMode));
	}
	else
	{
		// First, check what the fastest possible microstep is with the current state of stepCounter
		// We need to be sure that e.g. a quarter step is only done, if the stepCounter is divisible by 4.

		// We try to be fast here and avoid division since this is run in an ISR.
		// See https://graphics.stanford.edu/~seander/bithacks.html for all the nasty stuff in here
		nextStepSize = 1 << MAX_MICROSTEPPING_MODE;
		if(_stepCounter > 0)
			nextStepSize = _stepCounter & -_stepCounter;

		int desiredStepSize = 1 << (MAX_MICROSTEPPING_MODE - _desiredMicrosteppingMode);

		if(nextStepSize > desiredStepSize)
			nextStepSize = desiredStepSize;

		// Determine the microstepping mode from the chosen step size
		nextStepMode = MAX_MICROSTEPPING_MODE;
		int tempStepSize = nextStepSize;
		while(tempStepSize > 1)
		{
			tempStepSize >>= 1;
			nextStepMode--;
		}
	}

	unsigned long nextStepTime = _lastStepTime  + (_usPerFullStep >> nextStepMode);

	// Check if a step needs to be done yet
	if(nextStepTime < currentTime)
	{
		if(_currentMicrosteppingMode != nextStepMode)
			setMicrosteppingMode(nextStepMode);

		// Do a step
		SET(_stepPin);
		delayMicroseconds(2);
		CLR(_stepPin);

		_lastStepTime = nextStepTime;
		_stepCounter += nextStepSize;

		// stepCounter is only needed modulo the largest step size
		_stepCounter &= (1 << MAX_MICROSTEPPING_MODE) - 1;
	}
}

void Stepper::accelerate()
{
	unsigned long currentTime = micros();

	Serial.print(_currentMicrosteppingMode, DEC);Serial.print("\t");
	Serial.println();


	if(_lastUpdateTime == 0)
	{
			_lastUpdateTime = currentTime;
			return;
	}

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
		SET(_dirPin);
	else
		CLR(_dirPin);
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
	_currentMicrosteppingMode = microsteppingMode;
	switch (microsteppingMode)
	{
	case 0:
		CLR(_ms1Pin);
		CLR(_ms2Pin);
		CLR(_ms3Pin);
		break;

	case 1:
		SET(_ms1Pin);
		CLR(_ms2Pin);
		CLR(_ms3Pin);
		break;

	case 2:
		CLR(_ms1Pin);
		SET(_ms2Pin);
		CLR(_ms3Pin);
		break;

	case 3:
		SET(_ms1Pin);
		SET(_ms2Pin);
		CLR(_ms3Pin);
		break;

	case 4:
		SET(_ms1Pin);
		SET(_ms2Pin);
		SET(_ms3Pin);
		break;
	}
}
