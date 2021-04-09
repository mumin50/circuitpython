USB_VID = 0x239A
USB_PID = 0x805C
USB_PRODUCT = "Pyboard Version 1.1"
USB_MANUFACTURER = "George Robotic"

INTERNAL_FLASH_FILESYSTEM = 1

MCU_SERIES = F4
MCU_VARIANT = STM32F405xx
MCU_PACKAGE = LQFP64

LD_COMMON = boards/common_default.ld
LD_FILE = boards/STM32F405_fs.ld

CIRCUITPY_RGBMATRIX ?= 1
