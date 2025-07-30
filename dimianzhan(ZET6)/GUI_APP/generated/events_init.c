/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

#include "custom.h"
char input_num[5]= {0, 0, 0, 0, 0};
int input_index = 0;
int num_index = 0;
#include "custom.h"

static void screen_keyboard_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        lv_obj_t * obj = lv_event_get_target(e);
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        uint16_t key_index = lv_btnmatrix_get_selected_btn(guider_ui.screen_keyboard);
        const char *btn_key =  lv_btnmatrix_get_btn_text(guider_ui.screen_keyboard,key_index);
        if(btn_key[0] == 'A' || btn_key[0] == 'B')
        {
            if(input_index<4)
            {
                input_num[input_index++]=btn_key[0];
                input_num[input_index++]=btn_key[1];
                lv_table_set_cell_value(guider_ui.screen_input, 0, 0, input_num);
            }

        }
        else if(btn_key[0]=='O' && btn_key[1]=='K')
        {
            const char* input=input_num;
            show_ban_position(input, num_index);
            input_index = 0;
            num_index++;
            for(int i = 0; i < 4; i++)
                input_num[i] = 0;
            char input_label[] = "INPUT";
            lv_table_set_cell_value(guider_ui.screen_input, 0, 0, input_label);
        }
        else
        {
            for(int i = 0; i < 4; i++)
                input_num[i] = 0;
            char input_label[] = "INPUT";
            lv_table_set_cell_value(guider_ui.screen_input, 0, 0, input_label);
            input_index=0;
        }
        break;
    }
    default:
        break;
    }
}

static void screen_next_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_1, guider_ui.screen_1_del, &guider_ui.screen_del, setup_scr_screen_1, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

static void screen_run_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        Map map;
        Path path;
        int startRow = ROWS - 1;
        int startCol = COLS - 1;


        initMap(&map, ban_position);
        path.path[0][0] = startRow;
        path.path[0][1] = startCol;
        path.size = 1;
        map.visited[startRow][startCol] = true;
        //if (findPath(&map, &path, startRow, startCol)) {
        if (DFS(&map, &path, startRow, startCol, 1)) {
            draw_line(path);
            // lv_draw_line_dsc_t line_dsc_2;
            // lv_draw_line_dsc_init(&line_dsc_2);
            // line_dsc_2.color = lv_color_hex(0x00FF00);
            // // lv_point_t points[] = {{0, 0}, {100, 100}};
            // lv_point_t points2[2];
            // points2[0].x = path.path[path.size][0] * 50 + 55;
            // points2[0].y = path.path[path.size][1] * 50 + 48;
            // line_dsc_2.width = 3;
            // lv_point_t fix_error = {0, 0};
            // if(path.path[path.size][0] != 0 && map.grid[path.path[path.size][0]-1][path.path[path.size][1]] == 2 )
            // {
            //     fix_error.x = (path.path[path.size][0]-1) * 50 + 55;
            //     fix_error.y = path.path[path.size][1] *50 + 48;
            // }
            // if(path.path[path.size][0] != 8 && map.grid[path.path[path.size][0]+1][path.path[path.size][1]] == 2 )
            // {
            //     fix_error.x = (path.path[path.size][0]+1) * 50 + 55;
            //     fix_error.y = path.path[path.size][1] *50 + 48;
            // }
            // if(path.path[path.size][1] != 0 && map.grid[path.path[path.size][0]][path.path[path.size][1]-1] == 2 )
            // {
            //     fix_error.x = path.path[path.size][0] * 50 + 55;
            //     fix_error.y = (path.path[path.size][1] - 1) *50 + 48;
            // }
            // if(path.path[path.size][1] != 6 && map.grid[path.path[path.size][0]][path.path[path.size][1]+1] == 2 )
            // {
            //     fix_error.x = path.path[path.size][0] * 50 + 55;
            //     fix_error.y = (path.path[path.size][1] + 1) *50 + 48;
            // }
            // if(fix_error.x != 0 && fix_error.y != 0)
            // {
            //     points2[1].x = fix_error.x;
            //     points2[1].y = fix_error.y;
            //     path.size++;
            //     map.totalSteps++;
            //     lv_canvas_draw_line(guider_ui.screen_canvas_1, points2, 2, &line_dsc_2);
            // }

        }
        break;
    }
    default:
        break;
    }
}

void events_init_screen (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_keyboard, screen_keyboard_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_next, screen_next_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_run, screen_run_event_handler, LV_EVENT_ALL, ui);
}

static void screen_1_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_1_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_1 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_1_btn_1, screen_1_btn_1_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
