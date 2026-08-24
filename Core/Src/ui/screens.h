#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_BROWSER = 2,
    SCREEN_ID_VIEWER = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *browser;
    lv_obj_t *viewer;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *browser_list;
    lv_obj_t *browser_top_bar;
    lv_obj_t *browser_close_button;
    lv_obj_t *browser_back_button;
    lv_obj_t *browser_confirm_box;
    lv_obj_t *confirm_yes_button;
    lv_obj_t *confirm_no_button;
    lv_obj_t *confirm_text;
    lv_obj_t *browser_menu_box;
    lv_obj_t *menu_open_button;
    lv_obj_t *menu_rename_button;
    lv_obj_t *menu_delete_button;
    lv_obj_t *menu_info_button;
    lv_obj_t *browser_rename_box;
    lv_obj_t *rename_cancel_button;
    lv_obj_t *rename_ok_button;
    lv_obj_t *rename_input;
    lv_obj_t *rename_keyboard;
    lv_obj_t *viewer_note;
    lv_obj_t *viewer_text;
    lv_obj_t *viewer_top_bar;
    lv_obj_t *viewer_title;
    lv_obj_t *viewer_close_button;
    lv_obj_t *viewer_back_button;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_browser();
void tick_screen_browser();

void create_screen_viewer();
void tick_screen_viewer();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/