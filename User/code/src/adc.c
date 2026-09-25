#include "ch32v30x.h"

#include "adc.h"

#define VIN_CONV_RATIO 0.0161172f  // 输入电压转换系数
#define VOUT_CONV_RATIO 0.0161172f // 输出电压转换系数
#define IL_CONV_RATIO 0.0016117f   // 电感电流转换系数
#define IOUT_CONV_RATIO 0.0158012f // 输出电流转换系数
#define IIN_CONV_RATIO 0.0158012f  // 输入电流转换系数

static Digital_Param_t digtal_param; // 数字参量
Physical_Param_t physical_param;     // 物理参量

bool sample_run(void)
{
    bool updated = false;
    if (digtal_param.adc1_flag)
    {
        digtal_param.adc1_flag = 0;
        physical_param.VIN = (digtal_param.VIN_ADC + digtal_param.Calibrattion_Val_adc1) * VIN_CONV_RATIO;
        physical_param.VOUT = (digtal_param.VOUT_ADC + digtal_param.Calibrattion_Val_adc1) * VOUT_CONV_RATIO;
        physical_param.IOUT = (digtal_param.IOUT_ADC + digtal_param.Calibrattion_Val_adc1) * IOUT_CONV_RATIO;
        physical_param.IIN = (digtal_param.IIN_ADC + digtal_param.Calibrattion_Val_adc1) * IIN_CONV_RATIO;
        updated = true;
    }
    if (digtal_param.adc2_flag)
    {
        digtal_param.adc2_flag = 0;
        physical_param.IL = (digtal_param.IL_ADC + digtal_param.Calibrattion_Val_adc2) * IL_CONV_RATIO - 3.3f;
        updated = true;
    }
    return updated;
}

void ADC_Function_Init(void)
{
    ADC_InitTypeDef ADC_InitStc = {0};
    GPIO_InitTypeDef GPIO_InitStc = {0};
    NVIC_InitTypeDef NVIC_InitStc = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);
    // 144MHz/2/6=12MHz，手册提到采样时钟不能超过14MHz【在system_ch32v30x.c中的SetSysClockTo144_HSE()函数下降PCLK2进行2分频】
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    GPIO_InitStc.GPIO_Mode = GPIO_Mode_AIN;

    /* CH1 */ // VIN
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOA, &GPIO_InitStc);

    /* CH2 */ // VOUT
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOA, &GPIO_InitStc);

    /* CH3 */ // IL
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStc);

    /* CH6 */ // IOUT
    GPIO_InitStc.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStc);

    ADC_DeInit(ADC1);
    ADC_DeInit(ADC2);

    ADC_InitStc.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStc.ADC_ScanConvMode = ENABLE;
    ADC_InitStc.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStc.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStc.ADC_NbrOfChannel = 0; // 规则组通道数

    ADC_Init(ADC1, &ADC_InitStc);
    ADC_Init(ADC2, &ADC_InitStc);

    // 注入组最多4个通道（注入组信号JEOC无法通知DMA转运，即无法使用DMA）
    ADC_InjectedSequencerLengthConfig(ADC1, 4);
    ADC_InjectedSequencerLengthConfig(ADC2, 1);
    ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_CC4);
    ADC_ExternalTrigInjectedConvConfig(ADC2, ADC_ExternalTrigInjecConv_T1_CC4);

    // 注意adc采样时间不要超过pwm的周期，否则会导致无法及时定时器响应TRGO，导致采样率降低，超过时间在1~2倍之间，采样率降低一半，以此类推
    // 总采样时间 1/12*(Ts+12.5)*N us，N为通道数
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_1Cycles5);  // VOUT
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_1Cycles5);  // IOUT
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_1Cycles5);  // IIN
    ADC_InjectedChannelConfig(ADC1, ADC_Channel_6, 4, ADC_SampleTime_1Cycles5);  // VIN
    ADC_InjectedChannelConfig(ADC2, ADC_Channel_14, 1, ADC_SampleTime_1Cycles5); // IL
    ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);
    ADC_ExternalTrigInjectedConvCmd(ADC2, ENABLE);

    NVIC_InitStc.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStc.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStc.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStc.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStc);
    ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    ADC_BufferCmd(ADC1, DISABLE); // disable buffer

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1))
        ;
    digtal_param.Calibrattion_Val_adc1 = Get_CalibrationValue(ADC1);

    ADC_ITConfig(ADC2, ADC_IT_JEOC, ENABLE);
    ADC_Cmd(ADC2, ENABLE);
    ADC_BufferCmd(ADC2, DISABLE); // disable buffer

    ADC_ResetCalibration(ADC2);
    while (ADC_GetResetCalibrationStatus(ADC2))
        ;
    ADC_StartCalibration(ADC2);
    while (ADC_GetCalibrationStatus(ADC2))
        ;
    digtal_param.Calibrattion_Val_adc2 = Get_CalibrationValue(ADC2);
}

void ADC1_2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************

 * @fn      ADC1_2_IRQHandler
 *
 * @brief   ADC1_2 Interrupt Service Function.
 *
 * @return  none
 */
void ADC1_2_IRQHandler()
{
    if (ADC_GetITStatus(ADC1, ADC_IT_JEOC))
    {
        ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
        digtal_param.VOUT_ADC = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1);
        digtal_param.IOUT_ADC = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_2);
        digtal_param.IIN_ADC = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_3);
        digtal_param.VIN_ADC = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_4);
        digtal_param.adc1_flag = 1;
    }

    if (ADC_GetITStatus(ADC2, ADC_IT_JEOC))
    {
        ADC_ClearITPendingBit(ADC2, ADC_IT_JEOC);
        digtal_param.IL_ADC = ADC_GetInjectedConversionValue(ADC2, ADC_InjectedChannel_1);
        digtal_param.adc2_flag = 1;
    }
}