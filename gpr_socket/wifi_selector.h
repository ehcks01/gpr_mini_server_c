#include <stdbool.h>

#define WIFI_CHANNEL_CTN 13
#define HOST_LINE_CTN 8

char *getWifiInfoList();
void changeWifiChannel(char *channel);
bool checkHostFile();