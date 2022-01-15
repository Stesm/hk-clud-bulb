#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <math.h>
#include <driver/ledc.h>
#include <esp_err.h>
#include <esp_log.h>

#define LED_GPIO GPIO_NUM_15
#define LED_FREQ 5000
#define LED_MODE LEDC_HIGH_SPEED_MODE
#define LED_TIMER LEDC_TIMER_0
#define LED_RES LEDC_TIMER_13_BIT
#define LED_CHANNEL LEDC_CHANNEL_0

TimerHandle_t timer;
bool is_on;
uint32_t max_duty = (pow(2, LED_RES)) - 1;
uint8_t brightness = 100;

static void xTimerFireFade(TimerHandle_t xTimer)
{
    uint32_t duty = round(max_duty / 100.0 * brightness);
    ESP_LOGI("Br", "Switching to: %d[%d]", brightness, duty);
    
    ledc_set_fade_with_time(LED_MODE, LED_CHANNEL, duty, 500);
    ledc_fade_start(LED_MODE, LED_CHANNEL, LEDC_FADE_NO_WAIT);
}

void xBulbInit()
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LED_MODE,
        .timer_num = LED_TIMER,
        .duty_resolution = LED_RES,
        .freq_hz = LED_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LED_MODE,
        .channel = LED_CHANNEL,
        .timer_sel = LED_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LED_GPIO,
        .duty = max_duty,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ledc_fade_func_install(0);

    is_on = true;

    timer = xTimerCreate(
        "Timer", 
        pdMS_TO_TICKS(500), 
        pdFALSE,
        (void *) 0,
        xTimerFireFade
    );
    xTimerStart(timer, 0);
}

void xBulbSetBttightness(uint8_t percent)
{
    brightness = percent;
    xTimerReset(timer, 0);
}

void xBulbSwitch(bool state)
{
    if (state && !is_on) {
        ESP_LOGI("SW", "Switching on");

        uint32_t duty = round(max_duty / 100.0 * brightness);
        ledc_set_duty(LED_MODE, LED_CHANNEL, duty);
        ledc_update_duty(LED_MODE, LED_CHANNEL);
        is_on = true;
    } else if(!state && is_on) {
        ESP_LOGI("SW", "Switching off");
        
        ledc_set_duty(LED_MODE, LED_CHANNEL, 0);
        ledc_update_duty(LED_MODE, LED_CHANNEL);
        is_on = false;
    }
}

uint8_t iCurrentBrightnes()
{
    return brightness;
}