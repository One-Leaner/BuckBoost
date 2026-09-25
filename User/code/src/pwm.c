#include "pwm.h"
#include "ch32v30x.h"

pwm_cfg_t pwm_cfg;

/* 将纳秒转换为 TIM1 tick 数（四舍五入） */
#define TICK_FROM_NS(ns) \
    (uint16_t)((PWM_TIM_CLK_HZ / 1000000UL * ns + 500UL) / 1000UL)

/* 将占空比（x10）转换为 CCR 值 */
#define TICK_FROM_DUTY_X10(duty_x10) \
    (uint16_t)(pwm_cfg.period * duty_x10 / 1000UL)

/*********************************************************************
 * @fn      TIM1_Dead_Time_Init
 *
 * @brief   初始化 TIM1 四通道互补 PWM（带死区）
 *          tick 值由 pwm.h 中的物理参数自动计算
 *
 * @return  none
 */
void TIM1_Dead_Time_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStc = {0};
    TIM_OCInitTypeDef TIM_OCInitStc = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStc = {0};
    TIM_BDTRInitTypeDef TIM_BDTRInitStc = {0};

    /* --- 根据物理参数计算 tick 值 --- */
    pwm_cfg.period = (uint16_t)(PWM_TIM_CLK_HZ / PWM_FREQ_HZ);
    pwm_cfg.arr = pwm_cfg.period - 1;
    pwm_cfg.deadtime = TICK_FROM_NS(PWM_DEADTIME_NS);
    pwm_cfg.min_ccr = TICK_FROM_DUTY_X10(PWM_MIN_DUTY_X10);
    pwm_cfg.max_ccr = TICK_FROM_DUTY_X10(PWM_MAX_DUTY_X10);
    pwm_cfg.ch4_init = (pwm_cfg.deadtime + pwm_cfg.min_ccr) / 2;

    /* --- GPIO 初始化 --- */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOE | RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);
    GPIO_InitStc.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStc.GPIO_Speed = GPIO_Speed_50MHz;

    /* TIM1_CH1 */
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOE, &GPIO_InitStc);

    /* TIM1_CH1N */
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOE, &GPIO_InitStc);

    /* TIM1_CH2  BOOST输出 */
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOE, &GPIO_InitStc);

    /* TIM1_CH2N */
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOE, &GPIO_InitStc);

    /* TIM1_CH4 */
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_InitStc);

    /* --- 时基配置 --- */
    TIM_TimeBaseInitStc.TIM_Period = pwm_cfg.arr;
    TIM_TimeBaseInitStc.TIM_Prescaler = 0;
    TIM_TimeBaseInitStc.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStc.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStc);

    /* --- CH1/CH2 输出比较配置 --- */
    TIM_OCInitStc.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStc.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStc.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStc.TIM_Pulse = pwm_cfg.min_ccr;
    TIM_OCInitStc.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStc.TIM_OCNPolarity = TIM_OCPolarity_High;
    TIM_OCInitStc.TIM_OCIdleState = TIM_OCNIdleState_Reset;
    TIM_OCInitStc.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC1Init(TIM1, &TIM_OCInitStc);
    TIM_OC2Init(TIM1, &TIM_OCInitStc);

    /* --- CH4: ADC 触发用（低电平有效，上升沿触发 ADC）--- */
    TIM_OCInitStc.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStc.TIM_Pulse = pwm_cfg.ch4_init;
    TIM_OC4Init(TIM1, &TIM_OCInitStc);

    /* TRGO = OC4Ref, 用于触发 ADC 注入组转换 */
    TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC4Ref);

    /* --- 死区/刹车配置 --- */
    TIM_BDTRInitStc.TIM_OSSIState = TIM_OSSIState_Disable;
    TIM_BDTRInitStc.TIM_OSSRState = TIM_OSSRState_Disable;
    TIM_BDTRInitStc.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
    TIM_BDTRInitStc.TIM_DeadTime = pwm_cfg.deadtime;
    TIM_BDTRInitStc.TIM_Break = TIM_Break_Disable;
    TIM_BDTRInitStc.TIM_BreakPolarity = TIM_BreakPolarity_High;
    TIM_BDTRInitStc.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStc);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}