#include "unity.h"
#include "app.h"
#include "adc.h"
#include "pwm.h"
#include "enc.h"
#include <string.h> 
#include <stdio.h>
#include "ucmd.h"
/********************************************************************************
  EXTERN FUNCTIONS & VARS.
********************************************************************************/
extern uint32_t ADC_ReadCh(ADC_Channel_E adc_ch_e);

extern int8_t MockADC_SetBuffer(uint8_t idx, uint32_t val);

extern uint32_t dctocnts(uint32_t dcycle);

extern uint32_t gs_adc_ch_buf[ADC_CH_MAX_E];

extern uint16_t gs_pwm_ch_buf[PWM_PH_MAX]; 

extern int32_t gs_enc_cnt;

extern uint16_t gs_gpio_pin;

/*-----------------------------------------------------------------------------
 * uCmd extern functions & variables. 
 *-----------------------------------------------------------------------------*/
extern uCmd_CharRxCallback MockUART_ChrRxCallback;

extern void MockUART_SendChar(uint8_t ch);

uint8_t test_buff[uCMD_BUFF_SIZE] = {0};
/*******************************************************************************/

void setUp(void) {
  memset(gs_adc_ch_buf, 0, sizeof(uint32_t) * ADC_CH_MAX_E);
  memset(gs_pwm_ch_buf, 0, sizeof(uint16_t) * PWM_PH_MAX); 
}

void tearDown(void) {}

void test_ADC_read_value(void) {
  MockADC_SetBuffer(0, 10);
  MockADC_SetBuffer(ADC_CH_MAX_E - 1, 40);
  TEST_ASSERT_EQUAL_UINT32(10, ADC_ReadCh(0));
  TEST_ASSERT_EQUAL_UINT32(40, ADC_ReadCh(ADC_CH_MAX_E - 1));
}

void test_ADC_read_value_out_of_range_channel(void) {
  MockADC_SetBuffer(0, 10);
  TEST_ASSERT_EQUAL_UINT32(INT32_MAX, ADC_ReadCh(ADC_CH_MAX_E));
}

void test_ADC_get_voltage_zero(void) {
  MockADC_SetBuffer(0, 0);
  TEST_ASSERT_EQUAL_UINT32(0, App_GetVoltage(0));
}

void test_ADC_get_voltage_out_of_range_channel(void) {
  TEST_ASSERT_EQUAL_UINT32(INT32_MAX, App_GetVoltage(VAdcChMax_E));
}

void test_ADC_get_voltage_max(void) {
  MockADC_SetBuffer(0, 4095);
  TEST_ASSERT_EQUAL_UINT32(3300, App_GetVoltage(0));
}

void test_ADC_get_voltage_mid_range(void) {
  MockADC_SetBuffer(0, 2200);
  TEST_ASSERT_EQUAL_UINT32(1772, App_GetVoltage(0));
}

void test_ADC_get_voltage_bias(void) {
  MockADC_SetBuffer(0, 1934);
  TEST_ASSERT_EQUAL_UINT32(APP_PARAMS_IFBK_BIAS, App_GetVoltage(0));
}

void test_ADC_get_current_zero(void) {
  MockADC_SetBuffer(0, 0);
  TEST_ASSERT_EQUAL_INT32(-2337, App_GetCurrent(0));
}

void test_ADC_get_current_out_of_range_channel(void) {
  MockADC_SetBuffer(0, 0);
  TEST_ASSERT_EQUAL_INT32(INT32_MAX, App_GetCurrent(IfbkPhMax_E));
}

void test_ADC_get_current_zero_bias(void) {
  MockADC_SetBuffer(IfbkPhA_E, APP_PARAMS_IFBK_BIAS_ADC_COUNTS);
  TEST_ASSERT_EQUAL_INT32(0, App_GetCurrent(IfbkPhA_E));
}

void test_ADC_get_current_max(void) {
  MockADC_SetBuffer(IfbkPhA_E, 4095);
  TEST_ASSERT_EQUAL_INT32(2613, App_GetCurrent(IfbkPhA_E));
}

void test_ADC_get_current_min(void) {
  MockADC_SetBuffer(IfbkPhA_E, 0);
  TEST_ASSERT_EQUAL_INT32(-2337, App_GetCurrent(IfbkPhA_E));
}

void test_ADC_get_current_channels(void) {
  MockADC_SetBuffer(0, 1000);
  MockADC_SetBuffer(1, 2000);
  MockADC_SetBuffer(2, 3000);
  MockADC_SetBuffer(3, 4000);
  TEST_ASSERT_EQUAL_INT32(-1129, App_GetCurrent(IfbkPhA_E));
  TEST_ASSERT_EQUAL_INT32(79, App_GetCurrent(IfbkPhB_E));
  TEST_ASSERT_EQUAL_INT32(1288, App_GetCurrent(IfbkPhC_E));
  TEST_ASSERT_EQUAL_INT32(INT32_MAX , App_GetCurrent(IfbkPhMax_E));
}

void test_App_SetPwmDutyCycle_value(void) {
  App_SetPwmDutyCycle(0, 10);
  TEST_ASSERT_EQUAL_UINT16(1, gs_pwm_ch_buf[0]);
}

void test_App_SetPwmDutyCycle_out_of_range_channel(void) {
  uint8_t idx;
  App_SetPwmDutyCycle(PwmChMax_E, 10);
  for(idx = 0; idx < 3; idx++) {
    TEST_ASSERT_EQUAL_UINT16(0, gs_pwm_ch_buf[idx]);
  }
}

void test_App_SetPwmDutyCycle_out_of_range_value(void) {
  App_SetPwmDutyCycle(PwmChA_E, 10001);
  TEST_ASSERT_EQUAL_UINT16(PWM_TMR_ARR, gs_pwm_ch_buf[0]);
}

void test_App_SetPwmVoltage_zero_value(void) {
  App_SetPwmVoltage(PwmChC_E, 0);
  TEST_ASSERT_EQUAL_UINT16(0, gs_pwm_ch_buf[2]);
}

void test_App_SetPwmVoltage_mid_range_value(void) {
  App_SetPwmVoltage(PwmChB_E, 5001);
  TEST_ASSERT_EQUAL_UINT16(499, gs_pwm_ch_buf[1]);
}

void test_App_SetPwmVoltage_max_value(void) {
  App_SetPwmVoltage(PwmChC_E, APP_PARAMS_PWM_VOLTS);
  TEST_ASSERT_EQUAL_UINT16(PWM_TMR_ARR, gs_pwm_ch_buf[2]);
}

void test_APP_SetPwmVoltage_out_of_range_value(void) {
  App_SetPwmVoltage(PwmChA_E, 100000000);
  TEST_ASSERT_EQUAL_UINT16(PWM_TMR_ARR, gs_pwm_ch_buf[0]);
}

void test_App_GetPwmDutyCycle_zero(void) {
  TEST_ASSERT_EQUAL_UINT32(0, App_GetPwmDutyCycle(0));
}

void test_App_GetPwmDutyCycle_out_of_range_channel(void) {
  TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, App_GetPwmDutyCycle(PwmChMax_E));
}

void test_App_GetPwmDutyCycle_max_counter(void) {
  gs_pwm_ch_buf[0] = PWM_TMR_ARR;
  TEST_ASSERT_EQUAL_UINT32(10000, App_GetPwmDutyCycle(0));
}

void test_App_GetPwmDutyCycle_mid_range_value(void) {
  gs_pwm_ch_buf[2] = 500;
  TEST_ASSERT_EQUAL_UINT32(4170, App_GetPwmDutyCycle(PwmChC_E));
}

void test_App_GetPwmVoltage_zero_value(void) {
  TEST_ASSERT_EQUAL_UINT32(0, App_GetPwmVoltage(PwmChA_E));
}

void test_App_GetPwmVoltage_out_of_range_channel(void) {
  TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, App_GetPwmVoltage(PwmChMax_E));
}

void test_App_GetPwmVoltage_mid_range_value(void) {
  gs_pwm_ch_buf[1] = 999; 
  TEST_ASSERT_EQUAL_UINT32(9997, App_GetPwmVoltage(PwmChB_E));
}

void test_App_GetBusVoltage_zero_value(void) {
  MockADC_SetBuffer(ADC_VBUS_CH_E, 0);
  TEST_ASSERT_EQUAL_UINT32(0, App_GetBusVoltage());
}

void test_App_GetBusVoltage_mid_range_value(void) {
  MockADC_SetBuffer(ADC_VBUS_CH_E, 100);
  TEST_ASSERT_EQUAL_UINT32(1532, App_GetBusVoltage());
}

void test_App_GetBusVoltage_max_range_value(void) {
  MockADC_SetBuffer(ADC_VBUS_CH_E, 4095);
  TEST_ASSERT_EQUAL_UINT32(63201, App_GetBusVoltage());
}

void test_App_GetTemp_zero_buffer(void) {
  MockADC_SetBuffer(ADC_TEMP_SENS_CH_E, 0);
  TEST_ASSERT_EQUAL_INT32(-20365, App_GetTemp());
}

void test_App_GetTemp_mid_buffer(void) {
  MockADC_SetBuffer(ADC_TEMP_SENS_CH_E, 1500);
  TEST_ASSERT_EQUAL_INT32(31579, App_GetTemp());
}

void test_App_GetTemp_max_buffer(void) {
  MockADC_SetBuffer(ADC_TEMP_SENS_CH_E, 4095);
  TEST_ASSERT_EQUAL_INT32(121535, App_GetTemp());
}

void test_App_GetMotorPosition_zero_value(void) {
  gs_enc_cnt = 0;
  TEST_ASSERT_EQUAL_INT16(0, App_GetPosition());
}

void test_App_GetMotorPosition_mid_pos_value(void) {
  gs_enc_cnt = 412;
  TEST_ASSERT_EQUAL_INT32(3708, App_GetPosition());
}

void test_App_GetMotorPosition_max_pos_value(void) {
  gs_enc_cnt = INT16_MAX;
  TEST_ASSERT_EQUAL_INT32(294903, App_GetPosition());
}

void test_App_GetMotorPosition_mid_neg_value(void) {
  gs_enc_cnt = -3000; 
  TEST_ASSERT_EQUAL_INT32(-27000, App_GetPosition());
}

void test_App_GetMotorPosition_max_neg_value(void) {
  gs_enc_cnt = -30000;
  TEST_ASSERT_EQUAL_INT32(-270000, App_GetPosition());
}

void test_App_ArmPhase_valid(void) {
  App_ArmPhase(GpioChA_E);
  TEST_ASSERT_EQUAL_UINT16(GpioChA_E, gs_gpio_pin);
  App_ArmPhase(GpioChB_E);
  TEST_ASSERT_EQUAL_UINT16(GpioChB_E, gs_gpio_pin);
  App_ArmPhase(GpioChC_E);
  TEST_ASSERT_EQUAL_UINT16(GpioChC_E, gs_gpio_pin);
}

void test_App_ArmPhase_invalid(void) {
  gs_gpio_pin = 200;
  /* gs_gpio_pin is unmodified because function rejects
     invalid phase and returns without attempting write. */
  App_ArmPhase(100);
  TEST_ASSERT_EQUAL_UINT16(200, gs_gpio_pin);
}

void test_App_DisarmPhase_valid(void) {
  gs_gpio_pin = 100;
  App_DisarmPhase(GpioChA_E);
  TEST_ASSERT_EQUAL_UINT16(GpioChA_E, gs_gpio_pin);
  App_DisarmPhase(GpioChB_E);
  TEST_ASSERT_EQUAL_UINT16(GpioChB_E, gs_gpio_pin);
  App_DisarmPhase(GpioChC_E);
  TEST_ASSERT_EQUAL_UINT16(GpioChC_E, gs_gpio_pin);
}

void test_App_DisarmPhase_invalid(void) {
  gs_gpio_pin = 199;
  App_DisarmPhase(100);
  TEST_ASSERT_EQUAL_UINT16(199, gs_gpio_pin);
}

void test_App_ArmDrive(void) {
  gs_gpio_pin = 200;
  App_ArmDrive();
  TEST_ASSERT_EQUAL_UINT16(gs_gpio_pin, GpioChBkIn2_E);
}

void test_App_DisarmDrive(void) {
  gs_gpio_pin = 200;
  App_DisarmDrive();
  TEST_ASSERT_EQUAL_UINT16(gs_gpio_pin, GpioChBkIn2_E);
}


/*-----------------------------------------------------------------------------
 * uCmd: command line handler tests. 
 *-----------------------------------------------------------------------------*/

uint8_t chr_rx_not_attached;                    /* Holds rx char when callback
                                                   is not attached. */

void chr_rx_callback_not_attached(void* params) {
  chr_rx_not_attached = *((uint8_t*)params);
}

void test_uCmd_Init_function(void) {
  chr_rx_not_attached = 0xff;
  MockUART_ChrRxCallback = chr_rx_callback_not_attached;
  MockUART_SendChar('a'); 
  TEST_ASSERT_EQUAL_UINT8('a', chr_rx_not_attached);
  uCmd_Init(&MockUART_ChrRxCallback);
  /* Check buffer is empty. */
  TEST_ASSERT_TRUE(uCmd_BuffIsEmpty());
  /* Check count is zero. */
  TEST_ASSERT_EQUAL_UINT8(0, uCmd_GetCnt());
  /* Check buffer is not full. */
  TEST_ASSERT_FALSE(uCmd_BuffIsFull());
  /* Check buffer is initialized to zero. */
  /* Set a non-zero value prior to getting buffer to verify */
  /* it was, indeed, set to zero. */
  test_buff[0] = 'a';
  uCmd_GetBuff(test_buff);
  TEST_ASSERT_EACH_EQUAL_UINT8(0, test_buff, sizeof(test_buff));
  MockUART_SendChar('b');
  /* Check value is not modified now that callback is attached. */
  TEST_ASSERT_EQUAL_UINT8('a', chr_rx_not_attached);
}

void test_uCmd_NewCharCallback_add_chars(void) {
  uCmd_Init(&MockUART_ChrRxCallback);
  /* Check character count is zero. */
  TEST_ASSERT_EQUAL_UINT8(0, uCmd_GetCnt());
  MockUART_SendChar('a');
  /* Check character count increases. */
  TEST_ASSERT_EQUAL_UINT8(1, uCmd_GetCnt());
  /* Check first element is character 'a'. */
  uCmd_GetBuff(test_buff);
  TEST_ASSERT_EQUAL_UINT8('a', test_buff[0]);
  /* Add another char. Check count increases. */
  MockUART_SendChar('b');
  uCmd_GetBuff(test_buff);
  TEST_ASSERT_EQUAL_UINT8('a', test_buff[0]);
  TEST_ASSERT_EQUAL_UINT8('b', test_buff[1]);
  TEST_ASSERT_EQUAL_UINT8(2, uCmd_GetCnt());
}

void test_uCmd_BufferIsFull_flag(void) {
  uint8_t idx = 0;
  uCmd_Init(&MockUART_ChrRxCallback);
  for(; idx < uCMD_BUFF_SIZE; idx++) {
    test_buff[idx] = idx;
    TEST_ASSERT_FALSE(uCmd_BuffIsFull());
    MockUART_SendChar(idx);
  }
  /* Check full flag is set after filling the buffer. */
  TEST_ASSERT_TRUE(uCmd_BuffIsFull());
  /* Check count does not increment because of buffer full. */
  idx = uCmd_GetCnt();
  MockUART_SendChar('a');
  TEST_ASSERT_EQUAL_UINT8(idx, uCmd_GetCnt());
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ADC_read_value);
  RUN_TEST(test_ADC_read_value_out_of_range_channel);
  RUN_TEST(test_ADC_get_voltage_zero);
  RUN_TEST(test_ADC_get_voltage_out_of_range_channel);
  RUN_TEST(test_ADC_get_voltage_max);
  RUN_TEST(test_ADC_get_voltage_mid_range);
  RUN_TEST(test_ADC_get_voltage_bias);
  RUN_TEST(test_ADC_get_current_zero);
  RUN_TEST(test_ADC_get_current_out_of_range_channel);
  RUN_TEST(test_ADC_get_current_zero_bias);
  RUN_TEST(test_ADC_get_current_max);
  RUN_TEST(test_ADC_get_current_min);
  RUN_TEST(test_ADC_get_current_channels);
  RUN_TEST(test_App_SetPwmDutyCycle_value);
  RUN_TEST(test_App_SetPwmDutyCycle_out_of_range_channel);
  RUN_TEST(test_App_SetPwmDutyCycle_out_of_range_value);
  RUN_TEST(test_App_SetPwmVoltage_zero_value);
  RUN_TEST(test_App_SetPwmVoltage_mid_range_value);
  RUN_TEST(test_App_SetPwmVoltage_max_value);
  RUN_TEST(test_APP_SetPwmVoltage_out_of_range_value);
  RUN_TEST(test_App_GetPwmDutyCycle_zero);
  RUN_TEST(test_App_GetPwmDutyCycle_out_of_range_channel);
  RUN_TEST(test_App_GetPwmDutyCycle_max_counter);
  RUN_TEST(test_App_GetPwmDutyCycle_mid_range_value);
  RUN_TEST(test_App_GetPwmVoltage_zero_value);
  RUN_TEST(test_App_GetPwmVoltage_out_of_range_channel);
  RUN_TEST(test_App_GetPwmVoltage_mid_range_value);
  RUN_TEST(test_App_GetBusVoltage_zero_value);
  RUN_TEST(test_App_GetBusVoltage_mid_range_value);
  RUN_TEST(test_App_GetTemp_zero_buffer);
  RUN_TEST(test_App_GetTemp_mid_buffer);
  RUN_TEST(test_App_GetTemp_max_buffer);
  RUN_TEST(test_App_GetMotorPosition_zero_value);
  RUN_TEST(test_App_GetMotorPosition_mid_pos_value);
  RUN_TEST(test_App_GetMotorPosition_max_pos_value);
  RUN_TEST(test_App_GetMotorPosition_mid_neg_value);
  RUN_TEST(test_App_GetMotorPosition_max_neg_value);
  RUN_TEST(test_App_ArmPhase_valid);
  RUN_TEST(test_App_ArmPhase_invalid);
  RUN_TEST(test_App_DisarmPhase_valid);
  RUN_TEST(test_App_DisarmPhase_invalid);
  RUN_TEST(test_App_ArmDrive);
  RUN_TEST(test_App_DisarmDrive);
  RUN_TEST(test_uCmd_Init_function);
  RUN_TEST(test_uCmd_NewCharCallback_add_chars);
  RUN_TEST(test_uCmd_BufferIsFull_flag);
  return UNITY_END();
}
