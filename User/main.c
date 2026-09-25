#include "adc.h"
#include "debug.h"
#include "lead.h"
#include "pid.h"
#include "pwm.h"

/* --- 控制参数 --- */
#define VOUT_HYST 1.0f      // 模式切换滞回 (V)
#define IL_MAX 10.0f        // 电感电流最大值 (A)，电压环输出限幅
#define OUTER_LOOP_RATIO 10 // 外环降频比: 外环频率 = 内环频率 / 10 (=10kHz)
static float vref = 5.0f;   // 目标输出电压 (V), 可运行时修改

/* --- 数据打印 --- */
#define LOG_PERIOD_MS 500UL // 打印间隔 (ms)，改这里调频率

/* --- 硬件保护阈值 --- */
#define HW_PROTECT_EN 0 // 硬件保护开关：0=关(调试) / 1=开(正式)
#define VOUT_OVP 6.0f   // 输出过压保护 (V), vref * 1.2
#define IL_OCP 12.0f    // 电感过流保护 (A), 高于控制限幅 IL_MAX

/* --- BUCK 模式 PID 参数 (MATLAB 连续域) --- */
#define BUCK_V_KP 7.053491439964143f
#define BUCK_V_KI 1.577208633847270e+05f
#define BUCK_I_KP 0.034581792022934f
#define BUCK_I_KI 7.732723774704141e+02f

/* --- BOOST 模式 PID 参数 (MATLAB 连续域) --- */
#define BOOST_V_KP 70.496186420330190f
#define BOOST_V_KI 2.101790199871411e+05f
#define BOOST_I_KP 0.041730791579989f
#define BOOST_I_KI 1.244171823036415e+02f

/* --- BOOST 模式相位超前参数 (离散域) --- */
// denz=[1, -0.472540787641336]  numz=[1.220881118146201, -0.693421905787537]
#define BOOST_LEAD_B0 1.220881118146201f
#define BOOST_LEAD_B1 -0.693421905787537f
#define BOOST_LEAD_A1 -0.472540787641336f

/* --- 变换器工作模式 --- */
typedef enum
{
    MODE_BUCK,      // 降压: VIN > VOUT
    MODE_BOOST,     // 升压: VIN < VOUT
    MODE_BUCK_BOOST // 升降压: VIN ≈ VOUT
} ConvMode_t;

/* Global Variable */
pid_obj_t vpid;                          // 外环: 电压PI
pid_obj_t ipid;                          // 内环: 电流PI
lead_obj_t vlead;                        // 电压环相位超前补偿
float il_ref = 0.5f;                     // 电流参考值(电压环输出), 初始值
ConvMode_t conv_mode;                    // 当前工作模式
static ConvMode_t prev_mode = MODE_BUCK; // 上一周期模式(用于滞回)
static uint32_t loop_cnt = 0;            // 内环周期计数器(外环降频用)
static uint32_t log_cnt = 0;             // 数据打印计数器

/*********************************************************************
 * @fn      conv_mode_select
 *
 * @brief   根据VIN与vref的差值选择工作模式(带滞回)
 *          VIN > vref + hyster 则 选择 BUCK
 *          VIN < vref - hyster 则 选择 BOOST
 *          |VIN-vref| ≤ hyster   则 选择 BUCK_BOOST
 *
 * @param   vin - 输入电压实测值(V)
 *
 * @return  ConvMode_t - 工作模式
 */
ConvMode_t conv_mode_select(float vin)
{
    ConvMode_t new_mode;
    if (vin > vref + VOUT_HYST)
        new_mode = MODE_BUCK;
    else if (vin < vref - VOUT_HYST)
        new_mode = MODE_BOOST;
    else
        new_mode = MODE_BUCK_BOOST;

    if (new_mode == MODE_BUCK_BOOST || prev_mode == MODE_BUCK_BOOST)
        prev_mode = new_mode;
    else if (new_mode != prev_mode)
        prev_mode = new_mode;

    return prev_mode;
}

/*********************************************************************
 * @fn      pid_switch_params
 *
 * @brief   根据工作模式切换双环 PID 参数
 *          BUCK_BOOST 模式沿用 BUCK 参数
 *
 * @param   mode - 当前工作模式
 */
void pid_switch_params(ConvMode_t mode)
{
    if (mode == MODE_BOOST)
    {
        vpid.kp = BOOST_V_KP;
        vpid.ki = BOOST_V_KI;
        ipid.kp = BOOST_I_KP;
        ipid.ki = BOOST_I_KI;
        vlead.b0 = BOOST_LEAD_B0;
        vlead.b1 = BOOST_LEAD_B1;
        vlead.a1 = BOOST_LEAD_A1;
    }
    else /* MODE_BUCK / MODE_BUCK_BOOST: 相位超前直通 (b0=1, b1=0, a1=0) */
    {
        vpid.kp = BUCK_V_KP;
        vpid.ki = BUCK_V_KI;
        ipid.kp = BUCK_I_KP;
        ipid.ki = BUCK_I_KI;
        vlead.b0 = 1.0f;
        vlead.b1 = 0.0f;
        vlead.a1 = 0.0f;
    }
}

/*********************************************************************
 * @fn      pwm_update
 *
 * @brief   根据工作模式和PID输出刷新PWM占空比
 *          直通桥臂使用max_ccr，下管每周期短暂导通→自举电容自动充电
 *
 * @param   mode - 工作模式
 * @param   duty - PID输出的占空比值(CCR寄存器值)
 */
void pwm_update(ConvMode_t mode, uint16_t duty)
{
    if (duty < pwm_cfg.min_ccr)
        duty = pwm_cfg.min_ccr;
    if (duty > pwm_cfg.max_ccr)
        duty = pwm_cfg.max_ccr;

    switch (mode)
    {
    case MODE_BUCK:
        TIM_SetCompare1(TIM1, duty);            // Buck桥臂PWM (Q1/Q2开关)
        TIM_SetCompare2(TIM1, pwm_cfg.max_ccr); // Boost桥臂直通
        break;
    case MODE_BOOST:
        TIM_SetCompare1(TIM1, pwm_cfg.max_ccr); // Buck桥臂直通
        TIM_SetCompare2(TIM1, duty);            // Boost桥臂PWM (Q3/Q4开关)
        break;
    case MODE_BUCK_BOOST:
        TIM_SetCompare1(TIM1, duty); // 两桥臂同步PWM
        TIM_SetCompare2(TIM1, duty);
        break;
    }

    // 动态调整CH4上升沿 = (死区 + ON中点)，ADC在电感电流中点采样
    TIM_SetCompare4(TIM1, (pwm_cfg.deadtime + duty) / 2);
}

/*********************************************************************
 * @fn      log_print
 *
 * @brief   定时打印电路运行数据（外环上下文中调用）
 *
 * @return  none
 */
static void log_print(void)
{
    static const char *mode_name[] = {"BUCK", "BOOST", "BB"};
    printf("D: VIN=%.1fV VOUT=%.2fV IL=%.2fA Iref=%.2fA DUTY=%.1f%% MD=%s\r\n",
           (double)physical_param.VIN,
           (double)physical_param.VOUT,
           (double)physical_param.IL,
           (double)il_ref,
           (double)ipid.y * 100.0 / pwm_cfg.period,
           mode_name[conv_mode]);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   4开关Buck-Boost变换器电压闭环控制
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Debug_Init(115200);
    USART1_DMA_Idle_Init();
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("Buck-Boost Dual-Loop Start\r\n");

    // --- 初始化外设 ---
    ADC_Function_Init();
    TIM1_Dead_Time_Init();
    printf("PWM: period=%d DT=%d min=%d max=%d\r\n",
           pwm_cfg.period, pwm_cfg.deadtime, pwm_cfg.min_ccr, pwm_cfg.max_ccr);

    // --- 外环: 电压PI (10kHz) ---
    pid_init(&vpid);
    lead_init(&vlead);
    vpid.kp = BUCK_V_KP;
    vpid.ki = BUCK_V_KI;
    vpid.kd = 0.0f;
    vpid.f = (float)PWM_FREQ_HZ / OUTER_LOOP_RATIO;
    vpid.T = 1.0f / vpid.f;
    vpid.y_limit_h = IL_MAX;
    vpid.y_limit_l = 0.0f;
    vpid.y = il_ref;

    // --- 内环: 电流PI (100kHz) ---
    pid_init(&ipid);
    ipid.kp = BUCK_I_KP;
    ipid.ki = BUCK_I_KI;
    ipid.kd = 0.0f;
    ipid.f = (float)PWM_FREQ_HZ;
    ipid.T = 1.0f / ipid.f;
    ipid.y_limit_h = (float)pwm_cfg.max_ccr;
    ipid.y_limit_l = (float)pwm_cfg.min_ccr;
    ipid.y = (float)pwm_cfg.min_ccr;

    // --- 初始状态 ---
    conv_mode = MODE_BUCK;
    pwm_update(conv_mode, pwm_cfg.min_ccr);

    printf("V-PID: kp=%.2f ki=%.1f f=%.0fHz lim=[0,%.1fA]\r\n",
           vpid.kp, vpid.ki, vpid.f, IL_MAX);
    printf("I-PID: kp=%.2f ki=%.1f f=%.0fHz lim=[%d,%d]\r\n",
           ipid.kp, ipid.ki, ipid.f, pwm_cfg.min_ccr, pwm_cfg.max_ccr);
    printf("Lead: b0=%.3f b1=%.3f a1=%.3f (BUCK passthrough)\r\n",
           vlead.b0, vlead.b1, vlead.a1);

    while (1)
    {
        if (sample_run())
        {
            // ====== 硬件保护 (每周期检查, 最快响应) ======
#if HW_PROTECT_EN
            if (physical_param.VOUT > VOUT_OVP || physical_param.IL > IL_OCP)
            {
                pwm_update(conv_mode, pwm_cfg.min_ccr);
                ipid.y = (float)pwm_cfg.min_ccr;
                ipid.err_1 = 0.0f;
                ipid.err_2 = 0.0f;
                vpid.y = 0.0f;
                vpid.err_1 = 0.0f;
                vpid.err_2 = 0.0f;
                vlead.y = 0.0f;
                vlead.y_1 = 0.0f;
                vlead.u = 0.0f;
                vlead.u_1 = 0.0f;
                il_ref = 0.0f;
            }
            else
#endif
            {
                loop_cnt++;

                // ====== 内环: 电流PI (每周期执行) ======
                pid_ctrl(&ipid, il_ref, physical_param.IL);
                pwm_update(conv_mode, (uint16_t)ipid.y);

                // ====== 外环: 电压PI (降频 1/OUTER_LOOP_RATIO) ======
                if (loop_cnt == OUTER_LOOP_RATIO)
                {
                    ConvMode_t new_mode;
                    loop_cnt = 0;
                    new_mode = conv_mode_select(physical_param.VIN);
                    if (new_mode != conv_mode)
                    {
                        conv_mode = new_mode;
                        pid_switch_params(conv_mode);
                    }
                    pid_ctrl(&vpid, vref, physical_param.VOUT);
                    lead_ctrl(&vlead, vpid.y);
                    il_ref = vlead.y;
                    if (il_ref > IL_MAX)
                        il_ref = IL_MAX;
                    if (il_ref < 0.0f)
                        il_ref = 0.0f;

                    // ====== 定时打印 ======
                    log_cnt++;
                    if (log_cnt >= (LOG_PERIOD_MS * (PWM_FREQ_HZ / 1000UL) / OUTER_LOOP_RATIO))
                    {
                        log_cnt = 0;
                        log_print();
                    }

                    // ====== 非阻塞串口命令 (debug_scanf DMA方式) ======
                    char cmd[8];
                    float val;
                    if (debug_scanf("%7[^=]=%f", cmd, &val))
                    {
                        // printf("CMD\n");
                        if (strcmp(cmd, "VREF") == 0)
                        {
                            vref = val;
                            printf("OK %s=%.3f\r\n", cmd, (double)val);
                        }
                        else if (strcmp(cmd, "V_KP") == 0)
                        {
                            vpid.kp = val;
                            printf("OK %s=%.3f\r\n", cmd, (double)val);
                        }
                        else if (strcmp(cmd, "V_KI") == 0)
                        {
                            vpid.ki = val;
                            printf("OK %s=%.3f\r\n", cmd, (double)val);
                        }
                        else if (strcmp(cmd, "I_KP") == 0)
                        {
                            ipid.kp = val;
                            printf("OK %s=%.3f\r\n", cmd, (double)val);
                        }
                        else if (strcmp(cmd, "I_KI") == 0)
                        {
                            ipid.ki = val;
                            printf("OK %s=%.3f\r\n", cmd, (double)val);
                        }
                        else
                            printf("ERR unknown: %s\r\n", cmd);
                    }
                }
            }
        }
    }
}