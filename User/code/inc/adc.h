#ifndef __ADC_H__
#define __ADC_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct

{
    uint16_t VIN_ADC;
    uint16_t VOUT_ADC;
    uint16_t IL_ADC;
    uint16_t IOUT_ADC;
    uint16_t IIN_ADC;
    int16_t Calibrattion_Val_adc1;
    int16_t Calibrattion_Val_adc2;
    uint8_t adc1_flag;
    uint8_t adc2_flag;
} Digital_Param_t;

typedef struct
{
    float VIN;  // 输入电压
    float VOUT; // 输出电压
    float IL;   // 电感电流
    float IOUT; // 输出电流
    float IIN;  // 输入电流
} Physical_Param_t;

extern Physical_Param_t physical_param;

bool sample_run(void);
void ADC_Function_Init(void);

#endif