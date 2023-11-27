#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_wifi.h"
#include "../common/SensorData.h"
#include "../common/ApiComm.h"


#define PM_SET       GPIO_NUM_17
#define PM_RST       GPIO_NUM_16

#define PM_OUTPUT_PIN_SEL  ((1ULL<<PM_SET) | (1ULL<<PM_RST))

#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TIMEOUT_MS       1000
#define SENSOR_ADDR                 0x12


bool pt_verify_integrity(const uint8_t* data){
    if(data[0]==0x42 && data[1]==0x4d){
        unsigned int checksum=0;
        for(int i=0;i<0x28;i++){
            checksum+=data[i];
        }
        return checksum==(data[0x28]<<8)+data[0x29];
    }
    return false;
}
uint16_t get_sensor_data(const uint8_t* data, int index){
    return data[index+1]+(data[index]<<8);
}
void sensor_comm(void*)
{
    gpio_config_t io_conf = {};
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = PM_OUTPUT_PIN_SEL;
    gpio_config(&io_conf);
    gpio_hold_dis(PM_SET);
    gpio_hold_dis(PM_RST);
    gpio_set_level(PM_SET,1);
    gpio_set_level(PM_RST,1);

    printf("PM Enabled\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    printf("PM Reset\n");
    gpio_set_level(PM_RST,0);
    vTaskDelay(pdMS_TO_TICKS(500));
    printf("PM Done\n");
    gpio_set_level(PM_RST,1);
    printf("Waiting 20 secs for warm up\n");
    vTaskDelay(pdMS_TO_TICKS(20000));
    // i2c init
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
    uint8_t data[48];
    for(int i=0;i<30;i++){
        memset(data,0,48);
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        ESP_ERROR_CHECK(i2c_master_start(cmd));
        ESP_ERROR_CHECK(i2c_master_write_byte(cmd, ( SENSOR_ADDR << 1 ) | I2C_MASTER_READ, true));
        ESP_ERROR_CHECK(i2c_master_read(cmd, data, 48, I2C_MASTER_LAST_NACK));
        ESP_ERROR_CHECK(i2c_master_stop(cmd));
        auto ret = i2c_master_cmd_begin(i2c_master_port, cmd, 1000 / portTICK_RATE_MS);
        if(ret != ESP_OK){
            printf("I2C Comm Failed!\n");
        }
        i2c_cmd_link_delete(cmd);
        if(pt_verify_integrity(data)){
            printf("Check PASS\n");
            printf("PM Read end PM1.0=%d PM2.5=%d PM10=%d TVOC=%d HCHO=%d CO2=%d TEMP=%f HUMI=%f\n",
                   get_sensor_data(data,0x0a),
                   get_sensor_data(data,0x0c),
                   get_sensor_data(data,0x0e),
                   get_sensor_data(data,0x1c),
                   get_sensor_data(data,0x1f),
                   get_sensor_data(data,0x22),
                   get_sensor_data(data,0x24)/10.,
                   get_sensor_data(data,0x26)/10.
            );
            SensorData::push_pt_value(get_sensor_data(data,0x0a),
                                      get_sensor_data(data,0x0c),
                                      get_sensor_data(data,0x0e),
                                      get_sensor_data(data,0x1c),
                                      get_sensor_data(data,0x1f),
                                      get_sensor_data(data,0x22),
                                      (float)get_sensor_data(data,0x24)/10.f,
                                      (float)get_sensor_data(data,0x26)/10.f);
        }else{
            printf("Check Failed\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1500));
    }

    SensorData::pt_data_process();
    ApiComm::upload();
    const unsigned long wakeup_time_sec = 30*60;
    printf("Enabling timer wakeup, %lus\n", wakeup_time_sec);
    esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000UL);
    printf("Entering deep sleep\n");
    gpio_set_level(PM_SET,0);
    gpio_set_level(PM_RST,0);
    gpio_hold_en(PM_SET);
    gpio_hold_en(PM_RST);
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(5000));
    gpio_deep_sleep_hold_en();
    esp_deep_sleep_start();
    vTaskDelete(NULL);

}

