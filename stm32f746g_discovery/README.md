# STM32F746G-DISCOVERY project skeleton

This folder contains source and header files for the STM32CubeIDE skeleton for the Edge Keyword Spotter.

Files:
- Src/main.c  (already committed)
- Src/audio_capture.c
- Inc/audio_capture.h
- Src/inference.c
- Inc/inference.h
- Src/model_data.c (placeholder)
- README.md

Instructions:
1. In STM32CubeIDE create a new project for STM32F746G-DISCO (or import this folder's files into an existing project). If you want a fully generated .ioc, open CubeIDE and generate the project and peripheral code (I2S, DMA, FreeRTOS).
2. Copy the files from this repository into the project's Src/ and Inc/ directories and adjust peripheral handle names if needed (hi2s2 etc.).
