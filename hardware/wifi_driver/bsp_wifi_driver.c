#include "bsp_wifi_driver.h"

static const char *TAG_AP = "WIFI SoftAP";
static const char *TAG_STA = "WIFI STA";

// wifi连接重试次数
static int connect_retry_num = 0;

/*AP 模式初始化*/
esp_netif_t* bsp_wifi_init_ap(void)
{
    // 网络接口创建
    esp_netif_t* esp_netif_ap = esp_netif_create_default_wifi_ap();
    // 初始化 wifi_config_t 结构体
    wifi_config_t wifi_ap_set_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,                                // wifi 名称
            .ssid_len = strlen(WIFI_AP_SSID),                    // wifi 名称长度
            .password = WIFI_AP_PASSWORD,                        // wifi 密码
            .max_connection = WIFI_AP_MAX_CONNECT,               // wifi 最大连接数
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,                  // wifi 密码加密方式
            .channel = WIFI_AP_CHANNEL,                          // wifi 频道
            .pmf_cfg = {                                         // wifi pmf 配置
                .required = false                                // pmf 必选
            },
        },
    };

    //如果密码为空，则设置为开放模式
    if(strlen(WIFI_AP_PASSWORD) == 0){
        wifi_ap_set_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    //配置wifi 并且判断配置是否成功
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_set_config));

    // 打印日志信息
    ESP_LOGI(TAG_AP, "wifi_init_softap finished.SSID:%s password:%s channel:%d",
            WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);

    return esp_netif_ap;
}

/*STA 模式初始化*/
esp_netif_t* bsp_wifi_init_sta(void)
{
    // 网络接口初始化
    esp_netif_t* esp_netif_sta = esp_netif_create_default_wifi_sta();
    // 设置主机名,既 wifi station 的名称
    esp_netif_set_hostname(esp_netif_sta, "七夕");

    // 初始化 wifi_config_t 结构体
    wifi_config_t wifi_sta_set_config = {
        .sta = {
            .ssid = WIFI_STA_SSID,                               // wifi 名称
            .password = WIFI_STA_PASSWORD,                       // wifi 密码
            .scan_method = WIFI_STA_SCANNING_MODE,               // wifi 扫描方式
            .sort_method = WIFI_CONNECTION_TYPE,                 // wifi 连接方式
            .failure_retry_cnt = ESP_STA_MAXIMUM_RETRY,          // wifi 连接失败重试次数
            .threshold.authmode = WIFI_STA_AUTH,                 // wifi 密码加密方式
            .sae_pwe_h2e = WIFI_SAE_MODE,                        // wifi 密码加密方式
            
        },
    };

    // 配置 wifi 并且判断配置是否成功
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_set_config));
    // 打印日志信息
    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");

    return esp_netif_sta;
}

//FreeRTOS 事件组句柄 连接/断开时发送信号
static EventGroupHandle_t wifi_event_group;

// wifi 事件处理函数
static void bsp_wifi_event_handler(void* arg, 
                                    esp_event_base_t event_base, 
                                    int32_t event_id, 
                                    void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        default:    break;
        // AP 模式下，有新的设备连接时触发
        case WIFI_EVENT_AP_STACONNECTED:
            // 获取事件数据
            wifi_event_ap_staconnected_t* AP_event = (wifi_event_ap_staconnected_t*) event_data;
            // 打印日志信息 显示连接的设备的MAC地址
            ESP_LOGI(TAG_AP, "station:"MACSTR" join, AID=%d", MAC2STR(AP_event->mac), AP_event->aid);
            break;
        // AP 模式下，有设备断开连接时触发
        case WIFI_EVENT_AP_STADISCONNECTED:
            // 获取事件数据
            AP_event = (wifi_event_ap_stadisconnected_t*) event_data;
            // 打印日志信息 显示断开连接的设备的MAC地址
            ESP_LOGI(TAG_AP, "station:"MACSTR" leave, AID=%d", MAC2STR(AP_event->mac), AP_event->aid);
            break;
        // STA 模式下，连接成功时触发
        case WIFI_EVENT_STA_START:
            // 打印日志信息 显示连接成功
            ESP_LOGI(TAG_STA, "WiFi connected");
            // 连接 wifi
            esp_wifi_connect();
            break;
        // IP 事件 触发
        case IP_EVENT_STA_GOT_IP:
            // 获取事件数据
            ip_event_got_ip_t* event_STA = (ip_event_got_ip_t*) event_data;
            // 打印日志信息 事件 ip 地址
            ESP_LOGI(TAG_STA, "got ip:" IPSTR, IP2STR(&event_STA->ip_info.ip));
            // 连接重试次数清零
            connect_retry_num = 0;
            // 设置信号
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }
    
}

void bsp_wifi_init(void)
{
    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());

    // 初始化事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 初始化 可掉电存储器
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /*初始化事件组*/
    wifi_event_group = xEventGroupCreate();
    // 注册 WiFi 事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, 
                                                ESP_EVENT_ANY_ID, 
                                                &bsp_wifi_event_handler,
                                                NULL, 
                                                NULL));
    // 注册 IP 事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, 
                                                IP_EVENT_STA_GOT_IP, 
                                                &bsp_wifi_event_handler,
                                                NULL, 
                                                NULL));

    // 初始化 wifi 配置
    wifi_init_config_t wifi_set_config = WIFI_INIT_CONFIG_DEFAULT();
    // 初始化 wifi
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_set_config));
    // 设置 wifi 休眠模式
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));
    // 设置 wifi 模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    // 初始化 AP 模式
    ESP_LOGI(TAG_AP, "wifi_init_ap finished.");
    esp_netif_t* esp_netif_ap = bsp_wifi_init_ap();

    // 初始化 STA 模式
    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");
    esp_netif_t* esp_netif_sta = bsp_wifi_init_sta();

    // 启动 wifi
    ESP_ERROR_CHECK(esp_wifi_start());

    // 等待信号
    /*
    *等待连接建立(WIFI_CONNECTED_BIT)或连接失败(WIFI_FAIL_BIT)。
    *比特位由event_handler()设置(见上文)
    */
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, 
                                        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, 
                                        pdFALSE, 
                                        pdFALSE, 
                                        portMAX_DELAY);
    /*
    *xEventGroupWaitBits() 函数在 FreeRTOS 中（特别是在 ESP-IDF 项目中广泛使用）的作用是在一个事件组中等待一个或多个事件的发生。
    *此函数会阻塞调用任务，直到指定的事件集中的任意一个或全部事件成为已设置状态，或者达到了指定的超时时间。
    *当这个函数返回时，它不仅会解除阻塞等待的任务，还会返回一个参数，该参数指示在函数返回之前事件组中已设置的位（即事件）。
    *这意味着你可以通过分析返回值来确定实际上哪些事件已经发生，而不仅仅是你最初等待的那些事件。
    *这个返回值允许你编写更灵活的代码，能够响应多种不同的事件组合，而不只是简单地等待一个特定的事件。
    *你可以通过按位与操作来检查特定的事件是否已经发生，并据此做出相应的处理逻辑。
    *例如，如果你等待的是 BIT0 | BIT1，但返回值表明 BIT0 | BIT2 被设置了，
    *那么你可以知道虽然没有等到 BIT1，但是 BIT2 却意外出现了，你的程序可以根据这个信息做出适当反应。
    */

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_STA, "connected to ap SSID:%s password:%s", 
                WIFI_STA_SSID, WIFI_STA_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s, password:%s", 
                WIFI_STA_SSID, WIFI_STA_PASSWORD);
    }else{
        ESP_LOGE(TAG_STA, "UNEXPECTED EVENT");
        return;
    }

    // 设置默认网络接口
    esp_netif_set_default_netif(esp_netif_sta);

    if(esp_netif_napt_enable(esp_netif_sta) != ESP_OK){
        ESP_LOGE(TAG_STA, "esp_netif_napt_enable failed");
        ESP_LOGE(TAG_STA, "NAPT not enabled on the netif: %p", esp_netif_ap);
    }
}