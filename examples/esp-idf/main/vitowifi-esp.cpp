#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include <VitoWiFi.h>
#include <soc/uart_reg.h>

/*
This example is to show how you could build your own
interface and serves as a compilation test
*/

#define VITOWIFI_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define VITOWIFI_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define VITOWIFI_TEST_RTS (UART_PIN_NO_CHANGE)
#define VITOWIFI_TEST_CTS (UART_PIN_NO_CHANGE)

#define VITOWIFI_UART_PORT_NUM  (uart_port_t)(CONFIG_EXAMPLE_UART_PORT_NUM)
#define BUF_SIZE (1024)
// Dummy class that has all the methods for VitoWiFi to work
// but doesn't do anything in this case.
// Use as skeleton for your own implementation
class DummyInterface : VitoWiFiInternals::SerialInterface {
 public:
  bool begin() {
    // prepare the interface
    // optolink comm at 4800 baud, 8 bits, even parity and 2 stop bits
    // called at VitoWiFi::begin()
    /* Configure parameters of an UART driver,
 * communication pins and install the driver */
    uart_config_t uart_config = {
      .baud_rate = 4800,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_EVEN,
      .stop_bits = UART_STOP_BITS_2,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
      .source_clk = UART_SCLK_DEFAULT,
      .flags = {},
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(VITOWIFI_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, nullptr, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(VITOWIFI_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(VITOWIFI_UART_PORT_NUM, VITOWIFI_TEST_TXD, VITOWIFI_TEST_RXD, VITOWIFI_TEST_RTS, VITOWIFI_TEST_CTS));
    return true;
  }

  void end() {
    // stop the interface
    // called at VitoWiFi::end()
  }
  std::size_t write(const uint8_t* data, uint8_t length) {
    // tries to write `data` with length `length` to the interface
    // returns the actually written data
    return uart_write_bytes(VITOWIFI_UART_PORT_NUM, data, length);
  }
  uint8_t read() {
    uint8_t c;
    uart_read_bytes(VITOWIFI_UART_PORT_NUM, &c, 1, portMAX_DELAY);
    // read one byte from the interface
    // availability of data is checked first
    return 0;
  }
  size_t available() {
    size_t available = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(VITOWIFI_UART_PORT_NUM, &available));
    // check if data is available
    return available;
  }
};

// optolink on Dummy Interface, logging output on UART1 (connected to USB)
DummyInterface dummyInterface;
#define SERIAL1 dummyInterface
TaskHandle_t loopTaskHandle = NULL;
static const char *TAG = "VitoWiFi";

VitoWiFi::VitoWiFi<VitoWiFi::VS2> vitoWiFi(&SERIAL1);
bool readValues = false;
uint8_t datapointIndex = 0;

VitoWiFi::Datapoint datapoints[] = {
  VitoWiFi::Datapoint("outsidetemp", 0x5525, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("boilertemp", 0x0810, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("pump", 0x2906, 1, VitoWiFi::noconv)
};

void onResponse(const VitoWiFi::PacketVS2& response, const VitoWiFi::Datapoint& request) {
  // raw data can be accessed through the 'response' argument
  ESP_LOGI(TAG, "Raw data received:");
  ESP_LOG_BUFFER_HEX(TAG, response.data(), response.dataLength());

  // the raw data can be decoded using the datapoint. Be sure to use the correct type
  if (request.converter() == VitoWiFi::div10) {
    float value = request.decode(response);
    ESP_LOGI(TAG, "%s: %.1f", request.name(), value);
  } else if (request.converter() == VitoWiFi::noconv) {
    bool value = request.decode(response);
    // alternatively, we can just cast response.data()[0] to bool
    ESP_LOGI(TAG, "%s: %s", request.name(), value ? "ON" : "OFF");
  }
}

void onError(VitoWiFi::OptolinkResult error, const VitoWiFi::Datapoint& request) {
  ESP_LOGI(TAG, "Datapoint \"%s\" error: ", request.name());
  if (error == VitoWiFi::OptolinkResult::TIMEOUT) {
    ESP_LOGE(TAG, "timeout");
  } else if (error == VitoWiFi::OptolinkResult::LENGTH) {
    ESP_LOGE(TAG, "length");
  } else if (error == VitoWiFi::OptolinkResult::NACK) {
    ESP_LOGE(TAG, "nack");
  } else if (error == VitoWiFi::OptolinkResult::CRC) {
    ESP_LOGE(TAG, "crc");
  } else if (error == VitoWiFi::OptolinkResult::ERROR) {
    ESP_LOGE(TAG, "error");
  }
}

uint64_t millis() {
  return std::chrono::duration_cast<std::chrono::duration<uint32_t, std::milli>>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void yieldIfNecessary(){
  static uint64_t lastYield = 0;
  uint64_t now = millis();
  if((now - lastYield) > 2000) {
    lastYield = now;
    vTaskDelay(5); //delay 1 RTOS tick
  }
}


void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 60000UL) {  // read all values every 60 seconds
    lastMillis = millis();
    readValues = true;
    datapointIndex = 0;
  }

  if (readValues) {
    if (vitoWiFi.read(datapoints[datapointIndex])) {
      ++datapointIndex;
    }
    if (datapointIndex == 3) {
      readValues = false;
    }
  }

  vitoWiFi.loop();
}

extern "C"
{
  [[noreturn]] void loopTask(void *pvParameters) {
    for(;;) {
      yieldIfNecessary();
      loop();
    }
  }

  void app_main() {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Setting up vitoWiFi");

    vitoWiFi.onResponse(onResponse);
    vitoWiFi.onError(onError);
    vitoWiFi.begin();

    ESP_LOGI(TAG, "Setup finished");

    xTaskCreate(loopTask, "loopTask", 8192, nullptr, 1, &loopTaskHandle);
  }
}

