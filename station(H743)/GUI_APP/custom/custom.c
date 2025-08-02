/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/


/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include "custom.h"
#include "./usart/bsp_usart.h"
/*********************
 *      DEFINES
 *********************/
Ban_Position ban_position;
const int dr[] = {-1, 0, 1, 0};
const int dc[] = {0, 1, 0, -1};
void send_data(Path path)
{
  uint8_t data[128];
	uint8_t data2[10]={0xAA, 0x55, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, '\r', '\n'};
	data[0] = 0xAA;
	data[1] = 0x55;
	data[2] = path.size & 0xFF;
	for(int i = 0; i < path.size; i++)
	{
		data[i*2+3] = (path.path[i][0]) & 0xff;
		data[i*2+4] = (path.path[i][1]) & 0xff;
	}
	data[125] = 0x5D;
	data[126] = '\r';
	data[127] = '\n';
	uint8_t at[] = "AT+CIPSEND=0,128\r\n";
	HAL_UART_Transmit(&UartHandle,at,sizeof(at),100);
	HAL_Delay(500);
	HAL_UART_Transmit(&UartHandle,data,sizeof(data),100);
	int tt=0;
	for(volatile int i=0;i<10000;i++)
	 	tt++;
	//HAL_Delay(2500);
}
// 初始化地图和障碍
void initMap(Map* map, Ban_Position ban_p) {
    // 初始化地图全部可走
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            map->grid[i][j] = 0;
            map->visited[i][j] = 0;
        }
    }
    map->totalSteps = ROWS * COLS - 3;  // 总需遍历的格子数
    // 设置连续3个灰色方块
    for(int i = 0; i < 3; i++)
    {
        map->grid[ban_p.x[i]][ban_p.y[i]] = 1;
    }
    for(int k = 0; k < 3; k++)
        for(int i = 0; i < ROWS; i++){
            for(int j = 0; j < COLS; j++){
                if(map->grid[i][j] == 0){
                    int error_box = 0;
                    if(i == 0) error_box++;
                    else if(map->grid[i-1][j] != 0) error_box++;
                    if(i == 8) error_box++;
                    else if(map->grid[i+1][j] != 0) error_box++;
                    if(j == 0) error_box++;
                    else if(map->grid[i][j-1] != 0) error_box++;
                    if(j == 6) error_box++;
                    else if(map->grid[i][j+1] != 0) error_box++;

                    if(error_box == 3){
                        map->grid[i][j] = 1;
                        map->totalSteps --;
                    }    
                }
                
            }
        }
        
    
}
bool isConnected(Map* map, int r, int c, int remaining, bool tempVisited[ROWS][COLS]) {
    if (remaining == 0) return true;
    
    bool bfsVisited[ROWS][COLS];
    // 复制临时访问状态
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            bfsVisited[i][j] = tempVisited[i][j];
        }
    }
    
    int queue[MAX_PATH_LEN][2];
    int front = 0, rear = 0;
    queue[rear][0] = r;
    queue[rear][1] = c;
    rear++;
    int count = 0;
    
    while (front < rear) {
        int r0 = queue[front][0];
        int c0 = queue[front][1];
        front++;
        
        for (int i = 0; i < 4; i++) {
            int nr = r0 + dr[i];
            int nc = c0 + dc[i];
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                if (!map->grid[nr][nc] && !bfsVisited[nr][nc]) {
                    bfsVisited[nr][nc] = true;
                    count++;
                    queue[rear][0] = nr;
                    queue[rear][1] = nc;
                    rear++;
                }
            }
        }
    }
    return count >= remaining;
}

// DFS回溯搜索路径
bool DFS(Map* map, Path* path, int r, int c, int step) {
    // 完成所有格子遍历
    if (step == map->totalSteps) return true;
    
    for (int i = 0; i < 4; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        
        // 检查新位置是否有效
        if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
            if (!map->grid[nr][nc] && !map->visited[nr][nc]) {
                
                // 创建临时访问状态用于连通性检查
                bool tempVisited[ROWS][COLS];
                for (int i = 0; i < ROWS; i++) {
                    for (int j = 0; j < COLS; j++) {
                        tempVisited[i][j] = map->visited[i][j];
                    }
                }
                tempVisited[nr][nc] = true;
                
                // 检查连通性
                if (isConnected(map, nr, nc, map->totalSteps - step - 1, tempVisited)) {
                    // 记录路径
                    path->path[path->size][0] = nr;
                    path->path[path->size][1] = nc;
                    path->size++;
                    map->visited[nr][nc] = true;
                    
                    // 递归搜索
                    if (DFS(map, path, nr, nc, step + 1)) {
                        return true;
                    }
                    
                    // 回溯：移除路径点并重置访问状态
                    path->size--;
                    map->visited[nr][nc] = false;
                }
            }
        }
    }
    return false;
}

// 查找路径主函数
// bool findPath(Map* map, Path* path, int startRow, int startCol) {
//     path->size = 0;
//     path->path[path->size][0] = startRow;
//     path->path[path->size][1] = startCol;
//     path->size++;
    
//     return DFS(map, startRow, startCol, 1, path);
// }

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
void show_ban_position(const char *input, int num)
{
    lv_obj_t *ban[3] = {guider_ui.screen_ban1, guider_ui.screen_ban2,guider_ui.screen_ban3};
    lv_obj_clear_flag(ban[num],LV_OBJ_FLAG_HIDDEN);
    int r = input[1] - '0';
    int c = input[3] - '0';
    ban_position.x[num] = r-1;
    ban_position.y[num] = c-1;
    // char rchar[2]={0,0}, cchar[2]={0,0};
    // rchar[0] = '0' + ban_position.x[num]+1;
    // cchar[0] = '0' + ban_position.y[num]+1;
    // lv_table_set_cell_value(guider_ui.screen_table_1, 0, 0, rchar);  
    // lv_table_set_cell_value(guider_ui.screen_table_1, 0, 1, cchar);  
    //lv_table_set_cell_value(guider_ui.screen_table_1, 0, 1, c-1);  
    lv_obj_set_pos(ban[num], -11+r*40, -39+c*40);
}

void renovate_the_num(const char *data)
{
		static int animal_num[5]={0, 0, 0, 0, 0};
		int animal = data[0]; // 0:Elephant  1:Tiger  2:Wolf  3:Monkey  4:Peacock
		int ax = data[1], ay = data[2];
		animal_num[animal]++;
		char cchar[4]="A0B0", rchar[2]={0x00, 0x00};
		rchar[0] = '0' + animal_num[animal];
		cchar[1] = '0' + ax;
		cchar[3] = '0' + ay;
		const char* output1 = cchar;
		const char* output2 = rchar;
		lv_table_set_cell_value(guider_ui.screen_table_1, animal_num[animal] + 1, animal + 1, output1);
		lv_table_set_cell_value(guider_ui.screen_table_1, 1, animal + 1, output2);
}
/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */

void custom_init(lv_ui *ui)
{
    /* Add your codes here */
}

