/**
  ******************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   ���ݴ�����-��������
  ******************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� STM32H743������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************
  */  
#include "stm32h7xx.h"
#include "main.h"
#include "./led/bsp_led.h" 
#include "./usart/bsp_usart.h"
#include "./sdram/bsp_sdram.h" 
#include "./lcd/bsp_lcd.h"
#include "./touch/gt9xx.h"
#include "./touch/palette.h"
#include "./delay/core_delay.h"   
void Delay(__IO uint32_t nCount); 

#include "./porting/lv_port_disp.h"
#include "./porting/lv_port_indev.h"
#include "lvgl.h"
#include "gui_guider.h"
#include "events_init.h"
#include "./touch/gt9xx.h"
#include "tim.h"

lv_ui  guider_ui; 
void LCD_Test(void);

void esp_ap_init()
{
  uint8_t at0[] = "ATE0\r\n";
	uint8_t at1[] = "AT+CWMODE=2\r\n";
	uint8_t at2[] = "AT+RST\r\n";
	uint8_t at3[] = "AT+CWSAP=\"esp8266\",\"66666666\",1,3\r\n";
	uint8_t at4[] = "AT+CIPMUX=1\r\n";
	uint8_t at5[] = "AT+CIPSERVER=1,8086\r\n";
  HAL_UART_Transmit(&UartHandle,at0,sizeof(at0),100);
	HAL_Delay(500);
	HAL_UART_Transmit(&UartHandle,at1,sizeof(at1),100);
	HAL_Delay(1000);
	HAL_UART_Transmit(&UartHandle,at2,sizeof(at2),100);
	HAL_Delay(1000);
	HAL_UART_Transmit(&UartHandle,at3,sizeof(at3),100);
	HAL_Delay(1000);
	HAL_UART_Transmit(&UartHandle,at4,sizeof(at4),100);
	HAL_Delay(1000);
	HAL_UART_Transmit(&UartHandle,at5,sizeof(at5),100);
}

/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{   
	HAL_Init();
	/* ϵͳʱ�ӳ�ʼ����400MHz */
	SystemClock_Config();
	/* LED �˿ڳ�ʼ�� */
	LED_GPIO_Config();
	/* ���ô���1Ϊ��115200 8-N-1 */
	UARTx_Config();		
	esp_ap_init();
	//MX_USART3_UART_Init();
	//MX_TIM6_Init(); // 初�?�化TIM6
	lv_init();
	lv_port_disp_init();
	lv_port_indev_init(); 
	setup_ui(&guider_ui);
	events_init(&guider_ui);
	LED1_ON;
	//HAL_UART_Receive_IT(&UartHandle, &rx_buffer[0], 1); // ��������
  //Delay(0xfff);
	while(1)
	{		
			
//			static int led_flag = 0;
			//printf("1\n");
			lv_task_handler();
			lv_tick_inc(1); 
			//lv_tick_inc(1);
//			if(led_flag>=250000)
//			{
//				//printf("isTouch: %d\n", touch_active);
//				led_flag=0;
//				LED1_TOGGLE;
//			}
//			led_flag++;
	}
}

/**
  * @brief  System Clock ����
  *         system Clock ��������: 
	*            System Clock source  = PLL (HSE)
	*            SYSCLK(Hz)           = 400000000 (CPU Clock)
	*            HCLK(Hz)             = 200000000 (AXI and AHBs Clock)
	*            AHB Prescaler        = 2
	*            D1 APB3 Prescaler    = 2 (APB3 Clock  100MHz)
	*            D2 APB1 Prescaler    = 2 (APB1 Clock  100MHz)
	*            D2 APB2 Prescaler    = 2 (APB2 Clock  100MHz)
	*            D3 APB4 Prescaler    = 2 (APB4 Clock  100MHz)
	*            HSE Frequency(Hz)    = 25000000
	*            PLL_M                = 5
	*            PLL_N                = 160
	*            PLL_P                = 2
	*            PLL_Q                = 4
	*            PLL_R                = 2
	*            VDD(V)               = 3.3
	*            Flash Latency(WS)    = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;
  
  /*ʹ�ܹ������ø��� */
  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

  /* ��������ʱ��Ƶ�ʵ������ϵͳƵ��ʱ����ѹ���ڿ����Ż����ģ�?
		 ����ϵͳƵ�ʵĵ�ѹ����ֵ�ĸ��¿��Բο���Ʒ�����ֲᡣ  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
 
  /* ����HSE������ʹ��HSE��ΪԴ����PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
 
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
	/* ѡ��PLL��Ϊϵͳʱ��Դ����������ʱ�ӷ�Ƶ�� */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK  | \
																 RCC_CLOCKTYPE_HCLK    | \
																 RCC_CLOCKTYPE_D1PCLK1 | \
																 RCC_CLOCKTYPE_PCLK1   | \
                                 RCC_CLOCKTYPE_PCLK2   | \
																 RCC_CLOCKTYPE_D3PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}
void MX_TIM6_Init(void) {
  TIM_HandleTypeDef htim6;
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  __HAL_RCC_TIM6_CLK_ENABLE();

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = (SystemCoreClock / 1000000) - 1; // 1 MHz
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 1000 - 1; // 1 ms
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim6);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim6, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);

  HAL_TIM_Base_Start_IT(&htim6);
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//	LED1_TOGGLE;
//  if (htim->Instance == TIM6) {
//		lv_tick_inc(1); // 调用lv_tick_inc
//  }
//}
void Delay(__IO uint32_t nCount)	 //�򵥵���ʱ����
{
	for(; nCount != 0; nCount--);
}

/****************************END OF FILE***************************/
