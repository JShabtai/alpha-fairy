#ifndef _ALFY_CONF_H_
#define _ALFY_CONF_H_

//#define WIFI_AP_UNIQUE_NAME
#ifndef WIFI_AP_UNIQUE_NAME
#define WIFI_DEFAULT_SSID "fairywifi"
#else
#define WIFI_DEFAULT_SSID "fairy"
#endif
#define WIFI_DEFAULT_PASS  "1234567890"

//#define DISABLE_STATUS_BAR
//#define DISABLE_POWER_SAVE
#define DISABLE_ALL_MSG
#define DISABLE_CMD_LINE
//#define HTTP_MOCKBTNS_ENABLE
//#define HTTP_ON_BOOT
//#define HTTP_ENABLE_CMD_INTEFACE
//#define PMIC_LOG_ON_BOOT

#define WIFIPROFILE_LIMIT 20
#define WIFI_STRING_LEN 30

#define USE_SPRITE_MANAGER

#define SHUTTER_GPIO 26
#define SHUTTER_GPIO_ACTIVE_LOW
//#define SHUTTER_GPIO_ACTIVE_HIGH

#define SUBMENU_X_OFFSET 8
#define SUBMENU_Y_OFFSET 8
#define MICTRIG_LEVEL_MARGIN      16

#define BTN_DEBOUNCE 50

#endif
