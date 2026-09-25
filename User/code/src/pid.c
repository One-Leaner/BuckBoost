#include "pid.h"

void pid_init (pid_obj_t *obj) {
    obj->kp = 1;
    obj->ki = 1;
    obj->kd = 0;
    obj->f = 100000;
    obj->T = 1.0 / obj->f;
    obj->y_limit_h = 0;
    obj->y_limit_l = 0;
    obj->err = 0;
    obj->err_1 = 0;
    obj->err_2 = 0;
    obj->y = 0;
}

void pid_ctrl (pid_obj_t *obj, float ref, float fbk) {
    obj->err = ref - fbk;
    obj->y += obj->kp * (obj->err - obj->err_1) + obj->ki * obj->err * obj->T + obj->kd * (obj->err - 2 * obj->err_1 + obj->err_2) * obj->f;
    obj->err_2 = obj->err_1;
    obj->err_1 = obj->err;

    if (obj->y > obj->y_limit_h)
        obj->y = obj->y_limit_h;

    if (obj->y < obj->y_limit_l)
        obj->y = obj->y_limit_l;
}