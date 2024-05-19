#ifndef __BSP_WIFI_DRIVER_H__
#define __BSP_WIFI_DRIVER_H__

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "lwip/inet.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif

#define WIFI_CONNECTED_BIT BIT0      // wifi 连接成功
#define WIFI_FAIL_BIT      BIT1      // wifi 连接失败

#define WIFI_AP_SSID            "Magical_WangFan"                             // wifi的 名称
#define WIFI_AP_PASSWORD        "12345678"                             // wifi的 密码
#define WIFI_AP_PASSWORD_LEN    strlen(WIFI_AP_PASSWORD)       // wifi 密码长度
#define WIFI_AP_CHANNEL         1                              // wifi的 频道  1 ~ 13 默认 1
#define WIFI_AP_MAX_CONNECT     4                              // wifi 最大连接数

#define WIFI_STA_SSID            ""                            // 使用的 wifi 名称
#define WIFI_STA_PASSWORD        ""                            // 使用的 wifi 密码
#define WIFI_STA_SCANNING_MODE   WIFI_ALL_CHANNEL_SCAN         // 扫描模式 默认 扫描所有频道 1 ~ 13
#define WIFI_STA_AUTH            WIFI_AUTH_WPA2_PSK            // 认证模式 默认 WPA2_PSK
#define WIFI_CONNECTION_TYPE     WIFI_CONNECT_AP_BY_SECURITY   // 连接模式 默认 按照安全级连接
#define ESP_STA_MAXIMUM_RETRY    5                             // 最大重试次数
#define WIFI_SAE_MODE            WPA3_SAE_PWE_BOTH             // 默认 WPA3_SAE_PWE_BOTH

void bsp_wifi_init(void);

#endif