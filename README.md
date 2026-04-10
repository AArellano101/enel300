# STM32 Code as of Apr. 3, 2026

This project uses an STM32 microcontroller to control **two DC motors**, communicate over **Bluetooth (HC-05)**, read a **metal detection circuit** using an **ADC input**, read object distance using an **HC-SR04** module, generate noise through a PWM-controlled speaker and a generate a light in an LED.

<br>

## Features

- **Dual motor control (DRV8833)** using PWM signals (2 pins per motor)
- **Bluetooth serial communication (HC-05, slave mode)** using USART TX/RX
- **Metal detection sensing** using an ADC input and a GPIO output
- **Object detection sensing** using microsecond-resolution pulses
- **Noise and light generation**

<br>

## Pin Assignments

## Pin Configuration Diagram

<img width="873" height="758" alt="image" src="https://github.com/user-attachments/assets/954a899f-dd80-4b6d-b3ea-b0b3bd5b427b" />

<br>

## STM Connections 
<img width="951" height="529" alt="image" src="https://github.com/user-attachments/assets/796a8c29-f1ea-4f1b-a6da-5ec752e6f932" />
