#include "ibox_config.h"

IBOX_CONFIG ibox_config = {
    "192.168.1.1", // server ip
    "8000",        // port
    "fanwenl.top", // dsn
#ifdef USE_WIFI
    "ziroom102",  // wifi_ssid[];
    "4001001111", // wifi_password[];
    "0",
#else
    0,
#endif
};