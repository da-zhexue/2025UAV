#include "uart_receive.h"
#include "Drv_Uart.h"
#include "LX_FC_Fun.h"

CircleInfo circle_data = {.x = 80, .y = 60};
PositionInfo target_position = {.x = 0, .y = 0, .h = 0}, position_data;
DirectionInfo direction_data[64];
u8 total_step = 0;
LidarInfo lidar_data;

void processUART3Data(u8* data, u8 cmd);

void UART3_DataParser(u8 data) {
    static uart3_parse_state parse_state = WAIT_HEADER_AA;
    static u8 cmd_type = 0;
    static u8 data_index = 0;
    static u8 temp_buffer[5];  // 扩大缓冲区以容纳不同指令
    
    switch(parse_state) {
        case WAIT_HEADER_AA:
            if(data == 0xAA) 
                parse_state = WAIT_HEADER_55;
            break;
            
        case WAIT_HEADER_55:
            parse_state = (data == 0x55) ? WAIT_CMD : WAIT_HEADER_AA;
            break;
            
        case WAIT_CMD:
            cmd_type = data;  // 存储指令类型
            parse_state = PARSE_DATA;
            break;
            
        case PARSE_DATA:
            temp_buffer[data_index++] = data;
            if(data_index >= 4) {
                parse_state = WAIT_FOOTER_5D;
            }
            break;
            
        case WAIT_FOOTER_5D:
            if(data == 0x5D) {
                // 根据指令类型分发处理
								processUART3Data(temp_buffer, cmd_type);
            }
            parse_state = WAIT_HEADER_AA;
						data_index = 0;
            break;
    }
}

void processUART3Data(u8* data, u8 cmd)
{
		switch(cmd){
			case CMD_HEIGHT_GET:
					position_data.h = (data[0] << 8) | data[1];
					break;
			case CMD_XY_GET:
					position_data.x = (data[0] << 8) | data[1];
					position_data.y = (data[2] << 8) | data[3];
					break;
			case CMD_SEARCH:
					direction_data[total_step].step = data[0];
					direction_data[total_step].dis = 50;
					direction_data[total_step++].ang = (data[2] << 8) | data[3];
					total_step++;
					break;
			case CMD_LAND:
					circle_data.x = data[0];
					circle_data.y = data[1];
					break;
			case CMD_OBSTACLE_AVOID_1:
					lidar_data.dis[0] = (data[0] << 8) | data[1];
					lidar_data.dis[1] = (data[2] << 8) | data[3];
					break;
			case CMD_OBSTACLE_AVOID_2:
					lidar_data.dis[2] = (data[0] << 8) | data[1];
					lidar_data.dis[3] = (data[2] << 8) | data[3];
					break;
			case CMD_OBSTACLE_AVOID_3:
					lidar_data.min_ang = (data[0] << 8) | data[1];
					lidar_data.min_dis = (data[2] << 8) | data[3];
					break;
		}
}

void return_step(u8 step)
{
		u8 uart2_send_data[4];
		uart2_send_data[0] = 0xAA;
		uart2_send_data[1] = 0x55;
		uart2_send_data[2] = step;
		uart2_send_data[3] = 0x5D;
		
		DrvUart2SendBuf(uart2_send_data, sizeof(uart2_send_data));
}



