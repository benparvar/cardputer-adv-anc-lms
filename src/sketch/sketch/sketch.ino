#include <driver/i2s.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#define I2S_PORT I2S_NUM_0

#define SAMPLE_RATE 16000
#define BUFFER_SIZE 64
#define FILTER_TAPS 32
#define MU 0.00002f

static QueueHandle_t audioQueue;

float filter[FILTER_TAPS] = {0};
float x[FILTER_TAPS] = {0};

void setupI2S() {
  i2s_config_t config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .dma_buf_count = 6,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1
  };

  // Pinos internos típicos do Cardputer
  i2s_pin_config_t pins = {
    .bck_io_num = 41,
    .ws_io_num = 42,
    .data_out_num = 43,  // speaker
    .data_in_num = 46    // mic interno
  };

  i2s_driver_install(I2S_PORT, &config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
}

// 🎤 Captura
void captureTask(void *param) {
  int16_t buffer[BUFFER_SIZE];
  size_t bytesRead;

  while (true) {
    i2s_read(I2S_PORT, buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);
    xQueueSend(audioQueue, buffer, portMAX_DELAY);
  }
}

// 🧠 ANC LMS
void ancTask(void *param) {
  int16_t buffer[BUFFER_SIZE];
  size_t bytesWritten;

  while (true) {
    if (xQueueReceive(audioQueue, buffer, portMAX_DELAY) == pdTRUE) {

      for (int n = 0; n < BUFFER_SIZE; n++) {
        float input = (float)buffer[n];

        // histórico
        for (int i = FILTER_TAPS - 1; i > 0; i--) {
          x[i] = x[i - 1];
        }
        x[0] = input;

        float y = 0;
        for (int i = 0; i < FILTER_TAPS; i++) {
          y += filter[i] * x[i];
        }

        float error = input + y;

        for (int i = 0; i < FILTER_TAPS; i++) {
          filter[i] -= MU * error * x[i];
        }

        buffer[n] = (int16_t)(-y);
      }

      i2s_write(I2S_PORT, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);
    }
  }
}

void setup() {
  setCpuFrequencyMhz(240);

  audioQueue = xQueueCreate(4, sizeof(int16_t) * BUFFER_SIZE);

  setupI2S();

  xTaskCreatePinnedToCore(captureTask, "Capture", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(ancTask, "ANC", 8192, NULL, 3, NULL, 1);
}

void loop() {}