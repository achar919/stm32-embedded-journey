/**
 * Configuration for the stm32-ssd1306 library (MIT, Aleksander Alekseev)
 * Set up for: NUCLEO-F446RE, I2C1 (PB8 = SCL, PB9 = SDA),
 *             1.3" 128x64 OLED with SH1106 controller.
 */
#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

#define STM32F4                         // our microcontroller family

#define SSD1306_USE_I2C                 // talk over I2C (not SPI)
#define SSD1306_I2C_PORT   hi2c1        // the I2C handle CubeMX creates
#define SSD1306_I2C_ADDR   (0x3C << 1)  // 7-bit address 0x3C, shifted for HAL

// 1.3" displays usually use the SH1106 chip: its picture starts 2 columns in.
// If you see a thin garbage line on one edge, change 2 to 0 (for SSD1306 chips).
#define SSD1306_X_OFFSET   2

// Fonts we use (fewer fonts = less Flash used)
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18

#endif /* __SSD1306_CONF_H__ */
