#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>
#include <iot_button.h>
#include <app_wifi.h>
#include <app_hap_setup_payload.h>

#define RESET_TO_FACTORY_BUTTON_TIMEOUT 10
#define RESET_GPIO GPIO_NUM_0

typedef struct {
    hap_serv_read_t temperature_reader;
    hap_serv_read_t humidity_reader;
    hap_serv_read_t brightness_reader;
    hap_serv_write_t brightness_writer;
    hap_identify_routine_t identify;
} hapc_config_t;

void vHAPInit(hapc_config_t *conf);