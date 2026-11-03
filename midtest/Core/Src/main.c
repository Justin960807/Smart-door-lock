/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : TIM2 硬體 PWM 完美呼吸燈版 (修復時鐘未開啟問題)
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include <string.h>

UART_HandleTypeDef hlpuart1;
TIM_HandleTypeDef htim2;

volatile uint8_t tamper_alarm = 0;
volatile uint8_t unlock_flag = 0;

uint8_t rx_data;
char rx_buffer[32];
uint8_t rx_index = 0;

uint32_t blink_timer = 0;
uint32_t pwm_timer = 0;
uint32_t hold_timer = 0;
uint32_t pwm_value = 0;
uint8_t motor_state = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_TIM2_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_LPUART1_UART_Init();
  MX_TIM2_Init();

  // 啟動 TIM2 Channel 1 PWM 輸出 (PA0 外接綠燈)
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  TIM2->CCR1 = 0; // 初始全暗

  HAL_UART_Receive_IT(&hlpuart1, &rx_data, 1);

  while (1)
  {
    // 🚨 防拆警報
    if (tamper_alarm == 1)
    {
        TIM2->CCR1 = 0; // 強制關閉綠燈
        if (HAL_GetTick() - blink_timer >= 50)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); // PA6 紅燈狂閃
            blink_timer = HAL_GetTick();
        }
    }
    // 🟢 正常解鎖呼吸燈邏輯
    else
    {
        if (unlock_flag == 1)
        {
            motor_state = 1;
            unlock_flag = 0;
        }

        if (motor_state == 1) // 漸亮 (約 2 秒)
        {
            if (HAL_GetTick() - pwm_timer >= 2)
            {
                pwm_value += 20;
                if (pwm_value >= 19999)
                {
                    pwm_value = 19999;
                    motor_state = 2;
                    hold_timer = HAL_GetTick();
                }
                TIM2->CCR1 = pwm_value;
                pwm_timer = HAL_GetTick();
            }
        }
        else if (motor_state == 2) // 全亮維持 5 秒
        {
            if (HAL_GetTick() - hold_timer >= 5000)
            {
                motor_state = 3;
            }
        }
        else if (motor_state == 3) // 漸暗 (約 2 秒)
        {
            if (HAL_GetTick() - pwm_timer >= 2)
            {
                if (pwm_value <= 20) pwm_value = 0;
                else pwm_value -= 20;

                if (pwm_value == 0)
                {
                    motor_state = 0;
                }
                TIM2->CCR1 = pwm_value;
                pwm_timer = HAL_GetTick();
            }
        }
    }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
}

static void MX_TIM2_Init(void)
{
  // 🌟 最關鍵的一行：開啟 TIM2 時鐘電源，否則 PWM 永遠出不來！
  __HAL_RCC_TIM2_CLK_ENABLE();

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 169;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
  HAL_TIM_PWM_Init(&htim2);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);

  // 🌟 將 PA0 綁定為 TIM2_CH1 的硬體 PWM 輸出
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void MX_LPUART1_UART_Init(void)
{
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&hlpuart1);
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == LPUART1)
    {
        if (rx_data == '\n' || rx_data == '\r')
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0';

                if (strstr(rx_buffer, "RESET_ALARM") != NULL)
                {
                    tamper_alarm = 0;
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
                }
                else if (strstr(rx_buffer, "1234_OPEN") != NULL && tamper_alarm == 0)
                {
                    unlock_flag = 1;
                }

                memset(rx_buffer, 0, sizeof(rx_buffer));
                rx_index = 0;
            }
        }
        else
        {
            rx_buffer[rx_index++] = rx_data;
            if (rx_index >= 31) rx_index = 0;
        }

        HAL_UART_Receive_IT(&hlpuart1, &rx_data, 1);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {
        tamper_alarm = 1;
        motor_state = 0;
        pwm_value = 0;
    }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1){}
}
