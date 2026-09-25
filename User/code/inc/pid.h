#ifndef __PID_H__
#define __PID_H__

typedef struct
{
    float kp;
    float ki;
    float kd;
    float f;
    float T;
    float y_limit_h;
    float y_limit_l;
    float err;
    float err_1;
    float err_2;
    float y;
} pid_obj_t;

void pid_init(pid_obj_t *obj);
void pid_ctrl(pid_obj_t *obj, float ref, float fbk);

#endif