# ESP32-S3 Edge Keyword Spotter - VS Code + ESP-IDF skeleton

What this is
- Minimal ESP32-S3 project skeleton for a keyword-spotter demo:
  - I2S audio capture component (uses ESP-IDF i2s driver)
  - Inference stub (energy-based) — replace with TensorFlow Lite Micro later
  - FreeRTOS tasks: producer (capture) → queue → consumer (inference)
- Target: ESP32-S3 (use idf.py set-target esp32s3)

How to use (quick)
1. Install ESP-IDF and VS Code ESP-IDF extension following Espressif docs.
2. Clone project folder (or create a folder and save the files above preserving paths).
3. Open the project folder in VS Code (ESP-IDF extension should detect it).
4. In terminal run:
   - idf.py set-target esp32s3
   - idf.py menuconfig   (set serial port, partitions if needed)
   - idf.py build
   - idf.py -p /dev/ttyUSB0 flash monitor   (Windows use COMx)

Notes & adjustments
- I2S pins (in components/audio_capture/src/audio_capture.c) are set as example:
  - BCK/SCK = GPIO13, WS = GPIO12, SD = GPIO15
  Adjust them to match your devboard wiring (or use board defaults).
- LED pin in main/main.c is GPIO2 by default; change if your board differs.
- To run real ML inference:
  - Train/quantize a small model (e.g., micro_speech int8), convert to C array and add a TFLM component.
  - Add TFLM sources or use Espressif TFLM component and integrate in components/inference.

Next steps I can do for you
- Integrate a quantized micro_speech .tflite into model_data.c and add TFLM invocation in inference.c.
- Make PlatformIO or Arduino-compatible version if you prefer that workflow.
- Provide exact pin mapping for a specific ESP32-S3 devboard (tell me the board model).
