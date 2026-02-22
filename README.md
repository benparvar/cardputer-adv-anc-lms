# cardputer-adv-anc-lms
Cardputer ADV ANC LMS

💻 Adapted ANC LMS code (no external hardware required)

Important Adjustments

LMS_TAPS: 16–64 (higher = better cancellation, more CPU)

MU:

Too high → distortion/unstable

Too low → slow adaptation

Recommended range: 0.00001 to 0.0001

Technical Note

This is a “single-mic adaptive noise reduction” ANC. Real ANC (like premium headphones) needs:

External mic (noise)

Internal mic (error)

Feedforward/feedback structure

Even so, this LMS already removes:

constant noise (fan)

hiss

continuous ambient noise