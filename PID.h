#pragma once

class PID
{
public:
  PID(float ki, float kp, float kd);
  float calculate(float setPoint, float currentValue);

private:
  // Tuning constants
  float _ki;
  float _kp;
  float _kd;

  unsigned long _lastCalculateTime;    // last time calculate was called, in microseconds
  float _lastError;
  float _integratedError;
}
