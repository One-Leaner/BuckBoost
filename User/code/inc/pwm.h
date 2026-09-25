#ifndef __PWM_H__
#define __PWM_H__

#include <stdint.h>

/* ===== PWM 物理参数（修改这些即可，tick 由初始化自动计算）===== */
#define PWM_TIM_CLK_HZ 144000000UL /* TIM1 时钟频率 (144MHz)             */
#define PWM_FREQ_HZ 100000UL       /* 开关频率 (100kHz)                  */
#define PWM_DEADTIME_NS 347UL      /* 死区时间 (ns)                      */
#define PWM_MIN_DUTY_X10 65        /* 最小占空比 x10 (6.5%)              */
#define PWM_MAX_DUTY_X10 944       /* 最大占空比 x10 (94.4%), 直通桥臂用 */

/* ===== 运行时 tick 配置（由 TIM1_Dead_Time_Init 填充）===== */
typedef struct
{
    uint16_t period;   /* 周期 tick 数                          */
    uint16_t arr;      /* ARR 寄存器值 (period - 1)              */
    uint16_t min_ccr;  /* 最小占空比 CCR                         */
    uint16_t max_ccr;  /* 最大占空比 CCR                         */
    uint16_t deadtime; /* 死区 tick 数                          */
    uint16_t ch4_init; /* CH4 初始 CCR（采样中点）               */
} pwm_cfg_t;

extern pwm_cfg_t pwm_cfg;

void TIM1_Dead_Time_Init(void);

#endif