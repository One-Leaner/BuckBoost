#include "lead.h"

void lead_init(lead_obj_t *obj)
{
    obj->a1 = 0;
    obj->b0 = 1;
    obj->b1 = 0;
    obj->y = 0;
    obj->y_1 = 0;
    obj->u = 0;
    obj->u_1 = 0;
}

void lead_ctrl(lead_obj_t *obj, float u)
{
    obj->u = u;
    obj->y = -obj->a1 * obj->y_1 + obj->b0 * obj->u + obj->b1 * obj->u_1;
    obj->y_1 = obj->y;
    obj->u_1 = obj->u;
}