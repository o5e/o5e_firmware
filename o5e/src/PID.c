
/********************************************************************

  @note Generic, object oriented PID algorithm
  @note Currently, must be called on a fixed, regular time basis 
  @note for example, every 10 msec (this needs to be changed to use a clock)
  @note alternate: OUTPUT[0] = (Kp + Ki + Kd) * Error[0] - (Kp + 2 * Kd) * Error[1] + Kd * Error[2] + OUTPUT[1]

********************************************************************/

// caller needs to create and fill out a structure like this:

struct pid_struct {
    int32_t desired;            // desired output value
    int32_t current;            // current output value
    int32_t control;            // control/input value (gets updated below)
    int32_t Kp;                 // proportional gain  - bin8, example: .3
    int32_t Ki;                 // integral gain  - bin8, example: .1
    int32_t Kd;                 // derivative gain  - bin8, example: .1
    int32_t prev_error;         // used for derivative calc
    int32_t integral;           // saves integral between calls
};

int32_t PID(struct PID_struct *pid)
{
    int32_t error;
    int32_t correction;
    int32_t derivative;

// note: call this routine on some fixed, regular time basis
#define dt 1

    error = pid->desired - pid->current;

    pid->integral += error * dt;
    derivative = (error - pid->prev_error) / dt;

    // reset integral if at target - reduces overshoot
    if ((error >= 0 && pid->prev_error <= 0)
        || (error <= 0 && pid->prev_error >= 0))
        pid->integral = 0;

    // Caculate P,I,D
    correction =
        (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);

    // Adjiust bin point - bin8 to bin0
    correction = correction >> 8;

    // Save previous error for derivative calc
    pid->prev_error = error;

    // adjust control value
    pid->control += correction;

    return 0;
}                               // PID()
