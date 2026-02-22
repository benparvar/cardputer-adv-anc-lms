#include <M5Cardputer.h>

static constexpr size_t block_length = 256;
static constexpr size_t samplerate = 17000;

int16_t rec_buffer[block_length];
int16_t play_buffer[block_length];

// Estimador adaptativo de ruído
float noise_estimate = 0.0f;
static constexpr float noise_alpha = 0.98f;   // suavização do ruído
static constexpr float suppress_gain = 1.2f;  // intensidade da supressão

void noiseCancelProcess(int16_t* in, int16_t* out, size_t len) {
  for (size_t i = 0; i < len; i++) {
    float sample = (float)in[i];

    // Estima ruído (média exponencial do módulo)
    float abs_sample = fabs(sample);
    noise_estimate = noise_alpha * noise_estimate + (1.0f - noise_alpha) * abs_sample;

    // Subtração adaptativa de ruído
    float cleaned = sample;
    if (abs_sample < noise_estimate * 1.5f) {
      cleaned = sample - (noise_estimate * suppress_gain) * (sample > 0 ? 1 : -1);
    }

    // Clipping de segurança
    if (cleaned > 32767) cleaned = 32767;
    if (cleaned < -32768) cleaned = -32768;

    out[i] = (int16_t)cleaned;
  }
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setFont(&fonts::FreeSansBoldOblique12pt7b);
  M5Cardputer.Display.drawString("ANC REALTIME", 20, 20);

  // Começa com microfone ativo
  M5Cardputer.Speaker.end();
  M5Cardputer.Mic.begin();
}

void loop() {
  M5Cardputer.update();

  if (!M5Cardputer.Mic.isEnabled()) return;

  // Grava um pequeno bloco de áudio
  if (M5Cardputer.Mic.record(rec_buffer, block_length, samplerate)) {

    // Processa cancelamento de ruído
    noiseCancelProcess(rec_buffer, play_buffer, block_length);

    // Alterna rapidamente: mic OFF -> speaker ON
    while (M5Cardputer.Mic.isRecording()) delay(1);
    M5Cardputer.Mic.end();
    M5Cardputer.Speaker.begin();
    M5Cardputer.Speaker.setVolume(255);

    // Reproduz áudio filtrado
    M5Cardputer.Speaker.playRaw(play_buffer, block_length, samplerate, false, 1, 0);
    while (M5Cardputer.Speaker.isPlaying()) delay(1);

    // Retorna para gravação
    M5Cardputer.Speaker.end();
    M5Cardputer.Mic.begin();
  }

  // Botão A aumenta a agressividade do cancelamento
  if (M5Cardputer.BtnA.wasClicked()) {
    noise_estimate *= 1.2f;
    M5Cardputer.Display.clear();
    M5Cardputer.Display.drawString("ANC BOOST", 20, 20);
  }
}