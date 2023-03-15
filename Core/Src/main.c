/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file           : main.c
* @brief          : Main program body
******************************************************************************
* @attention
*
* Copyright (c) 2023 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CAPTURE_CNT 30
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

TIM_HandleTypeDef htim2;
DMA_HandleTypeDef hdma_tim2_ch1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t g_iUartRx;
uint32_t g_iCh1CapturedValue[CAPTURE_CNT] = {0};
uint32_t g_iCh1Diff[CAPTURE_CNT / 2] = {0};
int g_iCaptureFinishedFlag = 0;
int g_iTIM2Mode = 0;
double g_dCh1Frequency;
int g_iIntegralCnt = 0;
uint32_t g_iOutPeriod = 0;
int g_iTIM2Cnt = 2000;
int g_iTIM2Error = 0;
int g_iErrorFlag = 0;
int g_iTestMode = 0;
int g_iTestCnt = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
static void IC_TIM2_Init(void);
static void OP_TIM2_Init(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
* @brief  The application entry point.
* @retval int
*/
int main(void)
{ 
    /* USER CODE BEGIN 1 */
    
    /* USER CODE END 1 */
    
    /* MPU Configuration--------------------------------------------------------*/
    MPU_Config();
    
    /* MCU Configuration--------------------------------------------------------*/
    
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    
    /* USER CODE BEGIN Init */
    
    /* USER CODE END Init */
    
    /* Configure the system clock */
    SystemClock_Config();
    
    /* USER CODE BEGIN SysInit */
    
    /* USER CODE END SysInit */
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    //    MX_TIM2_Init();
    MX_USART1_UART_Init();
    /* USER CODE BEGIN 2 */
    char buf[256];
    /* USER CODE END 2 */
    
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        
        /* USER CODE BEGIN 3 */
        
        HAL_UART_Receive(&huart1, &g_iUartRx, sizeof(g_iUartRx), 10);
        if(g_iUartRx == 'b')
            g_iTIM2Error = 0;
        if(g_iUartRx == 'a')
        {
//            if(g_iTIM2Cnt != 2000)
//            {
//                g_iErrorFlag = 1;
//                g_iTIM2Error++;
//                sprintf(buf, "%.1f %d %d %d, %d ERR %d\r\n", g_dCh1Frequency, g_iCh1CapturedValue[CAPTURE_CNT - 1], g_iCh1CapturedValue[CAPTURE_CNT - 2], g_iIntegralCnt, g_iTIM2Cnt, g_iTIM2Error);
//                HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 100);
//            }
            
            
            if(g_iTestMode == 0)
            {
                if(g_iTIM2Cnt != 800)
                    g_iErrorFlag = 1;
            }
            else
            {
                if(g_iTIM2Cnt != 400)
                    g_iErrorFlag = 1;
            }
            
            g_iTestCnt++;
            if(g_iTestCnt > 10)
            {
                if(g_iTestMode == 0)
                    g_iTestMode = 1;
                else
                    g_iTestMode = 0;
                g_iTestCnt = 0;
            }
            
            if(g_iErrorFlag)
            {
                g_iTIM2Error++;
                sprintf(buf, "%.1f %d %d %d, %d ERR %d\r\n", g_dCh1Frequency, g_iCh1CapturedValue[CAPTURE_CNT - 1], g_iCh1CapturedValue[CAPTURE_CNT - 2], g_iIntegralCnt, g_iTIM2Cnt, g_iTIM2Error);
                HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 100);
                g_iErrorFlag = 0;
            }
            
            IC_TIM2_Init();
            g_iTIM2Mode = 0;
            g_iTIM2Cnt = 0;
            HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)g_iCh1CapturedValue, CAPTURE_CNT);
            g_iUartRx = 0;
        }
        if(g_iCaptureFinishedFlag)
        {
            g_dCh1Frequency = 0;
            //            for(int i = 0; i < CAPTURE_CNT; i = i + 2)
            //                g_dCh1Frequency += (double)(g_iCh1CapturedValue[i + 1] - g_iCh1CapturedValue[i]);
            //            g_dCh1Frequency = g_dCh1Frequency / (double)(CAPTURE_CNT / 2);
            for(int i = 0; i < CAPTURE_CNT; i = i + 2)
                g_iCh1Diff[i/2] = g_iCh1CapturedValue[i + 1] - g_iCh1CapturedValue[i];
            
            int n = CAPTURE_CNT / 2;
            int i, j, mode, freq, count = 1;
            for(i = 0; i < n; i++)
            {
                freq = 1;
                
                for(j = i + 1; j < n; j++)
                {
                    if(g_iCh1Diff[i] == g_iCh1Diff[j])
                        freq += 1;
                }
                if(freq >= count)
                {
                    mode = g_iCh1Diff[i];
                    count = freq;
                }
            }
            
            g_dCh1Frequency = (double)mode;
            
            
//            g_dCh1Frequency = (double)(g_iCh1CapturedValue[CAPTURE_CNT - 1] - g_iCh1CapturedValue[CAPTURE_CNT - 2]);
            g_dCh1Frequency = (HAL_RCC_GetPCLK1Freq() * 2) / g_dCh1Frequency;
            if(g_iTestMode == 0)
                g_iIntegralCnt = (int)floor(0.01 * g_dCh1Frequency + 0.5);
            else
                g_iIntegralCnt = (int)floor(0.005 * g_dCh1Frequency + 0.5);
            //            sprintf(buf, "Freq : %.1f, In_Cnt : %d\r\n", g_dCh1Frequency, g_iIntegralCnt);
            //            HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 100);
            g_iCaptureFinishedFlag = 0;
            g_iTIM2Mode = 1;
            g_iOutPeriod = ((HAL_RCC_GetPCLK1Freq() * 2) / (int)g_dCh1Frequency);
            OP_TIM2_Init();
            HAL_Delay(100);
            HAL_TIM_OnePulse_Start_IT(&htim2, TIM_CHANNEL_2);
        }
    }
    /* USER CODE END 3 */
}

/**
* @brief System Clock Configuration
* @retval None
*/
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /** Supply configuration update enable
    */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    
    /** Configure the main internal regulator output voltage
    */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
    
    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_0;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    
    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
        |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
            |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
* @brief TIM2 Initialization Function
* @param None
* @retval None
*/
static void MX_TIM2_Init(void)
{
    
    /* USER CODE BEGIN TIM2_Init 0 */
    
    /* USER CODE END TIM2_Init 0 */
    
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    /* USER CODE BEGIN TIM2_Init 1 */
    
    /* USER CODE END TIM2_Init 1 */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 4294967295;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_OnePulse_Init(&htim2, TIM_OPMODE_SINGLE) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM2_Init 2 */
    
    /* USER CODE END TIM2_Init 2 */
    HAL_TIM_MspPostInit(&htim2);
    
}

/**
* @brief USART1 Initialization Function
* @param None
* @retval None
*/
static void MX_USART1_UART_Init(void)
{
    
    /* USER CODE BEGIN USART1_Init 0 */
    
    /* USER CODE END USART1_Init 0 */
    
    /* USER CODE BEGIN USART1_Init 1 */
    
    /* USER CODE END USART1_Init 1 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART1_Init 2 */
    
    /* USER CODE END USART1_Init 2 */
    
}

/**
* Enable DMA controller clock
*/
static void MX_DMA_Init(void)
{
    
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    /* DMA interrupt init */
    /* DMA1_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    
}

/**
* @brief GPIO Initialization Function
* @param None
* @retval None
*/
static void MX_GPIO_Init(void)
{
    
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    
}

/* USER CODE BEGIN 4 */
static void IC_TIM2_Init(void)
{
    
    /* USER CODE BEGIN TIM2_Init 0 */
    
    /* USER CODE END TIM2_Init 0 */
    
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};
    
    /* USER CODE BEGIN TIM2_Init 1 */
    
    /* USER CODE END TIM2_Init 1 */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
//    htim2.Init.Prescaler = 9;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 4294967295;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    htim2.State = HAL_TIM_STATE_RESET;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM2_Init 2 */
    
    /* USER CODE END TIM2_Init 2 */
    
}

static void OP_TIM2_Init(void)
{
    
    /* USER CODE BEGIN TIM2_Init 0 */
    
    /* USER CODE END TIM2_Init 0 */
    
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_OnePulse_InitTypeDef sConfig;
    
    /* USER CODE BEGIN TIM2_Init 1 */
    
    /* USER CODE END TIM2_Init 1 */
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    //    htim2.Init.Period = 20000;
    htim2.Init.Period = ((g_iOutPeriod - 1) - 1);
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    htim2.State = HAL_TIM_STATE_RESET;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_OnePulse_Init(&htim2, TIM_OPMODE_SINGLE) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM2_Init 2 */
    sConfig.OCMode = TIM_OCMODE_PWM1;
    //    sConfig.Pulse = 10000;
    sConfig.Pulse = (g_iOutPeriod - 1) / 2;
    sConfig.OCPolarity = TIM_OCPOLARITY_LOW;
    
    sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    //    sConfig.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfig.ICFilter = 0;
    
    HAL_TIM_OnePulse_ConfigChannel(&htim2, &sConfig, TIM_CHANNEL_2, TIM_CHANNEL_1);
    /* USER CODE END TIM2_Init 2 */
    HAL_TIM_MspPostInit(&htim2);
    
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    HAL_UART_Receive_IT(&huart1, &g_iUartRx, sizeof(g_iUartRx));
//}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
        {
            if(g_iTIM2Mode == 0)
                g_iCaptureFinishedFlag = 1;
            else
            {
                if(g_iTIM2Cnt == g_iIntegralCnt)
                    HAL_TIM_OnePulse_Stop_IT(&htim2, TIM_CHANNEL_2);
            }
            
        }
    }
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    if(htim->Instance == TIM2)
//    {
//        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
//        {
//            if(g_iTIM2Mode == 1)
//                g_iTIM2Cnt++;
//            if(g_iTIM2Cnt == g_iIntegralCnt)
//            {
//                HAL_TIM_OnePulse_Stop(&htim2, TIM_CHANNEL_2);
//                g_iTIM2Cnt = 0;
//            }
//        }
//    }
//}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
        {
            if(g_iTIM2Mode == 1)
                g_iTIM2Cnt++;
            //            if(g_iTIM2Cnt == g_iIntegralCnt)
            //            {
            //                HAL_TIM_OnePulse_Stop_IT(&htim2, TIM_CHANNEL_2);
            //            }
        }
    }
}
/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};
    
    /* Disables the MPU */
    HAL_MPU_Disable();
    
    /** Initializes and configures the Region and the memory to be protected
    */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress = 0x0;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    /* Enables the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    
}

/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
