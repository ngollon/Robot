#pragma once
#include "I2Cdev.h"
#include "MPU6050.h"

class AngleSensor
{
public:
  AngleSensor();

  void update();
  float getAngle();
  float getRate();

private:
  MPU6050 _mpu;
  unsigned long _lastUpdateTime;   // last time update was called, in milliseconds
  float _currentAngle;             // Current estimate for angle in degrees
  float _currentRate;              // Current estimate for rate in degrees/s
}
