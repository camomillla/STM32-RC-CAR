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
#include <stdint.h>

#include "modules.h"
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define RX_BUF_SIZE   4096
#define REQ_BUF_SIZE  1600

#define WIFI_STATIC_IP   "192.168.8.127"
#define WIFI_GATEWAY     "192.168.8.1"
#define WIFI_NETMASK     "255.255.255.0"

#define WIFI_SSID        "HUAWEI-7975"
#define WIFI_PASS        "40720250"
/* USER CODE END Private defines */

/* USER CODE BEGIN Private variables */
static char rxbuf[RX_BUF_SIZE];
static uint16_t rxlen = 0;
static volatile uint8_t uart_rx_pause = 0;

/* Root page served under GET / (kept small to fit ESP AT CIPSEND limits). */
static const char ROOT_HTML[] =
    "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=devi"
    "ce-width,initial-scale=1'><title>Motor Telemetry</title><style>body{margin:0;font-family:s"
    "ystem-ui,Segoe UI,Roboto,Arial;background:#0b0f14;color:#e7eefc}#top{padding:10px 12px;bor"
    "der-bottom:1px solid rgba(255,255,255,.08);display:flex;gap:12px;align-items:center;flex-w"
    "rap:wrap}.k{color:#a8b3c7;font-size:12px}canvas{width:100%;height:240px;display:block}</st"
    "yle></head><body><div id='top'><b>Telemetry</b><span class='k'>A: <span id='a'>0</span></spa"
    "n><span class='k'>B: <span id='b'>0</span></span><span class='k'>(/telemetry)</span></div><"
    "canvas id='c' width='600' height='240'></canvas><script>(()=>{let c=document.getElementById"
    "('c'),x=c.getContext('2d'),A=[],B=[],N=120;function add(a,b){A.push(a);B.push(b);if(A.lengt"
    "h>N){A.shift();B.shift()}}function draw(){let w=c.width,h=c.height,p=10,xx=w-2*p,yy=h-2*p;"
    "x.clearRect(0,0,w,h);let m=A.length;if(m<2){requestAnimationFrame(draw);return}let lo=1e9,"
    "hi=-1e9;for(let i=0;i<m;i++){let a=A[i],b=B[i];if(a<lo)lo=a;if(a>hi)hi=a;if(b<lo)lo=b;if(b"
    ">hi)hi=b}if(lo==hi){lo-=1;hi+=1}let r=hi-lo;function line(ar,col){x.strokeStyle=col;x.begin"
    "Path();for(let i=0;i<m;i++){let X=p+i/(N-1)*xx;let Y=p+(hi-ar[i])/r*yy;if(i)x.lineTo(X,Y);e"
    "lse x.moveTo(X,Y)}x.stroke()}line(A,'#6ea8fe');line(B,'#33d69f');requestAnimationFrame(draw"
    ")}setInterval(()=>fetch('/telemetry?t='+Date.now(),{cache:'no-store'}).then(r=>r.json()).th"
    "en(j=>{let a=j.a_meas||0,b=j.b_meas||0;document.getElementById('a').textContent=a;document.g"
    "etElementById('b').textContent=b;add(+a,+b)}).catch(()=>{}),200);draw()})();</script></body"
    "></html>";

static char root_http[2400];
static char reqbuf[REQ_BUF_SIZE];
/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */
static void esp_send(const char *cmd);
static void esp_init(void);

static void rx_push_byte(uint8_t c);
static void rx_trim_to_tail(uint16_t keep);
static void process_rx_buffer(void);

static void send_http(uint8_t id, const char *payload);
static int wait_for_string(const char *str, uint32_t timeout_ms);

static int is_ping_request(const char *data);
static int is_root_request(const char *data);
static int is_favicon_request(const char *data);

static int extract_get_path(const char *data, char *out, uint16_t out_size);
static int parse_cmd_from_path(const char *path, char out_cmd4[5]);
static int parse_motor_mode_from_path(const char *path);

static void send_json_state(uint8_t id, const char *cmd4, int ok, const char *msg);
static void send_json_telemetry(uint8_t id);
/* USER CODE END Private function prototypes */

/* USER CODE BEGIN Private functions */
static void rx_push_byte(uint8_t c)
{
    if (rxlen >= RX_BUF_SIZE - 1)
    {
        rx_trim_to_tail(512);
    }

    rxbuf[rxlen++] = (char)c;
    rxbuf[rxlen] = 0;
}

static void rx_trim_to_tail(uint16_t keep)
{
    if (keep >= RX_BUF_SIZE - 1) keep = RX_BUF_SIZE - 1;

    if (rxlen <= keep) return;

    uint16_t drop = (uint16_t)(rxlen - keep);
    memmove(rxbuf, rxbuf + drop, keep);
    rxlen = keep;
    rxbuf[rxlen] = 0;
}

static int is_ping_request(const char *data)
{
    return (strncmp(data, "GET /ping", 9) == 0);
}

static int is_root_request(const char *data)
{
    return (strncmp(data, "GET / ", 6) == 0);
}

static int is_favicon_request(const char *data)
{
    return (strncmp(data, "GET /favicon.ico", 16) == 0);
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
            rx_push_byte(c);

            if (c == (uint8_t)str[idx])
            {
                idx++;
                if (str[idx] == 0)
                    return 1;
            }
            else
            {
                idx = (c == (uint8_t)str[0]) ? 1 : 0;
            }
        }
    }
    return 0;
}

static void esp_send(const char *cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
    HAL_Delay(200);
}

static void esp_init(void)
{
    char cmd[128];

    esp_send("AT+RST\r\n");
    HAL_Delay(1500);

    esp_send("ATE0\r\n");
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

    sprintf(cmd, "AT+CIPSEND=%d,%d\r\n", id, (int)strlen(payload));
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);

    if (!wait_for_string(">", 2000))
        goto done;

    HAL_UART_Transmit(&huart2, (uint8_t*)payload, strlen(payload), HAL_MAX_DELAY);

    wait_for_string("SEND OK", 3000);

    sprintf(cmd, "AT+CIPCLOSE=%d\r\n", id);
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);

done:
    uart_rx_pause = 0;
}

static int extract_get_path(const char *data, char *out, uint16_t out_size)
{
    if (strncmp(data, "GET ", 4) != 0) return 0;

    const char *p = data + 4;
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
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n\r\n%s",
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
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n\r\n%s",
        (int)strlen(body), body
    );

    send_http(id, http);
}

static void handle_request(uint8_t id, const char *data)
{
    if (is_ping_request(data))
    {
        send_http(id,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Cache-Control: no-store\r\n"
            "Connection: close\r\n"
            "Content-Length: 8\r\n\r\n"
            "{\"ok\":1}");
        return;
    }

    if (is_favicon_request(data))
    {
        send_http(id,
            "HTTP/1.1 204 No Content\r\n"
            "Cache-Control: max-age=86400\r\n"
            "Connection: close\r\n\r\n");
        return;
    }

    if (is_root_request(data))
    {
        int body_len = (int)strlen(ROOT_HTML);
        snprintf(root_http, sizeof(root_http),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Cache-Control: no-store\r\n"
            "Connection: close\r\n"
            "Content-Length: %d\r\n\r\n%s",
            body_len, ROOT_HTML);

        send_http(id, root_http);
        return;
    }

    {
        char path[160];
        if (!extract_get_path(data, path, sizeof(path)))
        {
            send_json_state(id, "", 0, "no GET path");
            return;
        }

        if (strncmp(path, "/telemetry", 10) == 0)
        {
            send_json_telemetry(id);
            return;
        }

        char cmd4[5];
        if (parse_cmd_from_path(path, cmd4))
        {
            ProcessCommand((uint8_t*)cmd4);
            send_json_state(id, cmd4, 1, "");
            return;
        }

        send_http(id,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Cache-Control: no-store\r\n"
            "Connection: close\r\n"
            "Content-Length: 3\r\n\r\n"
            "404");
    }
}

static void process_rx_buffer(void)
{
    for (;;)
    {
        char *s = strstr(rxbuf, "+IPD,");
        if (!s)
        {
            if (rxlen > 1024) rx_trim_to_tail(256);
            return;
        }

        if (s != rxbuf)
        {
            uint16_t drop = (uint16_t)(s - rxbuf);
            memmove(rxbuf, rxbuf + drop, rxlen - drop);
            rxlen = (uint16_t)(rxlen - drop);
            rxbuf[rxlen] = 0;
        }

        char *p = rxbuf + 5;

        int id = 0;
        while (*p >= '0' && *p <= '9') { id = id * 10 + (*p - '0'); p++; }
        if (*p != ',') return;
        p++;

        int len = 0;
        while (*p >= '0' && *p <= '9') { len = len * 10 + (*p - '0'); p++; }
        if (*p != ':') return;
        p++;

        uint16_t header_len = (uint16_t)(p - rxbuf);
        uint32_t need = (uint32_t)header_len + (uint32_t)len;

        if (need > RX_BUF_SIZE - 1)
        {
            memmove(rxbuf, rxbuf + header_len, rxlen - header_len);
            rxlen = (uint16_t)(rxlen - header_len);
            rxbuf[rxlen] = 0;
            return;
        }

        if (rxlen < need) return;

        uint16_t copy_len = (uint16_t)len;
        if (copy_len >= REQ_BUF_SIZE) copy_len = REQ_BUF_SIZE - 1;
        memcpy(reqbuf, p, copy_len);
        reqbuf[copy_len] = 0;

        handle_request((uint8_t)id, reqbuf);

        memmove(rxbuf, rxbuf + need, rxlen - (uint16_t)need);
        rxlen = (uint16_t)(rxlen - (uint16_t)need);
        rxbuf[rxlen] = 0;
    }
}
/* USER CODE END Private functions */

void WifiApi_Init(void)
{
    rxlen = 0;
    rxbuf[0] = 0;
    uart_rx_pause = 0;

    esp_init();
}

void WifiApi_Process(void)
{
    uint8_t c;

    if (!uart_rx_pause)
    {
        if (HAL_UART_Receive(&huart2, &c, 1, 10) == HAL_OK)
        {
            rx_push_byte(c);

            if (strstr(rxbuf, "+IPD"))
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
            }

            process_rx_buffer();
        }
        else
        {
            process_rx_buffer();
        }
    }
}
