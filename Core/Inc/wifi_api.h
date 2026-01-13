#ifndef INC_WIFI_API_H_
#define INC_WIFI_API_H_

#include "main.h"
#include "usart.h"
#include "gpio.h"

void WifiApi_Init(void);
void WifiApi_Process(void);
void WifiApi_PollCommands(void);

static int esp_send_raw(uint8_t id, const char *data, uint16_t len);
static void send_http_len(uint8_t id, const char *data, uint32_t len);
static void send_root_page(uint8_t id);

#endif /* INC_WIFI_API_H_ */
