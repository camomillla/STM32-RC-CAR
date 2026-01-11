/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : wifi_api.c
  * @brief          : ESP-05 simple REST API (/ping, /) + RC-Car endpoints
  ******************************************************************************
  */
/* USER CODE END Header */

#include "wifi_api.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "modules.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define RX_BUF_SIZE 1024

#define WIFI_STATIC_IP   "192.168.1.128"
#define WIFI_GATEWAY     "192.168.1.1"
#define WIFI_NETMASK     "255.255.255.0"

#define WIFI_SSID        "Maszt_5G"
#define WIFI_PASS        "KolotaKrul123"
/* USER CODE END Private defines */

/* USER CODE BEGIN Private variables */
static char rxbuf[RX_BUF_SIZE];
static uint16_t rxidx = 0;
static volatile uint8_t uart_rx_pause = 0;
/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */
static void esp_send(const char *cmd);
static void esp_init(void);
static void handle_ipd(char *data);
static void send_http(uint8_t id, const char *payload);
static int wait_for_string(const char *str, uint32_t timeout_ms);

static int is_ping_request(const char *data);
static int is_root_request(const char *data);

static int extract_get_path(const char *data, char *out, uint16_t out_size);
static int parse_cmd_from_path(const char *path, char out_cmd4[5]);
static int parse_motor_mode_from_path(const char *path);

static void send_json_state(uint8_t id, const char *cmd4, int ok, const char *msg);
static void send_json_telemetry(uint8_t id);
/* USER CODE END Private function prototypes */

/* USER CODE BEGIN Private functions */
static int is_ping_request(const char *data)
{
    const char *p = strstr(data, "GET ");
    if (!p) return 0;

    p += 4;
    if (strncmp(p, "/ping", 5) == 0)
        return 1;

    return 0;
}

static int is_root_request(const char *data)
{
    const char *p = strstr(data, "GET ");
    if (!p) return 0;

    p += 4;
    return (p[0] == '/' && p[1] == ' ');
}

static int wait_for_string(const char *str, uint32_t timeout_ms)
{
    uint8_t c;
    uint32_t idx = 0;
    uint32_t t0 = HAL_GetTick();

    while (HAL_GetTick() - t0 < timeout_ms)
    {
        if (HAL_UART_Receive(&huart2, &c, 1, 10) == HAL_OK)
        {
            if (c == (uint8_t)str[idx])
            {
                idx++;
                if (str[idx] == 0)
                    return 1;
            }
            else
            {
                idx = 0;
            }
        }
    }
    return 0;
}

static void esp_send(const char *cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
    HAL_Delay(300);
}

static void esp_init(void)
{
    char cmd[128];

    esp_send("AT+RST\r\n");
    HAL_Delay(1500);

    esp_send("AT+CWMODE=1\r\n");

    sprintf(cmd, "AT+CIPSTA=\"%s\",\"%s\",\"%s\"\r\n", WIFI_STATIC_IP, WIFI_GATEWAY, WIFI_NETMASK);
    esp_send(cmd);

    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASS);
    esp_send(cmd);
    HAL_Delay(5000);

    esp_send("AT+CIPMUX=1\r\n");
    esp_send("AT+CIPSERVER=1,80\r\n");
}

static void send_http(uint8_t id, const char *payload)
{
    char cmd[64];

    uart_rx_pause = 1;

    uint8_t dump;
    while (HAL_UART_Receive(&huart2, &dump, 1, 5) == HAL_OK) {}

    sprintf(cmd, "AT+CIPSEND=%d,%d\r\n", id, (int)strlen(payload));
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);

    if (!wait_for_string(">", 2000))
        goto exit;

    HAL_UART_Transmit(&huart2, (uint8_t*)payload, strlen(payload), HAL_MAX_DELAY);

    if (!wait_for_string("SEND OK", 2000))
        goto exit;

    sprintf(cmd, "AT+CIPCLOSE=%d\r\n", id);
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);

exit:
	sprintf(cmd, "AT+CIPCLOSE=%d\r\n", id);
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
    uart_rx_pause = 0;
}

static int extract_get_path(const char *data, char *out, uint16_t out_size)
{
    const char *p = strstr(data, "GET ");
    if (!p) return 0;

    p += 4;
    const char *e = strchr(p, ' ');
    if (!e) return 0;

    uint32_t len = (uint32_t)(e - p);
    if (len == 0) return 0;

    if (len >= (uint32_t)out_size)
        len = (uint32_t)out_size - 1;

    memcpy(out, p, len);
    out[len] = 0;
    return 1;
}

static int parse_motor_mode_from_path(const char *path)
{
    if (strncmp(path, "/motor/", 7) == 0)
    {
        int v = atoi(path + 7);
        if (v >= 1 && v <= 9) return v;
        return -1;
    }

    const char *q = strchr(path, '?');
    if (!q) return -1;

    const char *m = strstr(q, "mode=");
    if (m)
    {
        int v = atoi(m + 5);
        if (v >= 1 && v <= 9) return v;
        return -1;
    }

    m = strstr(q, "m=");
    if (m)
    {
        int v = atoi(m + 2);
        if (v >= 1 && v <= 9) return v;
        return -1;
    }

    return -1;
}

static int parse_cmd_from_path(const char *path, char out_cmd4[5])
{
    out_cmd4[0] = 0;

    if (strncmp(path, "/cmd?c=", 7) == 0)
    {
        const char *c = path + 7;
        if (strlen(c) < 4) return 0;

        out_cmd4[0] = c[0];
        out_cmd4[1] = c[1];
        out_cmd4[2] = c[2];
        out_cmd4[3] = c[3];
        out_cmd4[4] = 0;
        return 1;
    }

    if (strcmp(path, "/engine") == 0) { memcpy(out_cmd4, "CMD0", 5); return 1; }
    if (strcmp(path, "/lights") == 0) { memcpy(out_cmd4, "CMDA", 5); return 1; }
    if (strcmp(path, "/horn")   == 0) { memcpy(out_cmd4, "CMDB", 5); return 1; }
    if (strcmp(path, "/abs")    == 0) { memcpy(out_cmd4, "CMDC", 5); return 1; }
    if (strcmp(path, "/init")   == 0) { memcpy(out_cmd4, "INIT", 5); return 1; }

    if (strncmp(path, "/motor", 6) == 0)
    {
        int mode = parse_motor_mode_from_path(path);
        if (mode >= 1 && mode <= 9)
        {
            out_cmd4[0] = 'C';
            out_cmd4[1] = 'M';
            out_cmd4[2] = 'D';
            out_cmd4[3] = (char)('0' + mode);
            out_cmd4[4] = 0;
            return 1;
        }
        return 0;
    }

    return 0;
}

static void send_json_state(uint8_t id, const char *cmd4, int ok, const char *msg)
{
    char body[256];
    char http[512];

    if (!msg) msg = "";

    snprintf(body, sizeof(body),
        "{\"ok\":%d,\"cmd\":\"%s\",\"engine\":%d,\"horn\":%d,\"ind\":%d,\"dist\":%lu,\"a_set\":%d,\"b_set\":%d,\"a_meas\":%d,\"b_meas\":%d,\"msg\":\"%s\"}",
        ok ? 1 : 0,
        cmd4 ? cmd4 : "",
        engineOn,
        hornOn,
        indicationOverride,
        (unsigned long)Distance,
        motorA.set_speed,
        motorB.set_speed,
        motorA.measured_speed,
        motorB.measured_speed,
        msg
    );

    snprintf(http, sizeof(http),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n%s",
        (int)strlen(body), body
    );

    send_http(id, http);
}

static void send_json_telemetry(uint8_t id)
{
    char body[256];
    char http[512];

    snprintf(body, sizeof(body),
        "{\"ok\":1,\"a_set\":%d,\"b_set\":%d,\"a_meas\":%d,\"b_meas\":%d}",
        motorA.set_speed,
        motorB.set_speed,
        motorA.measured_speed,
        motorB.measured_speed
    );

    snprintf(http, sizeof(http),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n%s",
        (int)strlen(body), body
    );

    send_http(id, http);
}

static void handle_ipd(char *data)
{
    char *p = strstr(data, "+IPD,");
    if (!p) return;

    p += 5;
    int id = 0;
    while (*p >= '0' && *p <= '9')
    {
        id = id * 10 + (*p - '0');
        p++;
    }

    if (is_ping_request(data))
    {
        send_http((uint8_t)id,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 8\r\n"
            "Connection: close\r\n\r\n"
            "{\"ok\":1}");
        return;
    }

    if (is_root_request(data))
    {
        const char *body =
            "<!DOCTYPE html>"
            "<html><body style='margin:0;background:#00aa00'></body></html>";

        char http[256];
        sprintf(http,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n\r\n%s",
            (int)strlen(body), body);

        send_http((uint8_t)id, http);
        return;
    }

    {
        char path[128];
        if (!extract_get_path(data, path, sizeof(path)))
        {
            send_json_state((uint8_t)id, "", 0, "no GET path");
            return;
        }

        if (strcmp(path, "/telemetry") == 0)
        {
        	send_json_telemetry((uint8_t)id);
        	return;
        }

        char cmd4[5];
        if (parse_cmd_from_path(path, cmd4))
        {
            ProcessCommand((uint8_t*)cmd4);
            send_json_state((uint8_t)id, cmd4, 1, "");
            return;
        }

        send_http((uint8_t)id,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 3\r\n"
            "Connection: close\r\n\r\n"
            "404");
    }
}
/* USER CODE END Private functions */

void WifiApi_Init(void)
{
    rxidx = 0;
    memset(rxbuf, 0, RX_BUF_SIZE);
    uart_rx_pause = 0;

    esp_init();
}

void WifiApi_Process(void)
{
    uint8_t c;

    if (!uart_rx_pause &&
        HAL_UART_Receive(&huart2, &c, 1, 10) == HAL_OK)
    {
        rxbuf[rxidx++] = (char)c;
        rxbuf[rxidx] = 0;

        if (strstr(rxbuf, "+IPD"))
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // LD1
        }

        if (strstr(rxbuf, "\r\n\r\n"))
        {
            handle_ipd(rxbuf);
            rxidx = 0;
            memset(rxbuf, 0, RX_BUF_SIZE);
        }

        if (rxidx >= RX_BUF_SIZE - 1)
            rxidx = 0;
    }
}
