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

#define CMDQ_SIZE     16
#define ESP_TX_CHUNK  1024

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

static char reqbuf[REQ_BUF_SIZE];

static volatile uint8_t cmdq_head = 0;
static volatile uint8_t cmdq_tail = 0;
static char cmdq[CMDQ_SIZE][5];

/* UI: jedna kolejka, cooldown po komendach, telemetria co 500ms */
static const char ROOT_HTML[] =
"<!doctype html><html lang=pl><head><meta charset=utf-8><meta name=viewport content='width=device-width,initial-scale=1'>"
"<title>RC Telemetry</title><style>"
":root{--bg:#0b0f14;--t:#e7eefc;--m:#a8b3c7;--b:rgba(255,255,255,.08);--ok:#33d69f;--bad:#ff6b6b}"
"*{box-sizing:border-box}body{margin:0;font-family:system-ui,Segoe UI,Roboto,Arial;background:var(--bg);color:var(--t)}"
"#top{padding:10px 12px;border-bottom:1px solid var(--b);display:flex;gap:12px;align-items:center;flex-wrap:wrap}"
".dot{width:10px;height:10px;border-radius:50%;background:rgba(255,255,255,.18);border:1px solid var(--b)}"
".dot.ok{background:var(--ok);border-color:rgba(51,214,159,.45)}.dot.bad{background:var(--bad);border-color:rgba(255,107,107,.45)}"
".k{color:var(--m);font-size:12px}"
".wrap{padding:12px;display:flex;gap:14px;flex-wrap:wrap;align-items:flex-start}"
".card{background:rgba(17,24,36,.75);border:1px solid var(--b);border-radius:14px;padding:12px;backdrop-filter:blur(8px);display:grid;gap:10px}"
".btn{padding:10px 12px;border-radius:12px;border:1px solid var(--b);background:rgba(255,255,255,.06);color:var(--t);cursor:pointer;font-weight:700}"
".btn:active{transform:translateY(1px)}.btn:disabled{opacity:.55;cursor:not-allowed}"
".btn.active{background:rgba(51,214,159,.16);border-color:rgba(51,214,159,.30)}"
".dpad{display:grid;grid-template-columns:56px 56px 56px;grid-template-rows:56px 56px 56px;gap:8px}"
".dpad .btn{width:56px;height:56px;padding:0;display:grid;place-items:center;font-size:18px;user-select:none;touch-action:none}"
".charts{display:grid;gap:10px;min-width:320px;flex:1 1 520px}"
".row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}"
"canvas{width:100%;height:240px;display:block;border:1px solid var(--b);border-radius:14px;background:rgba(255,255,255,.03)}"
"</style></head><body>"
"<div id=top><b>RC Telemetry</b><span class=k>A:<span id=a>0</span></span><span class=k>B:<span id=b>0</span></span>"
"<span class=k>AVG:<span id=avg>0</span></span><span class=dot id=dot></span><span class=k id=st>Start…</span></div>"
"<div class=wrap>"
"<div class=card><div class=row>"
"<button class=btn id=engine>Silnik <span class=k id=se>OFF</span></button>"
"<button class=btn id=horn>Klakson <span class=k id=sh>OFF</span></button>"
"<button class=btn id=lights>Światła <span class=k id=sl>OFF</span></button>"
"</div><div class=dpad>"
"<button class=btn id=ul>↖</button><button class=btn id=up>▲</button><button class=btn id=ur>↗</button>"
"<button class=btn id=le>◀</button><button class=btn id=stp disabled>■</button><button class=btn id=ri>▶</button>"
"<button class=btn id=dl>↙</button><button class=btn id=dn>▼</button><button class=btn id=dr>↘</button>"
"</div><div class=k>Wciśnij i trzymaj kierunek — puszczenie wysyła STOP (/motor/1)</div></div>"
"<div class=charts>"
"<div class=card><b>Silnik A</b><canvas id=cA width=600 height=240></canvas></div>"
"<div class=card><b>Silnik B</b><canvas id=cB width=600 height=240></canvas></div>"
"<div class=card><b>Średnia</b><canvas id=cM width=600 height=240></canvas></div>"
"</div></div>"
"<script>"
"const $=id=>document.getElementById(id);"
"function sleep(ms){return new Promise(r=>setTimeout(r,ms));}"
"function setDot(k,txt){const d=$('dot');d.classList.remove('ok','bad');if(k)d.classList.add(k);$('st').textContent=txt||'';}"
"function setTog(j){if(!j||typeof j!=='object')return;"
"if('engine'in j){const on=+j.engine===1;$('engine').classList.toggle('active',on);$('se').textContent=on?'ON':'OFF';}"
"if('horn'in j){const on=+j.horn===1;$('horn').classList.toggle('active',on);$('sh').textContent=on?'ON':'OFF';}"
"if('ind'in j){const on=+j.ind===1;$('lights').classList.toggle('active',on);$('sl').textContent=on?'ON':'OFF';}"
"}"
"const N=180;let SA=[],SB=[],SM=[];"
"function push(a,b){a=+a||0;b=+b||0;let m=(a+b)/2;SA.push(a);SB.push(b);SM.push(m);"
"if(SA.length>N){SA.shift();SB.shift();SM.shift();}"
"$('a').textContent=a;$('b').textContent=b;$('avg').textContent=Math.round(m);}"
"function draw(c,arr){const x=c.getContext('2d');const w=c.width,h=c.height,p=10,xx=w-2*p,yy=h-2*p;"
"x.clearRect(0,0,w,h);const m=arr.length;if(m<2)return;"
"let lo=1e9,hi=-1e9;for(let i=0;i<m;i++){const v=arr[i];if(v<lo)lo=v;if(v>hi)hi=v;}if(lo===hi){lo-=1;hi+=1;}"
"const r=hi-lo;x.beginPath();for(let i=0;i<m;i++){const X=p+i/(N-1)*xx;const Y=p+(hi-arr[i])/r*yy;"
"if(i)x.lineTo(X,Y);else x.moveTo(X,Y);}x.stroke();}"
"function redraw(){draw($('cA'),SA);draw($('cB'),SB);draw($('cM'),SM);requestAnimationFrame(redraw);}redraw();"

"let queue=Promise.resolve();"
"let cooldownUntil=0;"
"const REQ_TIMEOUT_MS=6000;"
"const REQ_GAP_MS=150;"
"const AFTER_CMD_COOLDOWN_MS=800;"
"const TELEM_PERIOD_MS=500;"
"function isTelemetry(p){return p.startsWith('/telemetry');}"
"async function maybeCooldown(){const now=Date.now();if(now<cooldownUntil) await sleep(cooldownUntil-now);}"

"function cmd(p){"
"  const tel=isTelemetry(p);"
"  queue = queue.then(async ()=>{"
"    await maybeCooldown();"
"    const ac=new AbortController();"
"    const t=setTimeout(()=>ac.abort(), REQ_TIMEOUT_MS);"
"    try{"
"      const r=await fetch(p,{cache:'no-store',signal:ac.signal});"
"      const j=await r.json();"
"      if(!tel){"
"        const until=Date.now()+AFTER_CMD_COOLDOWN_MS;"
"        if(until>cooldownUntil) cooldownUntil=until;"
"      }"
"      await sleep(REQ_GAP_MS);"
"      return j;"
"    }catch(e){"
"      if(!tel){"
"        const until=Date.now()+AFTER_CMD_COOLDOWN_MS;"
"        if(until>cooldownUntil) cooldownUntil=until;"
"      }"
"      await sleep(REQ_GAP_MS);"
"      return null;"
"    }finally{"
"      clearTimeout(t);"
"    }"
"  });"
"  return queue;"
"}"

"function bindHold(id,code){const b=$(id);let pid=null;"
"b.addEventListener('pointerdown',e=>{if(pid!==null)return;pid=e.pointerId||1;try{b.setPointerCapture(pid);}catch{} cmd('/motor/'+code);});"
"const up=()=>{if(pid===null)return;pid=null;cmd('/motor/1');};"
"b.addEventListener('pointerup',up);b.addEventListener('pointercancel',up);}"
"bindHold('up',2);bindHold('ur',3);bindHold('ri',4);bindHold('dr',5);bindHold('dn',6);bindHold('dl',7);bindHold('le',8);bindHold('ul',9);"
"$('engine').onclick=()=>cmd('/engine').then(setTog);"
"$('horn').onclick=()=>cmd('/horn').then(setTog);"
"$('lights').onclick=()=>cmd('/lights').then(setTog);"

"(async()=>{"
"  setDot('', 'Init…');"
"  const init=await cmd('/init');"
"  if(init){setTog(init);setDot('ok','OK');}else{setDot('bad','Init error');}"
"  while(true){"
"    const j=await cmd('/telemetry?t='+Date.now());"
"    if(j&&+j.ok===1){push(j.a_meas??0,j.b_meas??0);setTog(j);setDot('ok','Online');}"
"    else{setDot('bad','No data');}"
"    await sleep(TELEM_PERIOD_MS);"
"  }"
"})();"
"</script></body></html>";
/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */
static void esp_send(const char *cmd);
static void esp_init(void);

static void rx_push_byte(uint8_t c);
static void rx_trim_to_tail(uint16_t keep);
static void process_rx_buffer(void);

static void timebase_init_fast(void);
static uint32_t now_us(void);
static int uart2_try_read(uint8_t *out);

static int wait_for_string(const char *str, uint32_t timeout_ms);

static int is_ping_request(const char *data);
static int is_root_request(const char *data);
static int is_favicon_request(const char *data);

static int extract_get_path(const char *data, char *out, uint16_t out_size);
static int parse_cmd_from_path(const char *path, char out_cmd4[5]);
static int parse_motor_mode_from_path(const char *path);

static void send_json_state(uint8_t id, const char *cmd4, int ok, const char *msg);
static void send_json_telemetry(uint8_t id);

static uint8_t cmdq_next(uint8_t v);
static int cmd_is_motor(const char cmd4[5]);
static int cmdq_push(const char cmd4[5]);
static int cmdq_pop(char out_cmd4[5]);

static int esp_send_raw(uint8_t id, const char *data, uint16_t len);
static void send_http_len(uint8_t id, const char *data, uint32_t len);
static void send_http(uint8_t id, const char *payload);
static void send_root_page(uint8_t id);
/* USER CODE END Private function prototypes */

/* USER CODE BEGIN Private functions */
static uint32_t dwt_div = 1;
static uint8_t dwt_ok = 0;

static void timebase_init_fast(void)
{
#if defined(DWT) && defined(CoreDebug) && defined(DWT_CTRL_CYCCNTENA_Msk) && defined(CoreDebug_DEMCR_TRCENA_Msk)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    dwt_div = SystemCoreClock / 1000000U;
    if (dwt_div == 0) dwt_div = 1;
    dwt_ok = 1;
#else
    dwt_ok = 0;
    dwt_div = 1;
#endif
}

static uint32_t now_us(void)
{
#if defined(DWT)
    if (dwt_ok) return (uint32_t)(DWT->CYCCNT / dwt_div);
#endif
    return HAL_GetTick() * 1000U;
}

static int uart2_try_read(uint8_t *out)
{
    UART_HandleTypeDef *hu = &huart2;

#ifdef UART_FLAG_ORE
    if (__HAL_UART_GET_FLAG(hu, UART_FLAG_ORE)) { __HAL_UART_CLEAR_OREFLAG(hu); }
#endif
#ifdef UART_FLAG_FE
    if (__HAL_UART_GET_FLAG(hu, UART_FLAG_FE))  { __HAL_UART_CLEAR_FEFLAG(hu); }
#endif
#ifdef UART_FLAG_NE
    if (__HAL_UART_GET_FLAG(hu, UART_FLAG_NE))  { __HAL_UART_CLEAR_NEFLAG(hu); }
#endif
#ifdef UART_FLAG_PE
    if (__HAL_UART_GET_FLAG(hu, UART_FLAG_PE))  { __HAL_UART_CLEAR_PEFLAG(hu); }
#endif

    if (!__HAL_UART_GET_FLAG(hu, UART_FLAG_RXNE))
        return 0;

#if defined(USART_RDR_RDR_Msk) || defined(USART_RDR_RDR)
    *out = (uint8_t)(hu->Instance->RDR & 0xFF);
#else
    *out = (uint8_t)(hu->Instance->DR & 0xFF);
#endif
    return 1;
}

static int wait_for_string(const char *str, uint32_t timeout_ms)
{
    uint8_t c;
    uint32_t idx = 0;
    uint32_t t0 = now_us();
    uint32_t timeout_us = timeout_ms * 1000U;

    while ((uint32_t)(now_us() - t0) < timeout_us)
    {
        if (uart2_try_read(&c))
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
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, (uint16_t)strlen(cmd), HAL_MAX_DELAY);
    HAL_Delay(200);
}

static void esp_init(void)
{
    char cmd[128];

    esp_send("AT+RST\r\n");
    HAL_Delay(1500);

    esp_send("ATE0\r\n");
    esp_send("AT+CWMODE=1\r\n");

    snprintf(cmd, sizeof(cmd), "AT+CIPSTA=\"%s\",\"%s\",\"%s\"\r\n", WIFI_STATIC_IP, WIFI_GATEWAY, WIFI_NETMASK);
    esp_send(cmd);

    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASS);
    esp_send(cmd);
    HAL_Delay(5000);

    esp_send("AT+CIPMUX=1\r\n");
    esp_send("AT+CIPSERVER=1,80\r\n");
}

static void rx_push_byte(uint8_t c)
{
    if (rxlen >= RX_BUF_SIZE - 1)
        rx_trim_to_tail(512);

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

static int is_ping_request(const char *data) { return (strncmp(data, "GET /ping", 9) == 0); }
static int is_root_request(const char *data) { return (strncmp(data, "GET / ", 6) == 0); }
static int is_favicon_request(const char *data) { return (strncmp(data, "GET /favicon.ico", 16) == 0); }

static uint8_t cmdq_next(uint8_t v)
{
    v++;
    if (v >= CMDQ_SIZE) v = 0;
    return v;
}

static int cmd_is_motor(const char cmd4[5])
{
    return (cmd4[0]=='C' && cmd4[1]=='M' && cmd4[2]=='D' && cmd4[3]>='1' && cmd4[3]<='9');
}

static int cmdq_push(const char cmd4[5])
{
    uint8_t head = cmdq_head;
    uint8_t tail = cmdq_tail;

    if (cmd_is_motor(cmd4) && head != tail)
    {
        uint8_t last = (head == 0) ? (CMDQ_SIZE - 1) : (head - 1);
        if (cmd_is_motor(cmdq[last]))
        {
            memcpy(cmdq[last], cmd4, 5);
            return 1;
        }
    }

    uint8_t next = cmdq_next(head);
    if (next == tail)
        return 0;

    memcpy(cmdq[head], cmd4, 5);
    cmdq_head = next;
    return 1;
}

static int cmdq_pop(char out_cmd4[5])
{
    uint8_t head = cmdq_head;
    uint8_t tail = cmdq_tail;

    if (head == tail)
        return 0;

    memcpy(out_cmd4, cmdq[tail], 5);
    cmdq_tail = cmdq_next(tail);
    return 1;
}

static int esp_send_raw(uint8_t id, const char *data, uint16_t len)
{
    char cmd[64];

    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d,%u\r\n", id, (unsigned)len);
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, (uint16_t)strlen(cmd), HAL_MAX_DELAY);

    if (!wait_for_string(">", 2500))
        return 0;

    HAL_UART_Transmit(&huart2, (uint8_t*)data, len, HAL_MAX_DELAY);

    if (!wait_for_string("SEND OK", 6000))
        return 0;

    return 1;
}

static void send_http_len(uint8_t id, const char *data, uint32_t len)
{
    char cmd[32];

    uart_rx_pause = 1;

    uint32_t off = 0;
    while (off < len)
    {
        uint16_t chunk = (uint16_t)((len - off) > ESP_TX_CHUNK ? ESP_TX_CHUNK : (len - off));
        if (!esp_send_raw(id, data + off, chunk))
            break;
        off += chunk;
    }

    snprintf(cmd, sizeof(cmd), "AT+CIPCLOSE=%d\r\n", id);
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, (uint16_t)strlen(cmd), HAL_MAX_DELAY);

    uart_rx_pause = 0;
}

static void send_http(uint8_t id, const char *payload)
{
    send_http_len(id, payload, (uint32_t)strlen(payload));
}

static void send_root_page(uint8_t id)
{
    char hdr[256];
    char cmd[32];

    int body_len = (int)strlen(ROOT_HTML);
    int n = snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n"
        "Content-Length: %d\r\n\r\n",
        body_len
    );
    if (n < 0) return;
    if (n >= (int)sizeof(hdr)) n = (int)sizeof(hdr) - 1;

    uart_rx_pause = 1;

    if (esp_send_raw(id, hdr, (uint16_t)n))
    {
        uint32_t off = 0;
        while (off < (uint32_t)body_len)
        {
            uint16_t chunk = (uint16_t)(((uint32_t)body_len - off) > ESP_TX_CHUNK ? ESP_TX_CHUNK : ((uint32_t)body_len - off));
            if (!esp_send_raw(id, ROOT_HTML + off, chunk))
                break;
            off += chunk;
        }
    }

    snprintf(cmd, sizeof(cmd), "AT+CIPCLOSE=%d\r\n", id);
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, (uint16_t)strlen(cmd), HAL_MAX_DELAY);

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
    char body[340];
    char http[700];

    if (!msg) msg = "";

    snprintf(body, sizeof(body),
        "{\"ok\":%d,\"cmd\":\"%s\",\"engine\":%d,\"horn\":%d,\"ind\":%d,\"dist\":%lu,"
        "\"a_set\":%d,\"b_set\":%d,\"a_meas\":%d,\"b_meas\":%d,\"msg\":\"%s\"}",
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
    char body[280];
    char http[560];

    snprintf(body, sizeof(body),
        "{\"ok\":1,\"engine\":%d,\"horn\":%d,\"ind\":%d,\"dist\":%lu,"
        "\"a_set\":%d,\"b_set\":%d,\"a_meas\":%d,\"b_meas\":%d}",
        engineOn,
        hornOn,
        indicationOverride,
        (unsigned long)Distance,
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
        send_root_page(id);
        return;
    }

    {
        char path[200];
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
            int pushed = cmdq_push(cmd4);
            send_json_state(id, cmd4, pushed ? 1 : 0, pushed ? "queued" : "busy");
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

    cmdq_head = 0;
    cmdq_tail = 0;

    timebase_init_fast();
    esp_init();
}

void WifiApi_Process(void)
{
    if (uart_rx_pause)
        return;

    uint8_t c;
    int got = 0;

    while (uart2_try_read(&c))
    {
        got = 1;
        rx_push_byte(c);
        process_rx_buffer();
    }

    if (!got)
        process_rx_buffer();
}

void WifiApi_PollCommands(void)
{
    char cmd4[5];
    if (!cmdq_pop(cmd4))
        return;

    ProcessCommand((uint8_t*)cmd4);
}
