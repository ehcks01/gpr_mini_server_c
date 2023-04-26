#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "wifi_selector.h"
#include "gpr_socket.h"
#include "../common/cJSON.h"
#include "../common/dir_control.h"

//라즈베리에서 스캔한 와이파이 채널 상태정보
char *getWifiInfoList()
{
    float rssiList[WIFI_CHANNEL_CTN] = {0.0};
    int connectCntList[WIFI_CHANNEL_CTN] = {0};
    FILE *file;
    char buf[1024];

    //라즈베리에서 지원하는 와이파이 채널상태를 명령어로 조회
    char *command = "sudo iw wlan0 scan | egrep 'signal|DS Parameter set: channel'";
    file = popen(command, "r");
    if (file != NULL)
    {
        //조회된 내용을 배열 데이터 형식으로 변경
        float signal = 0.0;
        while (fgets(buf, 1024, file) != NULL)
        {

            if (strstr(buf, "signal") != NULL)
            {
                char *ptr = strtok(buf, " ");
                //ex)signal: -88.00 dBm
                ptr = strtok(NULL, " ");
                if (ptr != NULL)
                {
                    signal = atof(ptr);
                }
            }
            else if (strstr(buf, "DS Parameter set: channel") != NULL)
            {
                char *ptr = strtok(buf, " ");
                //ex)DS Parameter set: channel 11
                for (int i = 0; i < 4; i++)
                {
                    ptr = strtok(NULL, " ");
                }
                if (ptr != NULL)
                {
                    int channel = atoi(ptr);
                    if (channel > 0 && channel < 14)
                    {
                        if (rssiList[channel - 1] == 0.0 || rssiList[channel - 1] < signal)
                        {
                            rssiList[channel - 1] = signal;
                        }
                        connectCntList[channel - 1]++;
                    }
                }
            }
        }
    }
    pclose(file);

    //라즈베리에서 현재 와이파이 채널상태를 명령어로 조회
    char *currentStr = "";
    file = popen("sudo iw wlan0 info | grep channel", "r");
    if (file != NULL)
    {
        //ex) channel 1 (2412 MHz), width: 20 MHz, center1: 2412 MHz
        if (fgets(buf, 1024, file) != NULL)
        {
            strtok(buf, " ");
            char *ptr = strtok(NULL, " ");
            if (ptr != NULL)
            {
                currentStr = ptr;
            }
        }
    }
    pclose(file);

    //조회한 와이파이 채널 정보를 json 데이터로 변경
    cJSON *root = cJSON_CreateObject();
    cJSON *channelArray = cJSON_CreateArray();
    for (int i = 0; i < WIFI_CHANNEL_CTN; i++)
    {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddItemToObject(item, "rssi", cJSON_CreateNumber(rssiList[i]));
        cJSON_AddItemToObject(item, "cnt", cJSON_CreateNumber(connectCntList[i]));
        cJSON_AddItemToArray(channelArray, item);
    }
    cJSON_AddItemToObject(root, "channels", channelArray);
    cJSON_AddItemToObject(root, "current", cJSON_CreateString(currentStr));

    char *out = cJSON_Print(root);
    cJSON_Delete(root);

    return out;
}

//와이파이 채널 변경
void changeWifiChannel(char *channel)
{
    //와이파이 채널 변경을 위해 라즈베리에서 hostapd.conf 파일 내용을 변경
    FILE *file;
    char buf[1024];

    char *hostLines[7] = {
        "country_code=KR\n",
        "interface=wlan0\n",
        "ssid=GPR_MOBILE\n", //ssid 확인!!! 채널 변경하면 이 ssid로 저장됨!!
        "hw_mode=g\n",
        "macaddr_acl=0\n",
        "auth_algs=1\n",
        "ignore_broadcast_ssid=0\n"};

    char *channelList[WIFI_CHANNEL_CTN] = {
        "channel=1\n",
        "channel=2\n",
        "channel=3\n",
        "channel=4\n",
        "channel=5\n",
        "channel=6\n",
        "channel=7\n",
        "channel=8\n",
        "channel=9\n",
        "channel=10\n",
        "channel=11\n",
        "channel=12\n",
        "channel=13\n"};

    //pclose(popen("sudo chmod 777 /etc/hostapd/hostapd.conf", "r"));
    file = fopen("/etc/hostapd/hostapd.conf", "w");
    if (file != NULL)
    {
        for (int i = 0; i < 7; i++)
        {
            fprintf(file, hostLines[i]);
        }
        fprintf(file, channelList[atoi(channel)-1]);
        fclose(file);
    }
}

// wifi 채널 변경을 했을 때, 라즈베리에 관련 명령어 수행
void wifiRestartCommand()
{
    pclose(popen("sudo systemctl restart hostapd", "r"));
}