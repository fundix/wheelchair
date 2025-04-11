#ifndef WIFIUPDATE_HPP
#define WIFIUPDATE_HPP

#include <string.h>

#include <freertos/FreeRTOS.h>
#include <esp_http_server.h>
#include <freertos/task.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_wifi.h>

#define WIFI_SSID "ESP32 OTA Update"

esp_err_t index_get_handler(httpd_req_t *req);
esp_err_t update_post_handler(httpd_req_t *req);
esp_err_t http_server_init(void);
esp_err_t softap_init(void);

#endif