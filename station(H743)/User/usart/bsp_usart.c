/**
  ******************************************************************************
  * @file    bsp_debug_usart.c
  * @author  fire
  * @version V1.0
  * @date    2016-xx-xx
  * @brief      使用串口2重定向c库printf函数到usart端口
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 H743 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */ 
#include "./led/bsp_led.h"  
#include "./usart/bsp_usart.h"
#include "custom.h"

UART_HandleTypeDef UartHandle;
UART_HandleTypeDef Uart3Handle; // UART3句柄
//UART_HandleTypeDef huart3;
//uint8_t rx_buffer[1];
//uint8_t rx_index;
 /**
  * @brief  USARTx GPIO 配置,波特率模式设置为115200 8-N-1
  * @param  无
  * @retval 无
  */  
void UARTx_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;
			
	UARTx_RX_GPIO_CLK_ENABLE();
	UARTx_TX_GPIO_CLK_ENABLE();
	
	/* 配置串口1时钟源*/
	RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	RCC_PeriphClkInit.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);
	/* 使能 UART 时钟 */
	UARTx_CLK_ENABLE();

	/**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10    ------> USART1_RX 
	*/
	/* 配置Tx引脚为复用功能  */
	GPIO_InitStruct.Pin = UARTx_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = UARTx_TX_AF;
	HAL_GPIO_Init(UARTx_TX_GPIO_PORT, &GPIO_InitStruct);
	
	/* 配置Rx引脚为复用功能 */
	GPIO_InitStruct.Pin = UARTx_RX_PIN;
	GPIO_InitStruct.Alternate = UARTx_RX_AF;
	HAL_GPIO_Init(UARTx_RX_GPIO_PORT, &GPIO_InitStruct); 
	
	/* 配置USARTx 参数 */
	UartHandle.Instance = UARTx;
	UartHandle.Init.BaudRate = 115200;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits = UART_STOPBITS_1;
	UartHandle.Init.Parity = UART_PARITY_NONE;
	UartHandle.Init.Mode = UART_MODE_TX_RX;
	HAL_UART_Init(&UartHandle);
	
	__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE);  //receive interrupt
	__HAL_UART_ENABLE_IT(&UartHandle, UART_IT_IDLE);  //idle interrupt
	HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

}

void UART3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

	__HAL_RCC_GPIOB_CLK_ENABLE(); // 启用GPIOB时钟
	__HAL_RCC_USART3_CLK_ENABLE(); // 启用USART3时钟

	/* 配置USART3时钟源 */
	RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
	RCC_PeriphClkInit.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
	HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

	/**USART3 GPIO Configuration    
	PB10     ------> USART3_TX
	PB11     ------> USART3_RX 
	*/
	/* 配置Tx引脚为复用功能 */
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* 配置Rx引脚为复用功能 */
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* 配置USART3参数 */
	Uart3Handle.Instance = USART3;
	Uart3Handle.Init.BaudRate = 115200;
	Uart3Handle.Init.WordLength = UART_WORDLENGTH_8B;
	Uart3Handle.Init.StopBits = UART_STOPBITS_1;
	Uart3Handle.Init.Parity = UART_PARITY_NONE;
	Uart3Handle.Init.Mode = UART_MODE_TX_RX;
	HAL_UART_Init(&Uart3Handle);

	/* 配置中断 */
	__HAL_UART_ENABLE_IT(&Uart3Handle, UART_IT_RXNE);  // 接收中断
	__HAL_UART_ENABLE_IT(&Uart3Handle, UART_IT_IDLE);  // 空闲中断
//	HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
//	HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void USART1_IRQHandler(void)
{
	uint8_t rx_data;
	static uint8_t pdata[128];
	static uint16_t data_index = 0;
	if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE))
	{
		// 接收数据
		HAL_UART_Receive(&UartHandle, &rx_data, 1, 0xFFFF);
		pdata[data_index++] = rx_data;
		__HAL_UART_CLEAR_OREFLAG(&UartHandle);

	}

	if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_IDLE))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&UartHandle);
		LED1_TOGGLE;
		char temp[4];
		for(int i = 0; i < data_index-2; i++){
				if(pdata[i] < 0x0A && pdata[i+1] < 0x0A && pdata[i+2] < 0x0A && (pdata[i] > 0x00 && pdata[i+1] > 0x00 && pdata[i+2] > 0x00)){
						temp[0] = pdata[i];
						temp[1] = pdata[i+1];
						temp[2] = pdata[i+2];
						temp[3] = '\0';
						const char *showed_data = temp;
						renovate_the_num(showed_data);
				}	
		}
		memset(pdata, 0, sizeof(pdata));
    data_index = 0;
	}
}

///重定向c库函数scanf到USARTx,支持scanf和getchar等函数
//int fgetc(FILE *f)
//{	
//	int ch;
//	/* 等待串口接收数据 */
//	while(__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE) == RESET);
//	__HAL_UART_CLEAR_OREFLAG(&UartHandle);
//	HAL_UART_Receive(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
//	return (ch);
//}
/*********************************************END OF FILE**********************/
