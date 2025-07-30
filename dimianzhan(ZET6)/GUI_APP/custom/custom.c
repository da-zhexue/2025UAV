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
#include "gpio.h"
#include "usart.h"
/*********************
 *      DEFINES
 *********************/
Ban_Position ban_position;
const int dr[] = {-1, 0, 1, 0};
const int dc[] = {0, 1, 0, -1};
void draw_line(Path path)
{
//    lv_draw_line_dsc_t line_dsc;
//    lv_draw_line_dsc_init(&line_dsc);
//    line_dsc.color = lv_color_hex(0x0000FF); // 蓝色
//    line_dsc.width = 3; // 线宽为5像素
//    line_dsc.opa = LV_OPA_COVER; // 不透明
    // line_dsc.color = lv_color_hex(0x00FF00);
    // lv_point_t points[] = {{0, 0}, {100, 100}};
		static lv_style_t style_line;
		lv_style_init(&style_line);
		lv_style_set_line_color(&style_line, lv_color_hex(0x0000FF)); // 蓝色
		lv_style_set_line_width(&style_line, 3); // 宽度4像素
		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_SET);
		for(int i = 1; i < path.size; i++)
		{
				int px = path.path[i][0] < path.path[i-1][0] ? path.path[i][0] : path.path[i-1][0];
				int py = path.path[i][1] < path.path[i-1][1] ? path.path[i][1] : path.path[i-1][1];
				lv_point_t points[2] = {{0, 0}, {0, 50}};
				if(path.path[i][1] == path.path[i-1][1]) 
				{
						points[1].x = 50;
						points[1].y = 0;
				}
				lv_line_set_points(guider_ui.line[i], points, 2);
				lv_obj_set_pos(guider_ui.line[i], 62 + px * 50, 53 + py * 50);
				lv_obj_set_size(guider_ui.line[i], 50, 50);
		}
    
    // line_dsc.width = 3;
    //lv_canvas_draw_line(guider_ui.screen_canvas_1, points, path.size, &line_dsc);  // 在画布上绘制[9](@ref)
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
    lv_obj_set_pos(ban[num], 12+r*50, 3+c*50);
}
const char* int_to_char(int input)
{
		char cchar[2];
		if(input < 10)
				cchar[0] = '0' + input;
		else
		{
				cchar[0] = '0' + (input / 10);
				cchar[0] = '0' + (input % 10);
		}
		const char* output = cchar;
		return output;
}
const char* int_to_pos(int x, int y)
{
		char cchar[5]="(0,0)";
		cchar[1] = '0' + x;
		cchar[3] = '0' + y;
		const char* output = cchar;
		return output;
}
void renovate_the_num(const char *data)
{
		static int animal_num[5]={0, 0, 0, 0, 0};
		int animal = data[0]; // 0:Elephant  1:Tiger  2:Wolf  3:Monkey  4:Peacock
		int ax = data[1], ay = data[2];
		if(lv_scr_act() == guider_ui.screen_1)
				lv_table_set_cell_value(guider_ui.screen_1_table_1, animal_num[animal] + 2, animal + 1, int_to_pos(ax, ay));
		animal_num[animal]++;
		if(lv_scr_act() == guider_ui.screen_1)
				lv_table_set_cell_value(guider_ui.screen_1_table_1, 1, animal + 1, int_to_char(animal_num[animal]));
}
void send_data(Path path)
{
    uint8_t data[256];
		data[0] = 0xAA;
		data[1] = 0x55;
		data[2] = path.size & 0xFF;
		for(int i = 1; i < path.size; i++)
		{
				int ang = 0;
				int dis = path.path[i][0] - path.path[i-1][0];
				if(dis > 0) ang = 180;
				else if(dis < 0){ ang = 0; dis=-dis;}
				else{
						dis = path.path[i][1] - path.path[i-1][1];
						if(dis > 0) ang = 270;
						else{ ang = 90; dis=-dis;}
				}
				data[i*4-1] = (dis >> 8) & 0xff;
				data[i*4] = dis & 0xff;
				data[i*4+1] = (ang >> 8) & 0xff;
				data[i*4+2] = ang & 0xff;
		}
		data[255] = 0x5D;
		uint8_t at[] = "AT+CIPSEND=256\r\n";
		HAL_UART_Transmit(&huart3,at,sizeof(at),100);
		HAL_Delay(250);
		HAL_UART_Transmit(&huart3,data,sizeof(data),100);
		HAL_Delay(2500);
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

