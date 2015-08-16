#pragma once

#define SERIAL_DEBUG_ENABLED 1

#define GET_NUM_ARGS(...) GET_NUM_ARGS_ACT(__VA_ARGS__, 5,4,3,2,1)
#define GET_NUM_ARGS_ACT(_1,_2,_3,_4,_5,N,...) N

#define macro_dispatcher(func, ...) \
            macro_dispatcher_(func, GET_NUM_ARGS(__VA_ARGS__))
#define macro_dispatcher_(func, nargs) \
            macro_dispatcher__(func, nargs)
#define macro_dispatcher__(func, nargs) \
            func ## nargs

#if SERIAL_DEBUG_ENABLED
#define DebugPrint(...) macro_dispatcher(DebugPrint, __VA_ARGS__)(__VA_ARGS__)
#define DebugPrintln(...) macro_dispatcher(DebugPrintln, __VA_ARGS__)(__VA_ARGS__)
#define DebugPrint2(str,modifier)  \
        Serial.print(millis());     \
        Serial.print(": ");    \
        Serial.print(__PRETTY_FUNCTION__); \
        Serial.print(' ');      \
        Serial.print(__LINE__);     \
        Serial.print(' ');      \
        Serial.print(str,modifier);
#define DebugPrint1(str)  \
        Serial.print(millis());     \
        Serial.print(": ");    \
        Serial.print(__PRETTY_FUNCTION__); \
        Serial.print(' ');      \
        Serial.print(__LINE__);     \
        Serial.print(' ');      \
        Serial.print(str);
#define DebugPrintln2(str,modifier)  \
        Serial.print(millis());     \
        Serial.print(": ");    \
        Serial.print(__PRETTY_FUNCTION__); \
        Serial.print(' ');      \
        Serial.print(__LINE__);     \
        Serial.print(' ');      \
        Serial.println(str,modifier);
#define DebugPrintln1(str)  \
        Serial.print(millis());     \
        Serial.print(": ");    \
        Serial.print(__PRETTY_FUNCTION__); \
        Serial.print(' ');      \
        Serial.print(__LINE__);     \
        Serial.print(' ');      \
        Serial.println(str);
#else
#define DebugPrint(...) macro_dispatcher(DebugPrint, __VA_ARGS__)(__VA_ARGS__)
#define DebugPrintln(...) macro_dispatcher(DebugPrintln, __VA_ARGS__)(__VA_ARGS__)
#define DebugPrint1(str)
#define DebugPrintln1(str)
#define DebugPrint2(str,modifier)
#define DebugPrintln2(str,modifier)
#endif

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

  unsigned long  _maxUsPerStep;    // Given the current max speed, how fast can steps be done?
  unsigned long  _lastUpdateTime;  // The last time updateSpeed was called
  unsigned long  _lastStepTime;    // The last step time in microseconds
  unsigned long  _usPerFullStep;   // The current interval between steps in microseconds. 0 means stopped.
  unsigned long  _usPerMicroStep;

  void setDirection(bool forward);
  void updateMicrosteppingMode();
  void setMicrosteppingMode(int microsteppingMode);
};
