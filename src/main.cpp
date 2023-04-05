#include <Arduino.h>
#include <Keypad.h>
#include <WiFi.h>
#include <esp_now.h>

typedef struct struct_message
{
  char data;
} struct_message;

struct_message mydata;

uint8_t broadcastAddress[] = {0x3C, 0x61, 0x05, 0x03, 0x50, 0xF0};
// 0xC4,0x5B,0xBE,0x3F,0x4C,0xD8
// 0x3C,0x61,0x05,0x03,0x50,0xF0

esp_now_peer_info_t peerInfo;

#define ROW 4
#define COLUMN 4

char keys[ROW][COLUMN] = {
    {'A', 'B', 'C', 'D'},
    {'E', 'F', 'G', 'H'},
    {'I', 'J', 'K', 'L'},
    {'M', 'N', 'O', 'P'}};

byte pin_rows[ROW] = {19, 18, 5, 17};
byte pin_column[COLUMN] = {16, 4, 0, 2};

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW, COLUMN);
int counter = 0;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Send Success" : "Send Fail");
}

void setup()
{
  Serial.begin(9600);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("error initialisation");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop()
{
  // read the state of the switch into a local variable:
  char reading = keypad.getKey();

  if (reading)
  {
    mydata.data = reading;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&mydata, sizeof(mydata));

    if (result == ESP_OK)
      Serial.println("Success");
    else
      Serial.println("fail");
  }
}
