#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wifi_selector.h"
#include "gpr_socket.h"
#include "../common/cJSON.h"
#include "../common/dir_control.h"

char *getWifiInfoList()
{
    float rssiList[WIFI_CHANNEL_CTN] = {0.0};
    int connectCntList[WIFI_CHANNEL_CTN] = {0};
    FILE *file;
    char buf[1024];

    char *command = "sudo iw wlan0 scan | egrep 'signal|DS Parameter set: channel'";
    file = popen(command, "r");
    if (file != NULL)
    {
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

void changeWifiChannel(char *channel)
{
    FILE *copyFile, *oriFile;
    char buf[1024];

    pclose(popen("sudo chmod 777 /etc/hostapd/hostapd.conf", "r"));

    oriFile = fopen("/etc/hostapd/hostapd.conf", "r");
    copyFile = fopen("/etc/hostapd/hostapd.conf.copy", "w");

    if (oriFile != NULL && copyFile != NULL)
    {
        while (fgets(buf, 1024, oriFile) != NULL)
        {
            if (strstr(buf, "channel=") != NULL)
            {
                char str[20];
                sprintf(str, "channel=%s\n", channel);
                fprintf(copyFile, str);
            }
            else
            {
                fprintf(copyFile, buf);
            }
        }
        pclose(popen("sudo mv /etc/hostapd/hostapd.conf.copy /etc/hostapd/hostapd.conf", "r"));
        fclose(copyFile);
        fclose(oriFile);
    }
    else
    {
        if (copyFile != NULL)
        {
            fclose(copyFile);
        }

        if (oriFile != NULL)
        {
            fclose(oriFile);
        }
    }
}

bool checkHostFile()
{
    FILE *file;
    char buf[1024];

    char *hostLines[HOST_LINE_CTN] = {
        "country_code=KR\n",
        "interface=wlan0\n",
        "ssid=GPR_MOBILE\n",
        "hw_mode=g\n",
        "channel=7\n",
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

    file = fopen("/etc/hostapd/hostapd.conf", "r");
    bool chekLines = true;
    int lineIndex = 0;

    if (file != NULL)
    {

        while (fgets(buf, 1024, file) != NULL && lineIndex < HOST_LINE_CTN)
        {
            if (lineIndex != 4)
            {
                if (strcmp(buf, hostLines[lineIndex]) != 0)
                {
                    chekLines = false;
                    break;
                }
            }
            else
            {
                int channelIndex = 0;
                bool findChannel = false;
                while (channelIndex < WIFI_CHANNEL_CTN)
                {
                    if (strcmp(buf, channelList[channelIndex]) == 0)
                    {
                        findChannel = true;
                        break;
                    }
                    channelIndex++;
                }
                if (findChannel == false)
                {
                    chekLines = false;
                    break;
                }
            }
            lineIndex++;
        }

        fclose(file);
    }

    if (lineIndex != HOST_LINE_CTN)
    {
        chekLines = false;
    }

    if (chekLines == false)
    {
        file = fopen("/etc/hostapd/hostapd.conf", "w");
        if (file != NULL)
        {
            for (int i = 0; i < HOST_LINE_CTN; i++)
            {
                fprintf(file, hostLines[i]);
            }
            fclose(file);
            chekLines = true;
            pclose(popen("sudo service hostapd restart", "r"));
        }
    }

    return chekLines;
}