
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gpr_socket.h"
#include "../common/cJSON.h"
#include "../common/dir_control.h"

#define WIFI_CHANNEL_CTN 13

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
                    if (channel > 0 && channel < 12)
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
    file = popen("iw wlan0 info | grep channel", "r");
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
    FILE *Orifile, *file;
    char buf[1024];

    file = popen("sudo mv /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.conf.ori", "r");
    pclose(file);
    file = popen("sudo chmod 777 /etc/hostapd/hostapd.conf.ori", "r");
    pclose(file);

    file = fopen("/etc/hostapd/hostapd.conf", "w");
    Orifile = fopen("/etc/hostapd/hostapd.conf.ori", "r");

    if (file != NULL && Orifile != NULL)
    {
        while (fgets(buf, 1024, Orifile) != NULL)
        {
            if (strstr(buf, "channel=") != NULL)
            {
                char str[20];
                sprintf(str, "channel=%s\n", channel);
                fprintf(file, str);
            }
            else
            {
                fprintf(file, buf);
            }
        }
    }

    if (file != NULL)
    {
        fclose(file);
    }

    if (Orifile != NULL)
    {
        fclose(Orifile);
    }

    file = popen("sudo rm /etc/hostapd/hostapd.conf.ori", "r");
    pclose(file);
}