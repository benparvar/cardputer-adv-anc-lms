# cardputer-adv-anc-lms
Cardputer ADV ANC LMS

💻 Adapted ANC LMS code (no external hardware required)

This code uses the internal microphone and speaker.

------

🔧 Important Adjustments

1️⃣ Verify correct pins

If there is no sound, confirm in your Cardputer schematic:

data_in_num = internal microphone pin

data_out_num = internal speaker pin

2️⃣ Latency (critical for ANC)
#define BUFFER_SIZE 32 // lower = better cancellation
3️⃣ Gain and stability

If you hear:

hissing → decrease MU

weak cancellation → increase MU slightly

🎯 Important limitation (internal microphone)

Since the microphone is far from the ear:

Cancellation will be moderate

Works best for constant noises (fan, motor)

For "professional headphone level" ANC, the microphone needs to be close to the ear (ANC feedback).

Next recommended step

Would you like me to adapt it for:

🎧 Use with headphones (P2 or I2S external DAC)

or

🎚️ Auto-calibration that automatically adjusts MU and latency?