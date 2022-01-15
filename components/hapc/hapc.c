#include "hapc.h"

static void vHubCreate(hapc_config_t *conf);
static void vLighterCreate(hapc_config_t *conf);
static void vSensorCreate(hapc_config_t *conf);
static void vHAPInitConfig();
static void vResetKeyInit();
static void vHAPSetCredentialsAndStart();

void vHAPInit(hapc_config_t *conf)
{
    vHAPInitConfig();
    
    vHubCreate(conf);
    vLighterCreate(conf);
    vSensorCreate(conf);

    vResetKeyInit();
    vHAPSetCredentialsAndStart();
}

static void vHubCreate(hapc_config_t *conf) 
{
    hap_acc_t *accessory;
    hap_acc_cfg_t brige = {
        .name = "Esp Cloud lighter",
        .manufacturer = "Pezdot@",
        .model = "ESP32TSW",
        .serial_num = "202101153210",
        .fw_rev = "1.1.0",
        .hw_rev = NULL,
        .pv = "1.1",
        .identify_routine = conf->identify,
        .cid = HAP_CID_BRIDGE,
    };

    accessory = hap_acc_create(&brige);
    hap_add_accessory(accessory);
}

static void vLighterCreate(hapc_config_t *conf)
{
    hap_acc_t *accessory;
    hap_serv_t *service;

    hap_acc_cfg_t cfg = {
        .name = "Cloud lighter",
        .manufacturer = "Pezdota@",
        .model = "TC01",
        .serial_num = "202101153211",
        .fw_rev = "0.1.0",
        .hw_rev = NULL,
        .pv = "1.1",
        .identify_routine = conf->identify,
        .cid = HAP_CID_LIGHTING,
    };
    
    accessory = hap_acc_create(&cfg);

    service = hap_serv_lightbulb_create(true);
    hap_serv_add_char(service, hap_char_name_create("Cloud lighter"));
    hap_serv_add_char(service, hap_char_brightness_create(50));

    hap_serv_set_write_cb(service, conf->brightness_writer);
    hap_serv_set_read_cb(service, conf->brightness_reader);

    hap_acc_add_serv(accessory, service);
    hap_add_bridged_accessory(accessory, hap_get_unique_aid(cfg.name));
}

static void vSensorCreate(hapc_config_t *conf)
{
    hap_acc_t *accessory;
    hap_serv_t *service;

    hap_acc_cfg_t cfg = {
        .name = "Cloud TH sensor",
        .manufacturer = "Pezdota@",
        .model = "TC01",
        .serial_num = "202101153212",
        .fw_rev = "0.1.0",
        .hw_rev = NULL,
        .pv = "1.1",
        .identify_routine = conf->identify,
        .cid = HAP_CID_SENSOR,
    };
    
    accessory = hap_acc_create(&cfg);

    service = hap_serv_humidity_sensor_create(0);
    hap_serv_add_char(service, hap_char_name_create("Humidity sensor"));
    hap_serv_set_read_cb(service, conf->humidity_reader);
    hap_acc_add_serv(accessory, service);

    service = hap_serv_temperature_sensor_create(0);
    hap_serv_add_char(service, hap_char_name_create("Temperature sensor"));
    hap_serv_set_read_cb(service, conf->temperature_reader);
    hap_acc_add_serv(accessory, service);

    hap_add_bridged_accessory(accessory, hap_get_unique_aid(cfg.name));
}

static void vHAPInitConfig() 
{
    hap_cfg_t hap_cfg;

    hap_get_config(&hap_cfg);
    hap_cfg.unique_param = UNIQUE_NAME;
    hap_set_config(&hap_cfg);

    hap_init(HAP_TRANSPORT_WIFI);
}

static void vHAPSetCredentialsAndStart()
{
    hap_set_setup_code(CONFIG_EXAMPLE_SETUP_CODE);
    hap_set_setup_id(CONFIG_EXAMPLE_SETUP_ID);
    app_hap_setup_payload(CONFIG_EXAMPLE_SETUP_CODE, CONFIG_EXAMPLE_SETUP_ID, false, HAP_CID_BRIDGE);
    hap_enable_mfi_auth(HAP_MFI_AUTH_HW);

    app_wifi_init();
    hap_start();
    app_wifi_start(portMAX_DELAY);
}

static void vResetToFactory(void* arg)
{
    hap_reset_to_factory();
}

static void vResetKeyInit()
{
    button_handle_t handle = iot_button_create(GPIO_NUM_0, BUTTON_ACTIVE_LOW);
    iot_button_add_on_press_cb(handle, RESET_TO_FACTORY_BUTTON_TIMEOUT, vResetToFactory, NULL);
}