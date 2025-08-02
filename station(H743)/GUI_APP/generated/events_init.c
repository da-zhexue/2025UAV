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
            //draw_line(path);
            static lv_point_t line_points[60];
            for(int i = 0; i < path.size; i++)
            {
                line_points[i].x = path.path[i][0] * 40 + 49;
                line_points[i].y = path.path[i][1] * 40 + 21;
            }
            if(path.size < 60)
            {
                for(int i = path.size; i < 60; i++)
                {
                    line_points[i].x = path.path[path.size - 1][0] * 40 + 37;
                    line_points[i].y = path.path[path.size - 1][1] * 40 + 21;
                }
            }
            lv_obj_t * line = lv_line_create(lv_scr_act());
            lv_line_set_points(line, line_points, path.size);
            static lv_style_t style_line;
            lv_style_init(&style_line);
            lv_style_set_line_width(&style_line, 3);
            lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
            lv_style_set_line_rounded(&style_line, true);
            lv_obj_add_style(line, &style_line, 0);
            //lv_obj_set_align(line, LV_ALIGN_TOP_LEFT);
            send_data(path);
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
    lv_obj_add_event_cb(ui->screen_run, screen_run_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
