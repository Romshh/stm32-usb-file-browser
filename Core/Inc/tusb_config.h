#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#define CFG_TUSB_MCU                 OPT_MCU_STM32F7
#define CFG_TUSB_OS                  OPT_OS_NONE
#define CFG_TUSB_DEBUG               0

#define CFG_TUH_ENABLED              1
#define CFG_TUH_MAX_SPEED            OPT_MODE_FULL_SPEED

#define CFG_TUH_MSC                  1
#define CFG_TUH_CDC                  0
#define CFG_TUH_HID                  0
#define CFG_TUH_VENDOR               0
#define CFG_TUH_HUB                  0
#define CFG_TUH_DEVICE_MAX           1

#define CFG_TUH_MEM_SECTION
#define CFG_TUH_MEM_ALIGN            __attribute__((aligned(4)))
#define CFG_TUH_ENUMERATION_BUFSIZE  256

#endif
