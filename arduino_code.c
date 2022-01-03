#include <SoftwareSerial.h>
#include <FastLED.h>
#include <MsTimer2.h>

#define LED_BAR_PIN 2
#define SWITCH_RED_LED 3
#define SWITCH_BLUE_LED 4
#define SWITCH_GREEN_LED 5
#define NUM_LEDS 5

CRGB leds[NUM_LEDS];
SoftwareSerial raspi_serial(6, 7); // 2:RX 3:TX
int bootWaitCnt = 0;

void setup()
{
    pinMode(SWITCH_RED_LED, OUTPUT);
    pinMode(SWITCH_BLUE_LED, OUTPUT);
    pinMode(SWITCH_GREEN_LED, OUTPUT);
    digitalWrite(SWITCH_RED_LED, HIGH);
    raspi_serial.begin(57600);
    FastLED.addLeds<WS2812, LED_BAR_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(5);
    leds[0] = CRGB(0, 0, 255);
    FastLED.show();

    //대기 시간이 길거나 짧으면 여기 수정
    MsTimer2::set(2800, bootWaitFunc);
    MsTimer2::start();
}

void loop()
{
    if (raspi_serial.available())
    {
        String inString = raspi_serial.readStringUntil('\n');
        MsTimer2::stop();
        if (inString.indexOf("boot completed") >= 0)
        {
            bootSuccess();
        }
        else if (inString.indexOf("battery gauge:") >= 0)
        {
            int tmep_index = inString.indexOf(":");
            inString = inString.substring(tmep_index + 1);
            batteryState(inString.toInt());
        }
        else if (inString.indexOf("server on") >= 0)
        {
            digitalWrite(SWITCH_BLUE_LED, HIGH);
        }
        else if (inString.indexOf("server off") >= 0)
        {
            digitalWrite(SWITCH_BLUE_LED, LOW);
        }
        else if (inString.indexOf("client on") >= 0)
        {
            digitalWrite(SWITCH_GREEN_LED, HIGH);
        }
        else if (inString.indexOf("client off") >= 0)
        {
            digitalWrite(SWITCH_GREEN_LED, LOW);
        }
    }
}

void bootWaitFunc()
{
    bootWaitCnt++;
    if (bootWaitCnt < NUM_LEDS)
    {
        leds[bootWaitCnt] = CRGB(0, 0, 255);
        FastLED.show();
    }
    else
    {
        MsTimer2::stop();
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB(255, 0, 0);
        }
        FastLED.show();
    }
}

void bootSuccess()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB(0, 255, 0);
    }
    FastLED.show();
    delay(1000);
}

void batteryState(int batteryGauge)
{
    if (batteryGauge > 80)
    {
        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = CRGB(0, 255, 0);
        }
    }
    else if (batteryGauge > 60)
    {
        batteryUpdate(1);
    }
    else if (batteryGauge > 40)
    {
        batteryUpdate(2);
    }
    else if (batteryGauge > 20)
    {
        batteryUpdate(3);
    }
    else
    {
        batteryUpdate(4);
    }
    FastLED.show();
}

void batteryUpdate(int offCnt)
{
    for (int i = 0; i < NUM_LEDS - offCnt; i++)
    {
        leds[i] = CRGB(0, 255, 0);
    }

    for (int i = NUM_LEDS - offCnt; i < NUM_LEDS; i++)
    {
        leds[i] = CRGB(0, 0, 0);
    }
}