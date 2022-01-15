#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/i2c.h>
#include <esp_event.h>
#include <esp_log.h>
#include <hap.h>
#include <hap_apple_chars.h>
#include <bulb.h>
#include <aht10.h>
#include <hapc.h>

#define ERR_CHECK(result, err_msg)           \
    if (result != ESP_OK) {                  \
        ESP_LOGE("ERROR: ", err_msg);        \
        ESP_LOGE("ERROR: ", "%d", result);   \
        abort();                             \
    }                                        \

static const char *TAG = "HAP Cloud bulb";

static void vInitI2C();
static int vIdentify(hap_acc_t *ha);
static int iLightRead(hap_char_t *hc , hap_status_t *status, void *serv_priv, void *read_priv);
static int iLightWrite(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv);
static int iTHSensorRead(hap_char_t *hc , hap_status_t *status, void *serv_priv, void *read_priv);

void app_main()
{
    hapc_config_t config = {
        .brightness_reader = iLightRead,
        .brightness_writer = iLightWrite,
        .temperature_reader = iTHSensorRead,
        .humidity_reader = iTHSensorRead,
        .identify = vIdentify,
    };

    vInitI2C();
    vAHTInit();
    xBulbInit();

    vHAPInit(&config);
}

static int iLightWrite(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv)
{
    if (hap_req_get_ctrl_id(write_priv)) {
        ESP_LOGI(TAG, "Received write from %s", hap_req_get_ctrl_id(write_priv));
    }
    int i, ret = HAP_SUCCESS;
    hap_write_data_t *write;

    for (i = 0; i < count; i++) {
        write = &write_data[i];

        if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_ON)) {
            ESP_LOGI(TAG, "Received Write for Light %s", write->val.b ? "On" : "Off");
            xBulbSwitch(write->val.b);

            *(write->status) = HAP_STATUS_SUCCESS;
        } else if (!strcmp(hap_char_get_type_uuid(write->hc), HAP_CHAR_UUID_BRIGHTNESS)) {
            ESP_LOGI(TAG, "Received Write for Light Brightness %d", write->val.i);
            xBulbSetBttightness(write->val.i);

            *(write->status) = HAP_STATUS_SUCCESS;
        } else {
            *(write->status) = HAP_STATUS_RES_ABSENT;
        }

        if (*(write->status) == HAP_STATUS_SUCCESS) {
            hap_char_update_val(write->hc, &(write->val));
        } else {
            ret = HAP_FAIL;
        }
    }
    return ret;
}

static int iLightRead(hap_char_t *hc , hap_status_t *status, void *serv_priv, void *read_priv)
{
    if (!strcmp(hap_char_get_type_uuid(hc), HAP_CHAR_UUID_BRIGHTNESS)) {
        uint8_t bright = iCurrentBrightnes();
        hap_val_t h_val = {.i = bright};

        *status = HAP_STATUS_SUCCESS;
        hap_char_update_val(hc, &h_val);
    }

    return HAP_SUCCESS;
}

static int iTHSensorRead(hap_char_t *hc , hap_status_t *status, void *serv_priv, void *read_priv) {
    if (hap_req_get_ctrl_id(read_priv)) {
        ESP_LOGI("HAP", "Received read from %s", hap_req_get_ctrl_id(read_priv));
    }

    if (!strcmp(hap_char_get_type_uuid(hc), HAP_CHAR_UUID_CURRENT_TEMPERATURE)) {
        float t = fAHTTempetarure();
        hap_val_t h_val = {.f = t > 0 ? t : 0};

        hap_char_update_val(hc, &h_val);
        *status = HAP_STATUS_SUCCESS;
    }

    if (!strcmp(hap_char_get_type_uuid(hc), HAP_CHAR_UUID_CURRENT_RELATIVE_HUMIDITY)) {
        float h = fAHTHumidity();
        hap_val_t t_val = {.f = h};

        hap_char_update_val(hc, &t_val);
        *status = HAP_STATUS_SUCCESS;
    }

    return HAP_SUCCESS;
}

static int vIdentify(hap_acc_t *ha)
{
    TickType_t delay = 250/portTICK_PERIOD_MS;

    xBulbSwitch(true);
    vTaskDelay(delay);
    
    xBulbSwitch(false);
    vTaskDelay(delay);
    
    xBulbSwitch(true);
    vTaskDelay(delay);
    
    xBulbSwitch(false);
    vTaskDelay(delay);
    
    xBulbSwitch(true);
    ESP_LOGI("HAP", "Accessory identified");

    return HAP_SUCCESS;
}

static void vInitI2C() {
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_18,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = GPIO_NUM_5,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };

    ERR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf), "i2c config failed");
    ERR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_conf.mode, 0, 0, 0), "i2c driver install failed");
}
