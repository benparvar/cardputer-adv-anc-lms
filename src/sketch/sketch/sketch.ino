#include <M5Cardputer.h>

static constexpr size_t BLOCK = 128;     // menor buffer = menor latência
static constexpr size_t SR = 17000;
static constexpr size_t LMS_TAPS = 16;   // menos taps = mais rápido
static constexpr float MU = 0.00008f;    // taxa maior para adaptação rápida

int16_t mic_buf[BLOCK];
int16_t out_buf[BLOCK];

// Filtro LMS
float w[LMS_TAPS] = {0};
float x_ref[LMS_TAPS] = {0};

void lmsProcess(int16_t* in, int16_t* out, size_t len) {
  for (size_t n = 0; n < len; n++) {
    float x = (float)in[n];

    // Atualiza buffer de referência (shift)
    for (int i = LMS_TAPS - 1; i > 0; i--) {
      x_ref[i] = x_ref[i - 1];
    }
    x_ref[0] = x;

    // Estimativa do ruído
    float y = 0.0f;
    for (int i = 0; i < LMS_TAPS; i++) {
      y += w[i] * x_ref[i];
    }

    // Erro = sinal limpo estimado
    float e = x - y;

    // Atualização LMS otimizada
    float mu_e = MU * e;
    for (int i = 0; i < LMS_TAPS; i++) {
      w[i] += mu_e * x_ref[i];
    }

    // Limite de segurança
    if (e > 32767) e = 32767;
    if (e < -32768) e = -32768;

    out[n] = (int16_t)e;
  }
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setFont(&fonts::FreeSansBoldOblique12pt7b);
  M5Cardputer.Display.drawString("ANC LMS 128", 30, 20);

  M5Cardputer.Speaker.end();
  M5Cardputer.Mic.begin();
}

void loop() {
  M5Cardputer.update();

  if (!M5Cardputer.Mic.isEnabled()) return;

  if (M5Cardputer.Mic.record(mic_buf, BLOCK, SR)) {

    // Processa LMS
    lmsProcess(mic_buf, out_buf, BLOCK);

    // Alternância rápida (quase tempo real)
    while (M5Cardputer.Mic.isRecording()) delay(0);
    M5Cardputer.Mic.end();

    M5Cardputer.Speaker.begin();
    M5Cardputer.Speaker.setVolume(255);
    M5Cardputer.Speaker.playRaw(out_buf, BLOCK, SR, false, 1, 0);

    while (M5Cardputer.Speaker.isPlaying()) delay(0);
    M5Cardputer.Speaker.end();
    M5Cardputer.Mic.begin();
  }

  // Botão A: reset do filtro adaptativo
  if (M5Cardputer.BtnA.wasClicked()) {
    memset(w, 0, sizeof(w));
    memset(x_ref, 0, sizeof(x_ref));
    M5Cardputer.Display.clear();
    M5Cardputer.Display.drawString("FILTER RESET", 20, 20);
  }
}