/* MQTT over Websockets Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

/*
*这个文件通常包含了与协议示例相关的通用功能、宏定义、数据结构、函数声明或初始化代码，
*旨在为一系列协议示例或演示程序提供共享的基础框架和工具集。
*
*在ESP-IDF（Espressif IoT Development Framework）环境中，
*特别是在涉及网络协议或通信协议（如Modbus、MQTT、HTTP、TCP/IP等）的示例项目里，
*protocol_examples_common.h可能包含了网络配置助手函数、日志;记录设置、错误处理宏、
*基础的网络连接初始化代码或者是硬件抽象层接口，以便于简化示例代码，使其更专注于演示具体协议的使用逻辑，
*而不必重复实现每一处的基础设施代码。

*具体这个头文件包含的内容会根据项目的具体实现和设计意图有所不同，
*但其核心思想是为了提高代码的模块化、重用性和维护性。
*开发者在编写基于特定协议的应用时，应该查阅该项目或框架的文档以了解protocol_examples_common.h确切提供的功能和如何正确使用这些工具。
*/
#include "protocol_examples_common.h"

/*
*这些头文件都是FreeRTOS实时操作系统（Real Time Operating System）框架的一部分，
*广泛用于嵌入式系统开发，特别是那些对时间敏感的应用。下面是每个头文件的大致含义和它们提供的主要功能：
*
*FreeRTOS.h：
*这是FreeRTOS的核心头文件，包含了FreeRTOS的基本类型定义、宏定义和全局函数原型声明。
*它提供了访问FreeRTOS内核功能的入口点，比如任务（线程）、消息队列、信号量、互斥锁、事件组等。
*几乎每一个使用FreeRTOS的项目都会包含这个头文件。
*
*task.h：
*此头文件专注于任务（Task）相关的功能。它定义了任务创建、删除、挂起、恢复、切换、延时等功能的函数原型，
*以及与任务优先级、状态相关的类型和宏。如果你的代码中需要创建或管理任务，就需要包含这个头文件。
*
*semphr.h：
*信号量（Semaphore）和互斥锁（Mutex）相关的头文件。
*它包含了创建、获取、释放信号量、二进制信号量、计数信号量、互斥锁等函数原型。
*信号量是FreeRTOS中用于同步和保护共享资源的重要机制。
*
*queue.h：
*队列（Queue）相关的头文件。它定义了消息队列、邮箱、环形缓冲区等数据结构的操作函数原型。
*消息队列是任务间通信的主要手段之一，允许任务安全地发送和接收数据，即使发送和接收的任务处于不同的优先级。
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

/*
*这三个头文件均来自LwIP库，这是一个轻量级的TCP/IP协议栈，常用于嵌入式系统和物联网设备中。
*下面是每个头文件的大致用途和包含的主要内容：
*
*lwip/sockets.h:
*此头文件包含了标准Berkeley套接字（BSD Sockets）API的实现。
*在LwIP中，这个接口允许应用程序以一种跨平台且标准化的方式来执行网络编程任务，
*如创建套接字、绑定端口、监听、接受连接、发送和接收数据等。它为TCP和UDP通信提供了基础。
*
*lwip/dns.h:
*DNS（域名系统）相关的头文件。
*它提供了查询域名解析的功能，允许应用程序将人类可读的域名转换为对应的IP地址，
*或者执行反向查询从IP地址得到域名。这在需要根据域名连接远程服务或进行网络诊断时非常有用。
*
*lwip/netdb.h:
*网络数据库头文件。它定义了结构和函数，用于处理网络服务数据库，
*如hostent结构体，这是解析主机名查询结果的标准数据结构，
*包含了主机名、别名列表、地址类型、地址列表等信息。通过这个头文件，开发者可以更方便地处理从DNS查询中返回的复杂数据。
*
*总结起来，这些头文件为基于LwIP的嵌入式系统提供了网络编程的基本构建块，
*从创建网络连接到解析域名，再到高级的网络信息查询，帮助开发者构建功能丰富的网络应用。
*/
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

/*日志头文件*/
#include "esp_log.h"

/*#include "mqtt_client.h" 这一行代码表明你的程序将包含MQTT客户端库的头文件。
*MQTT（Message Queuing Telemetry Transport）是一种轻量级的发布/订阅式消息传输协议，
*广泛应用于物联网（IoT）设备间的数据通信，因其低带宽消耗和低功耗特性而特别适合资源受限的环境。
*
*mqtt_client.h头文件通常包含了MQTT客户端所需的所有函数声明、数据类型定义、宏定义以及结构体等，
*允许你的程序创建MQTT客户端实例、连接MQTT代理服务器、订阅主题、发布消息以及处理接收到的消息等操作。
*这个库可能是一个独立的实现，也可能集成在某个更大的框架或SDK中，比如ESP-IDF（Espressif物联网开发框架）就包含了对MQTT的支持。
*
*使用这个头文件时，你需要确保相应的MQTT客户端库已经正确安装或集成到了你的项目中，
*并且在编译链接阶段能够找到对应的实现库文件。通过这个接口，
*你可以构建基于MQTT协议的物联网应用，实现设备与设备间、设备与云平台间的可靠消息交互。
*/
#include "mqtt_client.h"

/*在C语言编程中，这样的定义通常用于日志记录或者错误信息输出时作为标记使用，以便于在查看日志时能迅速识别消息来源于哪个部分或模块*/
static const char *TAG = "MQTTWS_EXAMPLE";

/*
* @brief 使用if语句检查error_code是否不等于0。如果不等于0，说明发生了错误。
*        调用ESP_LOGE函数记录错误日志。ESP_LOGE是ESP-IDF（Espressif IoT Development Framework，
*        一个专为ESP32和ESP8266等芯片设计的物联网开发框架）提供的日志宏，用于输出错误级别的日志。
*        这里的TAG（之前定义的字符串）会被用作日志的标签，message作为描述信息紧跟其后，
*        而0x%x是格式化字符串，会以十六进制形式输出error_code的值。这样，当错误发生时，
*        可以通过日志快速定位到错误源头和具体的错误代码。
* @param const char *message: 函数参数之一，是一个指向常量字符数组的指针，用于传递描述错误情况的文本信息。
*        const表示这个指针指向的字符串内容在函数内部不会被修改。
* @param int error_code: 函数的另一个参数，类型为整型，用于传递需要检查的错误代码。如果这个值不等于0，通常意味着有错误发生。
* @return void
*/
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *  用于接收MQTT事件的事件处理器
 *  This function is called by the MQTT client event loop.
 *  此函数由MQTT客户端事件循环调用。
 * @param handler_args user data registered to the event.
 *        注册到事件的用户数据。
 * @param base Event base for the handler(always MQTT Base in this example).
 *        事件处理器的基础（在此示例中始终为MQTT基础）。
 * @param event_id The id for the received event.
 *        接收到的事件的id。
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 *        与事件相关的数据，类型为esp_mqtt_event_handle_t。
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    /*
    * ESP_LOGD(TAG, "从事件循环分发的事件 基础=%s, 事件ID=%" PRIi32, base, event_id);

    * 这段代码是一个日志记录语句，用于在调试级别输出信息。它表示一个事件已经从事件循环中分发出来，并提供了关于该事件的一些详细信息：

    * TAG 是一个宏，通常代表当前日志消息的来源或模块名称，用于在日志中分类和过滤消息。
    * "Event dispatched from event loop base=%s, event_id=%" PRIi32 是日志消息的格式字符串，其中 %s 会被 base 参数的值替换，
    *           表示事件处理的基础（即事件源），%" PRIi32 会被 event_id 参数的值替换，并以十进制整数形式显示事件的唯一标识符。
    * base 参数指出了产生此事件的事件基础，这是一个字符串，用来标识事件的来源，在这个上下文中应该是 "MQTT"。
    * event_id 参数是一个整数值，表示接收到的具体事件类型，这有助于程序决定如何响应这一事件。
    * 总之，这条日志消息表明程序正在调试模式下运行，并记录了某个MQTT相关事件被处理的细节，包括事件来源于哪个基础以及事件的编号，
    * 这对于跟踪和理解程序中MQTT事件处理流程非常有帮助。
    */
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);

    /*
    * 这行代码进行了一个类型转换（或者说是变量赋值），将event_data指针所指向的数据强制转换（或简单地赋值给）
    * 为esp_mqtt_event_handle_t类型的变量event。这里假设event_data是指向MQTT事件数据结构的指针，
    * 该结构包含了关于MQTT事件的所有必要信息，如连接状态改变、消息发布或接收等。
    */
    esp_mqtt_event_handle_t event = event_data;

    /*
    * 从当前处理的MQTT事件(event)中获取到关联的MQTT客户端句柄，并将其存储在名为client的变量中。
    * 这个client变量后续可以用来直接操作或查询该MQTT客户端的状态、发送消息、订阅主题等，是进行进一步MQTT客户端相关操作的必要标识。
    */
    esp_mqtt_client_handle_t client = event->client;

    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        /*
        * @brief 这是在MQTT连接建立成功后立即执行的一个操作，用于发布一条MQTT消息。其各参数含义如下：
        * 
        * @param client: 已经连接成功的MQTT客户端句柄，之前通过esp_mqtt_client_handle_t client = event->client;获得。
        * @param "/topic/qos1": 消息要发布的主题。在这个例子中，消息将被发送到名为/topic/qos1的主题上。
        * @param "data_3": 实际要发送的消息内容，这里是字符串"data_3"。
        * @param 0: 保留位，通常设为0，MQTT规范允许该字段供未来扩展使用。
        * @param 1: QoS(服务质量)等级，这里的1表示至少一次的传输保证，即消息将至少送达一次给订阅者，但有可能会重复。
        * @param 0: 消息标识符的分配方式标志，0表示让MQTT客户端自动为这条消息分配一个唯一的消息ID，如果需要手动控制消息ID，
        *           可以设置为非0值并提供一个具体的ID。
        * @return msg_id变量用于存储返回的消息ID，这在某些情况下很有用，比如如果你想跟踪消息的发布确认或者取消尚未发送的消息。
        * 
        */
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        /*
        * @brief ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id); 
        *        这行代码的作用是在日志中记录一个信息（Information）级别的消息，
        *        用来指示一个MQTT发布操作已经成功完成。具体来说：
        *        "sent publish successful, msg_id=%d" 是日志消息的格式化字符串，其中 %d 是一个占位符，
        *        将会被后面的 msg_id 参数的值替换。
        *        msg_id 是之前调用 esp_mqtt_client_publish 函数发布消息时返回的消息ID。当这行日志被执行时，
        *        它会显示发送发布的消息已成功，并给出该消息的ID，这对于跟踪消息的发送状态或后续可能的错误排查非常有用。
        *        综上所述，该日志条目表明一个MQTT消息已经成功发送出去，并提供了该消息的唯一标识符（message ID），
        *        便于进一步的追踪或确认。
        */
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        /*
        * @brief 订阅主题：
        *        msg_id：此变量将接收订阅请求的返回消息ID。此ID可用于跟踪订阅请求的结果，
        *                例如在某些实现中，可以通过这个ID来取消未完成的订阅请求或者匹配来自服务器的SUBACK消息。
        *        esp_mqtt_client_subscribe：这是ESP-IDF框架中的函数，用于发起一个MQTT客户端的订阅请求，
        *                                   向MQTT服务器表明客户端希望接收指定主题上的消息。
        *        client：这是之前建立并成功连接MQTT服务器的客户端句柄，用于指定执行订阅操作的客户端实例。
        *        "/topic/qos0"：这是要订阅的MQTT主题的名称。客户端将开始接收任何发布到这个主题上的消息。
        *                       在这个例子中，主题名为/topic/qos0。
        *        0：这个参数指定了订阅时要求的服务质量（QoS）级别。在这里设置为0，意味着QoS 0级别，
        *           也即是最多一次的交付保证。这意味着消息可能会丢失，但也是最高效的传输方式，适用于对消息丢失不敏感的场景。
        *        执行此函数后，客户端会向MQTT服务器发送一个SUBSCRIBE报文，请求订阅指定主题。
        *        服务器通常会回复一个SUBACK报文，确认订阅请求，并告知最终协商的QoS级别。
        *        msg_id 可以用来追踪这个订阅请求的过程和结果。
        */
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);

        /*
        * 这行代码用于记录日志信息，表明一个MQTT订阅请求已经被成功发送出去。具体解析如下：
        * "sent subscribe successful, msg_id=%d": 这是一个格式化字符串，其中%d是一个占位符，
        * 将被后面的msg_id变量的值替换。整个字符串的意思是“发送订阅请求成功，消息ID为...”。
        * msg_id: 这是在调用esp_mqtt_client_subscribe函数订阅MQTT主题时返回的消息ID。当MQTT客户端发送订阅请求到服务器时，
        * 服务器会响应一个SUBACK消息，这个ID可以用来追踪该订阅请求的成功与否，虽然在某些情况下，如果没有收到SUBACK，这个ID也可能用于故障排查。
        * 
        * 综上，该日志条目告知开发者或维护人员，客户端已经成功发起了一个MQTT主题订阅请求，并且提供了请求的标识符msg_id，
        * 这对于监控系统运行状况和处理后续的订阅确认非常有用。
        */
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

        /*
        * @brief 这行代码是用来发布MQTT消息的。具体说明如下：
        *        msg_id：此变量将接收发布消息操作的返回消息ID。这个ID可以用于跟踪消息发送的状态，
        *        例如，确认消息是否被发送服务端接收到，或者在某些情况下取消尚未发送的消息。
        * @param esp_mqtt_client_publish：这是ESP-IDF框架中的函数，用于MQTT客户端向MQTT服务器发布消息。
        * @param client：这是之前创建并成功连接到MQTT服务器的客户端句柄，指定执行消息发布的客户端实例。
        * @param "/topic/qos0"：这是消息将要发布到的MQTT主题。所有订阅了此主题的客户端都会收到这条消息。
        * @param "data"：这是实际要发送的数据内容，本例中发送的是字符串"data"。
        * @param 0：保留位，MQTT协议规定的，一般情况下设为0，留给未来扩展使用。
        * @param 0：服务质量（QoS）等级，这里的0表示“至多一次”的传输保证，即消息可能会丢失，且不会重复发送，适用于对消息可靠性和顺序要求不高的场景。
        * @param 0：是否等待发送完成的标志，0表示不等待发送完成就立即返回，消息发送异步进行。如果需要等待发送完成并获取发送结果，可以设置为非0值。
        * @return 可以用来追踪这条消息的发送状态。
        */
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

        /*
        * 代表客户端已成功取消订阅了某个MQTT主题。
        * 当你的应用程序接收到带有此事件ID的通知时，意味着MQTT客户端之前发送的取消订阅请求（通过 esp_mqtt_client_unsubscribe 函数）
        * 已经得到了服务器的确认，从此之后，该客户端将不再接收来自该已取消订阅主题的任何消息。
        * 这对于管理客户端的订阅状态和减少不必要的网络流量是非常重要的。
        */
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

        /*
        * MQTT_EVENT_PUBLISHED 的翻译为“已发布事件”，在MQTT通信中，这个事件标志着一个重要的步骤：
        * 当客户端通过MQTT协议向服务器发送一条消息后，服务器成功接收到这条消息并进行了确认，客户端就会收到MQTT_EVENT_PUBLISHED事件通知。
        * 这意味着你尝试发布的数据已经安全到达MQTT服务器端，接下来服务器会负责将这条消息按需转发给所有订阅了相应主题的客户端。
        * 在编程处理中，接收到这个事件通常会用来进行一些后续逻辑处理，比如记录日志、更新状态或者触发下一个任务等。
        */
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

        /*
        * MQTT_EVENT_DATA: 这个事件表示MQTT客户端收到了一个主题上的新消息。
        * 当你的程序监听到这个事件时，意味着有数据从MQTT服务器发来，并且是针对你之前订阅过的某个主题。
        */
    case MQTT_EVENT_DATA:
        /*
        * 日志的内容——客户端接收到MQTT数据。
        */
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");

        /*
        * printf("TOPIC=%.*s\r\n", event->topic_len, event->topic); 这行代码打印了收到消息的主题。
        * %.*s是一个格式化字符串，其中.*表示输出字符串时，前面的整数（在这里是event->topic_len）决定了从地址event->topic开始读取多少个字符作为字符串输出。
        * \r\n是换行符，用于在终端上换行显示。这行代码让你知道消息是来自哪个MQTT主题。
        */
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);

        /*
        * printf("DATA=%.*s\r\n", event->data_len, event->data); 类似地，这行代码打印了消息的实际数据内容。
        * event->data_len指定了消息数据的长度，而event->data是消息数据的指针。这样，你可以看到该主题上的具体消息内容是什么。
        */
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");

        /*
        * MQTT_ERROR_TYPE_TCP_TRANSPORT 是一个常量，表示错误属于TCP传输层类别。
        * 整个判断就是在检查当前MQTT通信中遇到的错误是否源于TCP传输层面的问题，比如网络连接失败、数据包丢失、连接超时等与底层网络传输相关的错误。
        * 如果这个判断为真，那么就意味着需要处理与TCP传输相关的错误，可能需要重新建立连接、优化网络设置或检查网络状况等。
        */
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {

            /*
            * 函数调用：调用了log_error_if_nonzero函数，传入了两个参数：
            *           第一个参数是字符串"reported from esp-tls"，作为错误信息的一部分，用来描述错误来源，
            *               这里指的是错误来自于ESP-TLS层，ESP-TLS是ESP-IDF中用于实现TLS/SSL安全传输的组件。
            *           第二个参数event->error_handle->esp_tls_last_esp_err是从MQTT事件的错误处理结构中提取的ESP-TLS的最后一个错误代码。
            *               esp_tls_last_esp_err记录了与TLS/SSL握手或通信相关的最近错误。
            */
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);

            /*
            * 函数调用: log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            * 第一个参数："reported from tls stack" 是一个描述性信息，用于在日志中标识错误来源，
            *       这里指的是错误源自TLS协议栈（Transport Layer Security，传输层安全协议）。
            * 第二个参数：event->error_handle->esp_tls_stack_err 是从MQTT事件的错误处理结构中提取的特定于TLS堆栈的错误代码。
            *       这个字段通常包含了与TLS握手、证书验证、加密解密等TLS协议层相关的错误信息。
            */
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);

            /*
            * esp_transport_sock_errno 是ESP-IDF（Espressif IoT Development Framework）中与网络传输层相关的错误号变量。
            *     它反映了最近一次网络套接字（socket）操作失败的原因。在进行TCP、UDP等基于套接字的网络通信时，
            *     如果发生了错误（比如连接失败、数据发送/接收错误等），相应的错误代码就会被设置到esp_transport_sock_errno中。
            * 
            * 这个错误号遵循POSIX标准的errno错误代码约定，允许开发者根据错误码判断具体的问题所在，
            * 比如ECONNREFUSED（连接被拒绝）、ETIMEDOUT（连接超时）等。通过检查esp_transport_sock_errno的值，
            * 并使用如perror()或查阅相关文档，开发者能够识别错误原因并采取相应的处理措施。
            * 
            */
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            
            /*
            * strerror(event->error_handle->esp_transport_sock_errno)：这是用来获取错误描述的函数调用。
            * strerror是一个标准C库函数，它接受一个错误编号（在这里是event->error_handle->esp_transport_sock_errno，即网络套接字操作的错误编号）
            * 并返回一个描述该错误的字符串。这使得日志信息更加易读，便于理解错误的具体原因。
            * 
            * 综合来看，该代码段的功能是：当检测到与网络套接字操作相关的错误时，记录一条日志信息，展示该错误的详细描述文本，有助于开发者诊断和解决问题。
            * 
            */
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    /*
    * esp_mqtt_client_config_t：这是ESP-IDF框架定义的一个数据结构类型，用于存储MQTT客户端的各种配置信息，如代理地址、端口、用户名、密码等。
    * .broker.address.uri = CONFIG_BROKER_URI：这部分配置了MQTT代理的地址信息。
    * .broker.address.uri是结构体esp_mqtt_client_config_t内嵌套成员之一，用于指定MQTT代理的网络地址，
    *    它可以是一个URL字符串，如"mqtt://example.com"。CONFIG_BROKER_URI则是一个宏，其值通常在项目的配置文件中定义，
    *    例如在ESP-IDF环境中，这可能是通过KConfig系统在sdkconfig.h文件中定义的，允许用户灵活配置MQTT代理的实际地址而不硬编码在源代码中。
    */
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URI,
    };

    /*
    * esp_mqtt_client_handle_t client: 定义了一个变量client，
    *     其类型为esp_mqtt_client_handle_t。这个类型是一个句柄类型，用于唯一标识一个MQTT客户端实例。
    * esp_mqtt_client_init: 这是ESP-IDF框架提供的一个函数，用于根据给定的配置初始化一个MQTT客户端实例。
    *    它需要一个指向esp_mqtt_client_config_t结构体的指针作为参数，这个结构体包含了MQTT客户端的所有配置信息，比如代理地址、客户端ID、用户名、密码等。
    * &mqtt_cfg: 这是mqtt_cfg结构体变量的地址。由于esp_mqtt_client_init函数需要一个指向配置结构体的指针，因此使用取地址运算符&获取了结构体变量的地址。
    * 
    * 综上所述，这行代码的作用是基于之前定义的mqtt_cfg配置信息创建一个新的MQTT客户端实例，
    *    并通过client变量持有这个客户端的句柄，以便后续进行连接、发布消息、订阅主题等MQTT操作。
    */
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    /*
    * @brief 是ESP-IDF框架中的函数，用于注册MQTT客户端的事件监听。当指定的事件发生时，它会调用提供的事件处理器函数。
    * @param 是之前通过 esp_mqtt_client_init 初始化得到的MQTT客户端句柄，表示要为其注册事件处理函数的客户端实例。
    * @param ESP_EVENT_ANY_ID 是一个特殊的事件ID，表示注册的事件处理器将接收所有类型的MQTT事件，无论是连接、断开、消息发布、订阅、取消订阅还是错误等。
    * @param 是事件处理器函数的名称，当有事件触发时，这个函数会被调用去处理这些事件。
    * @param NULL 是该函数调用中的最后一个参数，根据之前的注释，这个位置本来可以用来向事件处理器传递数据。
    *        但在这里，因为使用了NULL，表示没有传递额外的数据给处理器。在实际应用中，你可以替换NULL为一个指向你想要传递数据的指针，
    *        以便在事件处理器中访问和使用这些数据。
    * 
    * 总结起来，这段代码注册了一个事件处理器，它对MQTT客户端的所有事件都感兴趣，并且在处理这些事件时没有附带额外的用户数据。
    */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

    /*
    * esp_mqtt_client_start(client); 这行代码的作用是启动一个之前已经初始化但尚未激活的MQTT客户端。
    * 解释如下：
    * esp_mqtt_client_start: 这是ESP-IDF（Espressif IoT Development Framework）中用于MQTT客户端操作的API函数。
    *                        它负责开始或激活一个MQTT客户端实例，使其开始尝试与MQTT代理（Broker）建立连接，并开始处理网络通信，
    *                        包括发送和接收MQTT消息、处理订阅和取消订阅请求等。
    * client: 这是一个esp_mqtt_client_handle_t类型的参数，代表了之前通过esp_mqtt_client_init函数初始化得到的MQTT客户端实例句柄。
    *         这个句柄是用于后续所有对该MQTT客户端操作的标识，包括启动、停止、发送消息、订阅主题等。
    * 综上所述，调用esp_mqtt_client_start(client);后，指定的MQTT客户端会开始其生命周期的关键步骤：连接到MQTT代理，一旦连接成功，
    *         即可开始发布消息、订阅主题、接收消息等MQTT协议允许的所有操作。这是MQTT客户端从配置阶段迈向实际运作的关键一步。
    */
    esp_mqtt_client_start(client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_WS", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
}
