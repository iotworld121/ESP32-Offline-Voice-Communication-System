#include <Arduino.h>
#include <driver/i2s.h>

#define SAMPLE_RATE 16000

#define MIC_I2S_PORT I2S_NUM_0
#define I2S_SCK 14
#define I2S_WS 15
#define I2S_SD 32

#define SPK_I2S_PORT I2S_NUM_1
#define I2S_BCLK 27
#define I2S_LRC 26
#define I2S_DIN 25

#define BUFFER_SIZE 128
#define VOLUME 0.65

void setupI2S() {

  i2s_config_t mic_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t mic_pins;

  mic_pins.bck_io_num = I2S_SCK;
  mic_pins.ws_io_num = I2S_WS;
  mic_pins.data_out_num = I2S_PIN_NO_CHANGE;
  mic_pins.data_in_num = I2S_SD;
  mic_pins.mck_io_num = I2S_PIN_NO_CHANGE;

  i2s_driver_install(
    MIC_I2S_PORT,
    &mic_config,
    0,
    NULL
  );

  i2s_set_pin(
    MIC_I2S_PORT,
    &mic_pins
  );

  i2s_zero_dma_buffer(MIC_I2S_PORT);

  i2s_config_t spk_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t spk_pins;

  spk_pins.bck_io_num = I2S_BCLK;
  spk_pins.ws_io_num = I2S_LRC;
  spk_pins.data_out_num = I2S_DIN;
  spk_pins.data_in_num = I2S_PIN_NO_CHANGE;
  spk_pins.mck_io_num = I2S_PIN_NO_CHANGE;

  i2s_driver_install(
    SPK_I2S_PORT,
    &spk_config,
    0,
    NULL
  );

  i2s_set_pin(
    SPK_I2S_PORT,
    &spk_pins
  );

  i2s_zero_dma_buffer(SPK_I2S_PORT);
}

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("INMP441 -> ESP32 -> MAX98357A");
  Serial.println("==============================");

  setupI2S();

  Serial.println("READY");
  Serial.println("Speak into microphone...");
}

void loop() {

  int32_t raw_buffer[BUFFER_SIZE];
  int16_t out_buffer[BUFFER_SIZE * 2];

  size_t bytesRead = 0;

  i2s_read(
    MIC_I2S_PORT,
    raw_buffer,
    sizeof(raw_buffer),
    &bytesRead,
    portMAX_DELAY
  );

  if (bytesRead == 0)
    return;

  int samplesRead =
    bytesRead / sizeof(int32_t);

  for (int i = 0; i < samplesRead; i++) {

    int32_t sample =
      raw_buffer[i] >> 14;

    sample =
      (int32_t)(sample * VOLUME);

    if (sample > 30000)
      sample = 30000;

    if (sample < -30000)
      sample = -30000;

    int16_t audio =
      (int16_t)sample;

    out_buffer[i * 2] =
      audio;

    out_buffer[i * 2 + 1] =
      audio;
  }

  size_t bytesWritten = 0;

  i2s_write(
    SPK_I2S_PORT,
    out_buffer,
    samplesRead * 2 * sizeof(int16_t),
    &bytesWritten,
    portMAX_DELAY
  );
}
