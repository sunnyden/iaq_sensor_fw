/* Deep sleep wake up example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "esp32/ulp.h"
#include "nvs_flash.h"

static RTC_DATA_ATTR struct timeval sleep_enter_time;

void power_mgmt(void*);
void network_stack(void*);
void sensor_comm(void*);
extern "C"{
    void app_main(void);
}
void app_main(void)
{
    //power_mgmt();
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER: {
            printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            printf("Not a deep sleep reset\n");
    }

    ESP_LOGI("WLAN", "ESP_WIFI_MODE_STA");
    xTaskCreate(network_stack, "network", 8 * 1024, NULL, 5, NULL);
    xTaskCreate(power_mgmt, "power_mgmt", 8 * 1024, NULL, 5, NULL);
    xTaskCreate(sensor_comm, "sensor", 8 * 1024, NULL, 5, NULL);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    //printf("Entering deep sleep\n");
    //gettimeofday(&sleep_enter_time, NULL);
    //vTaskDelay(10000 / portTICK_PERIOD_MS);
    //esp_deep_sleep_start();
}