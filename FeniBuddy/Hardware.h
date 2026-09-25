#pragma once
// Existing, documented NodeMCU wiring. Change these only to match your wiring.
constexpr uint8_t TFT_CS = D0, TFT_DC = D1, TFT_RESET = D2;
constexpr uint8_t TOUCH_PIN = D6; // TTP223 OUT; VCC=3V3, GND=GND
constexpr bool TOUCH_ENABLED = false; // Enable after the TTP223 is soldered and connected.
constexpr bool TOUCH_ACTIVE_HIGH = true; // Momentary/direct mode, not toggle mode
constexpr uint8_t FLASH_BUTTON = D3; // Active-low onboard FLASH button; primary control.
constexpr uint8_t TFT_ROTATION = 1;
constexpr uint8_t TFT_TAB = INITR_BLACKTAB;
const char SETUP_SSID[] = "Feni-Setup";
const char SETUP_PASSWORD[] = "fenibuddy";
