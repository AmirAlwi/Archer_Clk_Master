#include <Arduino.h>
#include <Keypad.h>
#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h> 
#include <Adafruit_GFX.h>
#include <Wire.h>

#define ROW 4
#define COLUMN 4

enum status{
	start = 0,
	stop,
	reset,
	idle,
	busy,
};

enum seq {
	PA = 0b0001,
	PB = 0b0010,
	PC = 0b0100,
	PD = 0b1000,
	PAB = 0b0011,
	PCD = 0b1100,
	PABCD = 0b1111,
};

uint8_t broadcastAddress[] = {0x3C,0x61,0x05,0x03,0x50,0xF0}; // 0xC4,0x5B,0xBE,0x3F,0x4C,0xD8
Adafruit_SSD1306 display(128, 64, &Wire, -1);
esp_now_peer_info_t peerInfo;

typedef struct struct_message
{
	char data;
} struct_message;

struct_message mydata;
int player=PABCD;
int status = idle;
int success = 1;

unsigned long ref_time = 0, lastmillis= 0;
int start_duration = 10;
int timer = reset;
int duration = 0, countdown = 0;
bool isreading = false;

char keys[ROW][COLUMN] = {
	{'A', 'B', 'C', 'D'},
	{'E', 'F', 'G', 'H'},
	{'I', 'J', 'K', 'L'},
	{'M', 'N', 'O', 'P'}};

byte pin_rows[ROW] = {19, 18, 5, 17}; // byte pin_rows[ROW] = {12, 14, 27, 26};
byte pin_column[COLUMN] = {16, 4, 0, 2}; // byte pin_column[COLUMN] = {25, 33, 32, 35};

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW, COLUMN);

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t _status)
{
	Serial.println(_status == ESP_NOW_SEND_SUCCESS ? "Send OK" : "Send Fail");
	success = _status;
	isreading = true;
}

void get_timer(int _input)
{
	if (_input == 1)
		duration = 30;
	else if (_input == 2)
		duration = 60;
	else if (_input == 3)
		duration = 90;
	else if (_input == 4)
		duration = 120;
	else if (_input == 5)
		duration = 150;
	else if (_input == 6)
		duration = 180;
}

void get_seq(int _input)
{
	if (_input == 7)
	{
	if(player&PA)
		player &= (~PA);
	else
		player |= (PA);
	}
	else if (_input == 8)
	{
	if(player&PB)
		player &= (~PB);
	else
		player |= (PB);
	}
	else if (_input == 9)
	{
	if(player&PC)
		player &= (~PC);
	else
		player |= (PC);
	}
	else if (_input == 10)
	{
	if(player&PD)
		player &= (~PD);
	else
		player |= (PD);
	}
	else if (_input == 11)
	{
	if(player&PAB)
		player &= (~PAB);
	else
		player |= (PAB);
	}
	else if (_input == 12)
	{
	if(player&PCD)
		player &= (~PCD);
	else
		player |= (PCD);
	}
	else if (_input == 13)
	{
	if(player&PABCD)
		player &= (~PABCD);
	else
		player |= (PABCD);
	}
}

void print_p(int x, int y, char _char)
{
  display.setCursor(x,y);
  display.print(_char);
}

void disp_player()
{
	if(player&1)
		print_p(0,27,'A');
	if((player>>1)&1)
		print_p(20,27,'B');
	if((player>>2)&1)
		print_p(0,45,'C');
	if((player>>3)&1)
		print_p(20,45,'D');
}

void disp_time(int _val)
{
	display.setCursor(5, 0);
	display.print("Timer:");
	display.println(String(_val));
	display.drawLine(0,20,128,20,SSD1306_WHITE);
}

void conn_ok()
{
	display.drawCircle(108,44,10,SSD1306_WHITE);
	display.drawCircle(108,44,6,SSD1306_WHITE);
	display.drawCircle(108,44,4,SSD1306_WHITE);
}

void disp_status()
{
	if(timer == start){
		display.drawLine(52,37,52,52,SSD1306_WHITE);
		display.drawLine(52,37,72,45,SSD1306_WHITE);
		display.drawLine(53,39,53,50,SSD1306_WHITE);
		display.drawLine(53,39,71,45,SSD1306_WHITE);
		display.drawLine(54,41,54,48,SSD1306_WHITE);
		display.drawLine(54,41,70,45,SSD1306_WHITE);
		display.drawLine(55,43,54,46,SSD1306_WHITE);
		display.drawLine(55,43,70,45,SSD1306_WHITE);
		display.drawLine(56,43,54,44,SSD1306_WHITE);
		display.drawLine(55,46,70,45,SSD1306_WHITE);
		display.drawLine(54,48,70,45,SSD1306_WHITE);
		display.drawLine(53,50,71,45,SSD1306_WHITE);
		display.drawLine(52,52,72,45,SSD1306_WHITE);
		display.drawLine(52,53,74,45,SSD1306_WHITE);
		display.drawLine(52,54,74,45,SSD1306_WHITE);
		return;
	}
	else
		display.drawLine(52,36,56,36,SSD1306_WHITE);
		display.drawLine(52,38,56,38,SSD1306_WHITE);
		display.drawLine(52,40,56,40,SSD1306_WHITE);
		display.drawLine(52,42,56,42,SSD1306_WHITE);
		display.drawLine(52,44,56,44,SSD1306_WHITE);
		display.drawLine(52,46,56,46,SSD1306_WHITE);
		display.drawLine(52,48,56,48,SSD1306_WHITE);
		display.drawLine(52,50,56,50,SSD1306_WHITE);

		display.drawLine(62,36,66,36,SSD1306_WHITE);
		display.drawLine(62,38,66,38,SSD1306_WHITE);
		display.drawLine(62,40,66,40,SSD1306_WHITE);
		display.drawLine(62,42,66,42,SSD1306_WHITE);
		display.drawLine(62,44,66,44,SSD1306_WHITE);
		display.drawLine(62,46,66,46,SSD1306_WHITE);
		display.drawLine(62,48,66,48,SSD1306_WHITE);
		display.drawLine(62,50,66,50,SSD1306_WHITE);
}

void oled_init()
{
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
	{
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			; // infinite loop, before proceed
	}

	display.display();
	delay(1000);
	display.clearDisplay();
	display.setTextSize(2, 2);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(15, 15);
	display.println("initialisation");
	display.display();
	delay(500);
	display.clearDisplay();
	disp_status();
	disp_time(duration);
	disp_player();
	display.display();
	
}

void setup()
{
	Serial.begin(115200);

	oled_init();

	WiFi.mode(WIFI_MODE_STA);
	Serial.println(WiFi.macAddress());

	if (esp_now_init() != ESP_OK){
		Serial.println("error initialisation");
		return;
	}

	esp_now_register_send_cb(onDataSent);

	memcpy(peerInfo.peer_addr, broadcastAddress, 6);
	peerInfo.channel = 0;
	peerInfo.encrypt = false;

	if (esp_now_add_peer(&peerInfo) != ESP_OK){
		Serial.println("Failed to add peer");
		return;
	}

	ref_time = millis();
}

void display_set()
{
	disp_status();
	disp_player();
	if(!success)
		conn_ok();
	display.display();
}

void get_cmd(char _input, int *datacp)
{
	if (_input == 'A')
		*datacp = 1;
	else if (_input == 'B')
		*datacp = 2;
	else if (_input == 'C')
		*datacp = 3;
	else if (_input == 'E')
		*datacp = 4;
	else if (_input == 'F')
		*datacp = 5;
	else if (_input == 'G')
		*datacp = 6;
	else if (_input == 'M')
		*datacp = 7;
	else if (_input == 'N')
		*datacp = 8;
	else if (_input == 'O')
		*datacp = 9;
	else if (_input == 'P')
		*datacp = 10;
	else if (_input == 'I')
		*datacp = 11;
	else if (_input == 'J')
		*datacp = 12;
	else if (_input == 'K')
		*datacp = 13;
	else if (_input == 'D')
		*datacp = 14;
	else if (_input == 'H')
		*datacp = 15;
}

void check_conn()
{
	char test = 'T';
	esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&test, sizeof(char)); 
	delay(20);

	isreading = false;
	
	display.clearDisplay();
	disp_time(countdown);
	display_set();
	ref_time = millis()	;
}

void loop()
{
  // read the state of the switch into a local variable:
	char reading = 0;
	int datacp = 0;

	if (status == idle){
		while(status==idle){
			
			reading = keypad.getKey();
			mydata.data = reading;
			get_cmd(reading, &datacp);

			if(reading){
				esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&mydata, sizeof(mydata)); 	
				ref_time = millis();
			}

			if (reading == 0 && (millis() - ref_time) > 5000)
				check_conn();

			if (isreading == false)
				goto END;

			isreading = false;
			if (success)
				goto END;

			if(datacp >= 1 && datacp <= 6){ //timer
				get_timer(datacp);
				countdown = duration;   
			}

			if (datacp >= 7 && datacp <= 13){
				get_seq(datacp);
			}

			if(datacp == 14 || datacp == 15){
				if (datacp == 14){//start
					timer = start;
					status = busy;
					Serial.println("start");
					break;
				} else if(datacp == 15){ //reset
					duration = 0;
					countdown = 0;
					start_duration = 10;
				}
			}

			END:
			display.clearDisplay();
			disp_time(countdown);
			display_set();
		}

	}else if (status == busy){
		lastmillis = millis();
		while(status == busy){
			reading = keypad.getKey();
			mydata.data = reading;
			get_cmd(reading, &datacp);

			if(reading){
				esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&mydata, sizeof(mydata)); 	
				ref_time = millis();
			}

			if (reading == 0 && (millis() - ref_time) > 5000 && start_duration < 0)
				check_conn();

			if (isreading == false)
				goto TIMER;

			isreading = false;
			if (success){
				goto TIMER;
			}

			if(datacp == 14 || datacp == 15){

				if ( datacp == 14){//start/stop
					if(timer == start)
						timer = stop;
					else if(timer == stop){
						timer = start;
						Serial.println("start");
					}
				} else if(datacp == 15 && timer == stop){ //reset
					if(player==PAB)
						player=PCD;
					else if(player==PCD)
						player=PAB;
						
					status = idle;

					countdown = duration;
					start_duration = 10;
					display.clearDisplay();
					disp_time(countdown);
					display_set();
					break;
				}

			}
			
			TIMER:
			if ((timer == start )&& (millis()-lastmillis >= 1000)){

				if (countdown <= 0){	
					timer = stop;
					status = idle;
					countdown = duration;
					start_duration = 10;
					
					if(player==PAB)
						player=PCD;
					else if(player==PCD)
						player=PAB;

					display.clearDisplay();
					disp_time(countdown);
					display_set();
					break;
				}

				display.clearDisplay();
				if (start_duration < 1){
					countdown = countdown - 1;
					disp_time(countdown);
				} else {
					start_duration = start_duration - 1;
					disp_time(start_duration);
				}
				display_set();

				lastmillis = millis();
			}
		}
	}
}
