#include <M5Cardputer.h>

static constexpr size_t BLOCK = 128;
static constexpr size_t SR = 17000;
static constexpr size_t LMS_TAPS = 16;
static constexpr float MU = 0.00008f;

int16_t mic_buf[BLOCK];
int16_t out_buf[BLOCK];

float w[LMS_TAPS] = {0};
float x_ref[LMS_TAPS] = {0};

float computeRMS(int16_t* buf, size_t len) {
  float sum = 0.0f;
  for (size_t i = 0; i < len; i++) {
    float s = (float)buf[i];
    sum += s * s;
  }
  return sqrtf(sum / len);
}

void lmsProcess(int16_t* in, int16_t* out, size_t len) {
  for (size_t n = 0; n < len; n++) {
    float x = (float)in[n];

    for (int i = LMS_TAPS - 1; i > 0; i--) {
      x_ref[i] = x_ref[i - 1];
    }
    x_ref[0] = x;

    float y = 0.0f;
    for (int i = 0; i < LMS_TAPS; i++) {
      y += w[i] * x_ref[i];
    }

    float e = x - y;

    float mu_e = MU * e;
    for (int i = 0; i < LMS_TAPS; i++) {
      w[i] += mu_e * x_ref[i];
    }

    if (e > 32767) e = 32767;
    if (e < -32768) e = -32768;

    out[n] = (int16_t)e;
  }
}

void drawWaveforms(int16_t* in, int16_t* out, size_t len) {
  int h = M5Cardputer.Display.height();
  int w = M5Cardputer.Display.width();
  int centerY = h / 2;

  M5Cardputer.Display.fillRect(0, 0, w, 50, BLACK);

  for (int x = 0; x < w - 1 && x < (int)len - 1; x++) {
    int y1 = centerY + (in[x] >> 8);
    int y2 = centerY + (in[x + 1] >> 8);
    M5Cardputer.Display.drawLine(x, y1, x + 1, y2, WHITE);

    int y3 = centerY + (out[x] >> 8);
    int y4 = centerY + (out[x + 1] >> 8);
    M5Cardputer.Display.drawLine(x, y3, x + 1, y4, GREEN);
  }
}

void drawReductionMeter(float reduction) {
  reduction = constrain(reduction, 0.0f, 1.0f);

  int barWidth = M5Cardputer.Display.width() - 40;
  int filled = (int)(barWidth * reduction);

  M5Cardputer.Display.fillRect(20, 60, barWidth, 20, TFT_DARKGREY);
  M5Cardputer.Display.fillRect(20, 60, filled, 20, TFT_GREEN);

  M5Cardputer.Display.setCursor(20, 90);
  M5Cardputer.Display.printf("Noise Red: %d%%", (int)(reduction * 100));
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setFont(&fonts::FreeSansBoldOblique12pt7b);
  M5Cardputer.Display.drawString("ANC LMS 128", 30, 110);

  M5Cardputer.Speaker.end();
  M5Cardputer.Mic.begin();
}

void loop() {
  M5Cardputer.update();

  if (!M5Cardputer.Mic.isEnabled()) return;

  if (M5Cardputer.Mic.record(mic_buf, BLOCK, SR)) {

    float rms_in = computeRMS(mic_buf, BLOCK);

    lmsProcess(mic_buf, out_buf, BLOCK);

    float rms_out = computeRMS(out_buf, BLOCK);

    float reduction = 0.0f;
    if (rms_in > 1.0f) {
      reduction = 1.0f - (rms_out / rms_in);
    }

    drawWaveforms(mic_buf, out_buf, BLOCK);
    drawReductionMeter(reduction);

    while (M5Cardputer.Mic.isRecording()) delay(0);
    M5Cardputer.Mic.end();

    M5Cardputer.Speaker.begin();
    M5Cardputer.Speaker.setVolume(255);
    M5Cardputer.Speaker.playRaw(out_buf, BLOCK, SR, false, 1, 0);

    while (M5Cardputer.Speaker.isPlaying()) delay(0);
    M5Cardputer.Speaker.end();
    M5Cardputer.Mic.begin();
  }

  if (M5Cardputer.BtnA.wasClicked()) {
    memset(w, 0, sizeof(w));
    memset(x_ref, 0, sizeof(x_ref));
    M5Cardputer.Display.clear();
    M5Cardputer.Display.drawString("FILTER RESET", 20, 110);
  }
}