/*
* Copyright 2023 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

#define ROWS 9
#define COLS 7
#define MAX_PATH_LEN (ROWS * COLS)

typedef struct {
    int grid[ROWS][COLS];
    bool visited[ROWS][COLS];
    int totalSteps;
} Map;

// 路径存储结构
typedef struct {
    int path[MAX_PATH_LEN][2];
    int size;
} Path;

typedef struct {
     int x[3];
     int y[3];
} Ban_Position;
extern Ban_Position ban_position;
void show_ban_position(const char *input, int num);
void send_data(Path path);
void initMap(Map* map, Ban_Position ban_p);
bool findPath(Map* map, Path* path, int startRow, int startCol);
bool DFS(Map* map, Path* path, int r, int c, int step) ;
void draw_line(Path path);
void renovate_the_num(const char *data);
void custom_init(lv_ui *ui);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
