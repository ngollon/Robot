#include "Arduino.h"
#include "PID.h"

PID::PID(float ki, float kp, float kd)
{
    _ki = ki;
    _kp = kp;
    _kd = kd;

    _lastCalculateTime = 0;
    _lastError = 0;
    _integratedError = 0;
}

float PID::calculate(float setPoint, float currentValue)
{
    unsigned long currentTime = micros();
    if(_lastCalculateTime == 0)
    {
        _lastCalculateTime = currentTime;
        _lastError = setPoint - currentValue;
        return setPoint;
    }
    else
    {
        // Calculate different error types
        float diffTime = (currentTime - _lastCalculateTime) * 1e-6;
        float error = setPoint - currentValue;
        _integratedError += error * diffTime;
        float differentialError = (_lastError - error) / diffTime;

        _lastError = error;

        return _ki * _integratedError + _kp * error + _kd * differentialError;
    }
}
