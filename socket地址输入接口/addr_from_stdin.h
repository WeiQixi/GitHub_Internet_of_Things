/*  Common utilities for socket address input interface:
    The API get_addr_from_stdin() is mainly used by socket client examples which read IP address from stdin (if configured).
    This option is typically used in the CI, but could be enabled in the project configuration.
    In that case this component is used to receive a string that is evaluated and processed to output
    socket structures to open a connectio
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

    通用工具函数：用于处理socket地址输入接口
    API函数get_addr_from_stdin()主要用于socket客户端示例，这些示例从标准输入(stdin)读取IP地址（如果配置为这样做的话）。
    这个选项通常在持续集成(CI)环境中使用，但也可以根据项目配置启用。
    在这种情况下，这个组件用于接收一个字符串，该字符串将被解析和处理，以输出用于建立连接的socket结构体。
    此示例代码属于公有领域（或者按您的选择，采用CC0许可）。
    除非适用法律另有要求或书面同意，否则此类软件将以“原样”基础分发，不附带任何明示或暗示的保证或条件。

    翻译解释：
    这一段描述了某个软件组件中关于处理socket地址输入的一个通用实用功能。
    特别是一个名为get_addr_from_stdin()的函数，它设计来帮助socket客户端示例程序从用户的键盘输入（标准输入流stdin）
    中读取IP地址信息。这种功能在自动化测试环境（如持续集成）中较为常见，但也能够根据项目的具体配置，在其他情境下启用。
    一旦从用户处获取到输入的地址信息，该组件会解析这些信息并转换成必要的socket结构，以便于建立网络连接。
    同时，再次重申了软件的授权方式及免责声明，强调了在无特定法律要求或协议约定的前提下，软件以无保证的形式提供。
 */

#pragma once

#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read and evaluate IP address from stdin
 *
 * This API reads stdin and parses the input address using getaddrinfo()
 * to fill in struct sockaddr_storage (for both IPv4 and IPv6) used to open
 * a socket. IP protocol is guessed from the IP address string.
 * 
 * 解释：
 * 这里描述的是一个API的功能，它的主要任务是从程序的标准输入中读取用户输入的IP地址，
 * 然后利用getaddrinfo()这个函数来分析并处理这个地址。getaddrinfo()是一个在许多编程环境中都可用的函数，
 * 它能够根据给定的主机名或IP地址及端口号等信息，填充一个结构体（在这里是struct sockaddr_storage），
 * 这个结构体包含了建立网络连接所需的所有地址信息，支持IPv4和IPv6两种协议。
 * 该API智能地识别输入的IP地址是属于IPv4还是IPv6，从而适配不同类型的网络环境，为后续创建socket连接做好准备。
 *
 * @param[in] port port number of expected connection                           //解释：函数调用时需要提供的一个输入参数——port。这里的[in]标记表示该参数是以输入为目的传递给函数的，即函数将使用该参数的值但不会修改它。
 *                                                                                      port number of expected connection说明这个参数是用来指定期望建立连接的端口号。
 * @param[in] sock_type expected protocol: SOCK_STREAM or SOCK_DGRAM            //解释：函数需要一个输入参数sock_type，用来指定期望使用的通信协议类型。
 * @param[out] ip_protocol resultant IP protocol: IPPROTO_IP or IPPROTO_IP6     //解释：此参数说明描述了一个输出参数ip_protocol，它用于接收函数执行后确定的IP协议类型。
 * @param[out] addr_family resultant address family: AF_INET or AF_INET6        //解释：此参数描述定义了一个输出参数addr_family，其作用在于返回函数执行后确定的地址家族类型。
 *                                                                                     在套接字编程中，AF_INET对应于IPv4地址族，而AF_INET6则对应于IPv6地址族。地址族决定了套接字可以处理的地址类型。
 * @param[out] dest_addr sockaddr_storage structure (for both IPv4 and IPv6)    //解释：此参数说明指出了一个输出参数dest_addr，它是一个sockaddr_storage结构体。
 *                                                                                      sockaddr_storage是一个足够大的结构体，能够容纳IPv4（sockaddr_in）和IPv6（sockaddr_in6）两种地址结构。
 *                                                                                      这意味着，无论解析输入地址后确定使用的是IPv4还是IPv6协议，dest_addr都能存储相应的地址信息。
 *                                                                                      此结构体通常用于在不确定目标地址是IPv4还是IPv6时，以一种统一的方式处理地址信息，提高代码的灵活性和兼容性。
 *                                                                                      函数将根据解析结果填充这个结构体，以便于进一步创建和配置套接字连接。
 * @return ESP_OK on success, ESP_FAIL otherwise
 */
esp_err_t get_addr_from_stdin(int port, int sock_type,
                              int *ip_protocol,
                              int *addr_family,
                              struct sockaddr_storage *dest_addr);

#ifdef __cplusplus
}
#endif
