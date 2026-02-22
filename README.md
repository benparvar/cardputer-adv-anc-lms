# cardputer-adv-anc-lms
Cardputer ADV ANC LMS

💻 Adapted ANC LMS code (no external hardware required)

What changed for ultra-low latency

BLOCK = 128 → ~7.5 ms of audio per cycle

LMS_TAPS = 16 → less processing

delay(0) → avoids extra delays

Optimized LMS update (mu_e)

Typical latency: ~10–15 ms, limited by hardware (non-simultaneous mic/speaker).

Fine-tuning tips

Constant noise environment (fan): increase MU slightly (0.0001)

Distorted voice: reduce MU (0.00003)

Weak cancellation: increase LMS_TAPS to 24 (more CPU)