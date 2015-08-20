#include "Arduino.h"
#include "AngleSensor.h"

#if 0

#define GYRO_FACTOR 0.98
#define ACCEL_FACTOR 0.02

// Where do these come from?
#define GYRO_X_OFFSET -327
#define GYRO_Y_OFFSET = 160;
#define GYRO_Z_OFFSET = 17;



AngleSensor::AngleSensor()
{
    // Initialize variables
    _lastUpdateTime = 0;

    Fastwire::setup(800, true);
    _mpu.initialize();
    _mpu.setXGyroOffset(0);
    _mpu.setYGyroOffset(0);
    _mpu.setZGyroOffset(0);
}

float AngleSensor::getAngle()
{
    return _currentAngle;
}

float AngleSensor::getRate()
{
    return _currentRate;
}

void AngleSensor::update()
{
    unsigned long currentTime = micros();

    // Read raw accel/gyro measurements from MPU6050
    float ax, ay, az, gx, gy, gz;
    _mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Remove offsets
    gx -= GYRO_X_OFFSET;
    gy -= GYRO_Y_OFFSET;
    gz -= GYRO_Z_OFFSET;

    // Calculate angles both from accelerometer and gyroscope
    float angleAccel = -atan((double)ax / az) * 57.29578;

    // Correct for angle overflow...
    if (angleAccel > 0 && abs(angleAccel) > 80)
        angleAccel = abs(angleAccel) * ((ax < 0) ? -1 : 1);

    _currentRate = gy * 7.62939453125e-6;

    float diffTime = (currentTime - _lastUpdateTime) * 1e-6;	// in seconds
    if(_lastUpdateTime == 0)
    {
        _currentAngle = angleAccel;
    }
    else
    {
        float angleGyro = _currentAngle + _currentRate * diffTime;   // 250 / 32768 / 1000000
        _currentAngle = angleGyro * GYRO_FACTOR + angleAccel * ACCEL_FACTOR;
    }
    _lastUpdateTime = currentTime;
}
#endif
