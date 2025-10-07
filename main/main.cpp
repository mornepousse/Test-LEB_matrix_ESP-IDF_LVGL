/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_log.h"
#include "lvgl.h"
#include "driver/gpio.h"
#include "freertos/idf_additions.h"
#include "soc/gpio_num.h"
#include "esp_timer.h"

#define NO_GFX
#define PANEL_W 64  // largeur d’un module
#define PANEL_H 32  // hauteur d’un module
#define CHAIN_LEN 5 // nb de modules en chaîne

MatrixPanel_I2S_DMA *panel = nullptr;

// === LVGL draw buffer ===
// Taille « tuilée » (pas l’écran entier) pour limiter la RAM.
#define LVGL_BUF_PIXELS (PANEL_W * 16)
static lv_color_t lv_buf1[LVGL_BUF_PIXELS];
static lv_color_t lv_buf2[LVGL_BUF_PIXELS];

static void lv_tick_cb(void *arg) { lv_tick_inc(1); }

static inline void hub75_blit565(MatrixPanel_I2S_DMA *panel,
                                 int16_t x, int16_t y,
                                 const uint16_t *bitmap,
                                 int16_t w, int16_t h)
{
    for (int16_t j = 0; j < h; j++) {
        const uint16_t *row = bitmap + j * w;
        for (int16_t i = 0; i < w; i++) {
            panel->drawPixel(x + i, y + j, row[i]);
        }
    }
}

// Blit LVGL -> HUB75
static void hub75_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    int32_t w = (area->x2 - area->x1 + 1);
    int32_t h = (area->y2 - area->y1 + 1);

    // LVGL donne du RGB565 contigu -> la lib sait le prendre d’un coup
    //panel->fillRect(area->x1, area->y1, w, h, color_p->full);
     hub75_blit565(panel, area->x1, area->y1,
                  (const uint16_t *)color_p, w, h);
    // panel->fillRectDMA(area->x1, area->y1, w, h, color_p->full);

    lv_disp_flush_ready(drv);
}

static void lvgl_task(void *arg)
{
    while (1)
    {
        lv_timer_handler();            // traite les animations/timers LVGL
        vTaskDelay(pdMS_TO_TICKS(10)); // 5 à 10 ms est ok
    }
}

static void hub75_start(void)
{
    HUB75_I2S_CFG mxcfg(
        PANEL_W,  // largeur module
        PANEL_H,  // hauteur module
        CHAIN_LEN // nb modules
        // pins                // mapping GPIO
    );

    // Quelques options utiles :
    // mxcfg.clkphase = false;   // si pixels décalés/ghosting (voir README)
     mxcfg.i2sspeed = HUB75_I2S_CFG::HZ_20M; // fréquence clk si besoin

    panel = new MatrixPanel_I2S_DMA(mxcfg);
    panel->begin();             // alloue DMA + démarre le rafraîchissement
    panel->setBrightness8(100); // 0..255
    panel->clearScreen();
    panel->fillScreen(0); // noir
}
lv_disp_t *disp ;
extern "C" void app_main()
{

    gpio_set_direction(GPIO_NUM_47, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_48, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_20, GPIO_MODE_OUTPUT);

    gpio_set_level(GPIO_NUM_47, 1);
    gpio_set_level(GPIO_NUM_38, 1);
    gpio_set_level(GPIO_NUM_19, 1);
    gpio_set_level(GPIO_NUM_20, 1);

    hub75_start();

    // --- LVGL ---
    lv_init();

    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, lv_buf1, lv_buf2, LVGL_BUF_PIXELS);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = PANEL_W * CHAIN_LEN;
    disp_drv.ver_res = PANEL_H;
    disp_drv.flush_cb = hub75_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp = lv_disp_drv_register(&disp_drv);

    // Tick 1 ms
    const esp_timer_create_args_t targs = {
        .callback = &lv_tick_cb, .name = "lv_tick"};
    esp_timer_handle_t tick;
    esp_timer_create(&targs, &tick);
    esp_timer_start_periodic(tick, 1000); // 1 ms

    // Tâche LVGL
    xTaskCreate(lvgl_task, "lvgl", 4096, NULL, 5, NULL);


// Positionner le label
    // rectangle
    // lv_obj_t *rect = lv_obj_create(lv_scr_act());
    // lv_obj_set_size(rect, PANEL_W * CHAIN_LEN, PANEL_H);
    // lv_obj_set_style_bg_color(rect, lv_color_hex(0xFF0F03), 0);
    // lv_obj_align(rect, LV_ALIGN_CENTER, 0, 0);
    // Créer un objet de type rectangle (en fait un objet de base)
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));




        lv_obj_t *label = lv_label_create(lv_scr_act());
        static int count = 0;
        char buf[32];
        snprintf(buf, sizeof(buf), "Hello LVGL! %d", count++);
        lv_label_set_text(label, buf);
        if (count > 10)
        {
            count = 0;
        }
        // vTaskDelay(pdMS_TO_TICKS(1000));
        // lv_obj_t *rect = lv_obj_create(lv_scr_act());
        // lv_obj_set_size(rect, PANEL_W * CHAIN_LEN, PANEL_H);
        // lv_obj_set_style_bg_color(rect, lv_color_hex(0xFF0F03), 0);
        // lv_obj_align(rect, LV_ALIGN_CENTER, 0, 0);
        // vTaskDelay(pdMS_TO_TICKS(1000));
        // lv_obj_t *recte = lv_obj_create(lv_scr_act());
        // lv_obj_set_size(recte, PANEL_W * CHAIN_LEN, PANEL_H);
        // lv_obj_set_style_bg_color(recte, lv_color_hex(0xFF0FFF), 0);
        // lv_obj_align(recte, LV_ALIGN_CENTER, 0, 0);
    }
}
