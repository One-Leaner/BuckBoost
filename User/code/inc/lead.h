#ifndef __LEAD_H__
#define __LEAD_H__

typedef struct
{
    float a1;
    float b0;
    float b1;
    float y;
    float y_1;
    float u;
    float u_1;
} lead_obj_t;

void lead_init(lead_obj_t *obj);
void lead_ctrl(lead_obj_t *obj, float u);

#endif