#include "usart.h"
#include "bsp_uart.h"
#include "bsp_pwm.h"

uint8_t esp_data[256];
uint8_t drone_data[128];
uint8_t openmv_data[128];
int esp_data_index=0;
int drone_data_index=0;
int openmv_data_index=0;
uint8_t uart6_rx_con=0;
uint8_t uart6_rx_buf[128];
PositionTypeDef position_data[64];
PositionTypeDef current_position;

void send_height(uint16_t height)
{
		uint8_t send_data[8];
    send_data[0] = 0xAA; // 帧头1
		send_data[1] = 0x55; // 帧头2
		send_data[2] = 0x01; // 指令位 0x01:高度 0x02:X、Y  0x04:路径规划 0x05:openmv辅助降落信息 
    send_data[3] = (height >> 8) & 0xff; // 数据位1
    send_data[4] = height & 0xff; // 数据位2
		send_data[5] = 0x00; // 数据位3
		send_data[6] = 0x00; // 数据位4
    send_data[7] = 0x5D; // 帧尾
    HAL_UART_Transmit(&huart2, send_data, 8, 0xffff);
}
void send_xy(int16_t x, int16_t y)
{
		uint8_t send_data[8];
    send_data[0] = 0xAA; // 帧头1
		send_data[1] = 0x55; // 帧头2
		send_data[2] = 0x02; // 指令位 0x01:高度 0x02:X、Y  0x04:路径规划 0x05:openmv辅助降落信息 
    send_data[3] = (x >> 8) & 0xff; // 数据位1
    send_data[4] = x & 0xff; // 数据位2
		send_data[5] = (y >> 8) & 0xff; // 数据位3
		send_data[6] = y & 0xff; // 数据位4
    send_data[7] = 0x5D; // 帧尾
    HAL_UART_Transmit(&huart2, send_data, 8, 0xffff);
}
void send_obstacle(uint16_t angle, uint16_t dist)
{
		uint8_t send_data[8];
    send_data[0] = 0xAA; // 帧头1
		send_data[1] = 0x55; // 帧头2
		send_data[2] = 0x08; // 指令位 0x08:雷达避障信息(最近障碍物角度和距离)
    send_data[3] = (angle >> 8) & 0xff; // 数据位1
    send_data[4] = angle & 0xff; // 数据位2
		send_data[5] = (dist >> 8) & 0xff; // 数据位3
		send_data[6] = dist & 0xff; // 数据位4
    send_data[7] = 0x5D; // 帧尾
    HAL_UART_Transmit(&huart2, send_data, 8, 0xffff);
}
void send_lidar_data(LidarDataTypeDef *data)
{
		uint8_t send_data[8];
    send_data[0] = 0xAA; // 帧头1
		send_data[1] = 0x55; // 帧头2
		send_data[2] = 0x06; // 指令位 0x06:雷达避障信息(0°和90°) 0x07:雷达避障信息(180°和270°)
    send_data[3] = (data[0].distance >> 8) & 0xff; // 数据位1
    send_data[4] = data[0].distance & 0xff; // 数据位2
		send_data[5] = (data[1].distance >> 8) & 0xff; // 数据位3
		send_data[6] = data[1].distance & 0xff; // 数据位4
    send_data[7] = 0x5D; // 帧尾
    HAL_UART_Transmit(&huart2, send_data, 8, 0xffff);
	
		send_data[2] = 0x07; 
    send_data[3] = (data[2].distance >> 8) & 0xff; // 数据位1
    send_data[4] = data[2].distance & 0xff; // 数据位2
		send_data[5] = (data[3].distance >> 8) & 0xff; // 数据位3
		send_data[6] = data[3].distance & 0xff; // 数据位4
		HAL_UART_Transmit(&huart2, send_data, 8, 0xffff);
}
// ESP8266  TX:A9 RX:A10   
void USART1_IRQHandler(void)
{
	volatile uint8_t receive;
	//receive interrupt �����ж�
	if(huart1.Instance->SR & UART_FLAG_RXNE)
	{
			receive = huart1.Instance->DR;
			
			esp_data[esp_data_index++]=receive;
		
	}
	//idle interrupt �����ж�
	else if(huart1.Instance->SR & UART_FLAG_IDLE)
	{
			receive = huart1.Instance->DR;
			if(esp_data[2] == '+' && esp_data[3] == 'I' && esp_data[11] == 0xAA && esp_data[12] == 0x55 && esp_data[138] == 0x5D)
			{
				uint8_t step = esp_data[13];
				uint8_t transform_data[8];
				for(int i = 0; i < step; i++)
				{
						position_data[i].x = esp_data[i*2 + 14];
						position_data[i].y = esp_data[i*2 + 15];
						if(i > 0)
						{
							uint16_t ang = 0;
							int dis = position_data[i].x - position_data[i-1].x;
							if(dis > 0) ang = 180;
							else if(dis < 0){ ang = 0; dis=-dis;}
							else{
									dis = position_data[i].y - position_data[i-1].y;
									if(dis > 0) ang = 270;
									else{ ang = 90; dis=-dis;}
							}
							transform_data[0] = 0xAA;
							transform_data[1] = 0x55;
							transform_data[2] = 0x04;
							transform_data[3] = (i - 1) & 0xff;
							transform_data[4] = 0x32;
							transform_data[5] = (ang >> 8) & 0xff;
							transform_data[6] = ang & 0xff;
							transform_data[7] = 0x5D;
							HAL_UART_Transmit(&huart2,transform_data,sizeof(transform_data),100);
						}
				}
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
			}
			for(int i=0;i<=esp_data_index;i++)
				esp_data[i]=0;
			esp_data_index=0;
	}
}

// Drone  TX:A2 RX:A3
void USART2_IRQHandler(void)
{
	volatile uint8_t receive;
	//receive interrupt �����ж�
	if(huart2.Instance->SR & UART_FLAG_RXNE)
	{
			receive = huart2.Instance->DR;
			drone_data[drone_data_index++]=receive;
		
	}
	//idle interrupt �����ж�
	else if(huart2.Instance->SR & UART_FLAG_IDLE)
	{
			receive = huart2.Instance->DR;
			if(drone_data[0] == 0xAA && drone_data[1] == 0x55 && drone_data[3] == 0x5D)
			{
					if(drone_data[2] != 0xff)
					{
							current_position.x = position_data[drone_data[2]].x;
							current_position.y = position_data[drone_data[2]].y;
					}
					else 
							HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
			}
	}
}

//OPENMV  TX:B10 RX:B11
void USART3_IRQHandler(void)
{
	volatile uint8_t receive;
	//receive interrupt �����ж�
	if(huart3.Instance->SR & UART_FLAG_RXNE)
	{
			receive = huart3.Instance->DR;
			
			openmv_data[openmv_data_index++]=receive;
		
	}
	//idle interrupt �����ж�
	else if(huart3.Instance->SR & UART_FLAG_IDLE)
	{
			receive = huart3.Instance->DR;
			uint8_t message_to_station[5];
			if(openmv_data[0] == 0xAA && openmv_data[1] == 0x55 && openmv_data[3] == 0x5D)
			{
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
					message_to_station[0] = openmv_data[2];
					message_to_station[1] = current_position.x;
					message_to_station[2] = current_position.y;
					message_to_station[3] = '\r';
					message_to_station[4] = '\n';
					uint8_t at1[] = "AT+CIPSEND=5\r\n";
					HAL_UART_Transmit(&huart1,at1,sizeof(at1),100); // 0:Elephant  1:Tiger  2:Wolf  3:Monkey  4:Peacock
					int tt=0;
					for(volatile int i=0;i<10000;i++)
							tt++;
					HAL_UART_Transmit(&huart1,message_to_station,sizeof(message_to_station),100);	
			}
			for(int i=0;i<=openmv_data_index;i++)
					openmv_data[i]=0;
			openmv_data_index = 0;
	}
}

void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
	volatile uint8_t Res;
	LidarDataTypeDef lidardata[4];
	if(huart6.Instance->SR & UART_FLAG_RXNE)
	{
			Res = huart6.Instance->DR;
			
			if (uart6_rx_con < 2)
			{
				//接收帧头
				if(uart6_rx_con == 0)  
				{
					//判断帧头1 AA
					if(Res == LD_HEADER1)
					{
						uart6_rx_buf[uart6_rx_con] = Res;
						uart6_rx_con = 1;					
					}
				}
				else
				{
					//判断帧头2 55
					if(Res == LD_HEADER2 || Res == LD_HEADER3 || Res == LD_HEADER4)
					{
						uart6_rx_buf[uart6_rx_con] = Res;
						uart6_rx_con = 2;					
					}
					else uart6_rx_con = 0;
						
				}
			}
			else  //接收数据
			{
				//判断是否接收完
				if(uart6_rx_buf[1] == LD_HEADER2)
				{
					if(uart6_rx_con < LD_F_LEN1)
					{
						uart6_rx_buf[uart6_rx_con] = Res;
						uart6_rx_con++;
					}
					else
					{
						
						for (int i = 0; i < 4; i++) {
							uint16_t high_byte = uart6_rx_buf[2 + i*2];
							uint16_t low_byte  = uart6_rx_buf[3 + i*2];
							lidardata[i].distance = (high_byte << 8) | low_byte;
							lidardata[i].angle = i * 90;
						}
						send_lidar_data(lidardata);
	
						for(int i=0; i<LD_F_LEN1; i++)
						{	
							uart6_rx_buf[i] = 0;
						}
						//复位
						uart6_rx_con = 0;
					}
				}
				else if(uart6_rx_buf[1] == LD_HEADER3 || uart6_rx_buf[1] == LD_HEADER4)
				{
					if(uart6_rx_con < LD_F_LEN2)
					{
						uart6_rx_buf[uart6_rx_con] = Res;
						uart6_rx_con++;
					}
					else
					{
						if(uart6_rx_buf[1] == LD_HEADER3)
						{
							int16_t lidar_x = (uart6_rx_buf[2] << 8) | uart6_rx_buf[3];
							int16_t lidar_y = (uart6_rx_buf[4] << 8) | uart6_rx_buf[5];
							send_xy(lidar_x, lidar_y);
						}
						else if(uart6_rx_buf[1] == LD_HEADER4)
						{
							uint16_t min_angle = (uart6_rx_buf[2] << 8) | uart6_rx_buf[3];
							uint16_t min_dist  = (uart6_rx_buf[4] << 8) | uart6_rx_buf[5];
							send_obstacle(min_angle, min_dist);
						}
						for(int i=0; i<LD_F_LEN1; i++)
						{	
							uart6_rx_buf[i] = 0;
						}
						//复位
						uart6_rx_con = 0;
					}
				}
			}
			
	}
	else if(huart6.Instance->SR & UART_FLAG_IDLE)
	{
			Res = huart6.Instance->DR;
	}
}
