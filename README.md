# CM4_LiteRTOS

<p align="center">
    <a href="https://github.com/imahjoub/CM4_LiteRTOS/actions">
        <img src="https://github.com/imahjoub/CM4_LiteRTOS/actions/workflows/CM4_LiteRTOS.yml/badge.svg" alt="Build Status"></a>
    <a href="https://github.com/imahjoub/CM4_LiteRTOS/issues">
        <img src="https://custom-icon-badges.herokuapp.com/github/issues-raw/imahjoub/CM4_LiteRTOS?logo=github%22%20alt=%22Issues" /></a>
    <a href="https://github.com/imahjoub/CM4_LiteRTOS/blob/main/LICENSE">
        <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="MIT License"></a>
    <a href="https://github.com/imahjoub/CM4_LiteRTOS" alt="GitHub code size in bytes">
        <img src="https://img.shields.io/github/languages/code-size/imahjoub/CM4_LiteRTOS" /></a>
    <a href="https://github.com/imahjoub/CM4_LiteRTOS" alt="Activity">
        <img src="https://img.shields.io/github/commit-activity/y/imahjoub/CM4_LiteRTOS" /></a>
</p>


CM4_LiteRTOS is a minimal, high-performance real-time operating system designed for ARM Cortex-M4 microcontrollers.  
It provides essential multitasking features like thread scheduling, time delays, and context switching — implemented in clean, bare-metal C with a small footprint.

Although the current example targets the STM32F446RE, the core OS is hardware-agnostic and can be adapted to any Cortex-M4 MCU with minimal changes.

## Features
- **Preemptive scheduling** with PendSV handler
- **Round-robin task switching**
- **Configurable thread priorities**
- **Blocking delays with millisecond granularity**
- **Idle task with low-power hooks**
- **Compact footprint** — minimal RAM/flash usage
- **Easily portable** to other Cortex-M4 MCUs

## Example
The provided `App.c` demonstrates running two concurrent tasks:
- A blinking LED task
- A GPIO toggle task

## Supported MCUs
- Tested on STM32F446RE
- Portable to any ARM Cortex-M4 with minimal adaptation

