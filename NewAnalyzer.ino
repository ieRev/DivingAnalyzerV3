#include <SimpleDHT.h>
#include <Adafruit_ADS1015.h>
#include <uRTCLib.h>
#include "Wire.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "battery.h"
#include <RunningAverage.h>

#define RA_SIZE 20
#define MINVOLTAGE 3000
#define MAXVOLTAGE 4200
#define BATTERY_SENSE A0
#define USB_CONNECTED 6

#define TFT_DC 8
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 9
#define TFT_MISO 12
int pinDHT22 = 2;


Adafruit_ADS1115 ads1115(0x48);
uRTCLib rtc(0x68);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
Battery battery(MINVOLTAGE, MAXVOLTAGE, BATTERY_SENSE);
RunningAverage batteryRA(RA_SIZE);
SimpleDHT22 dht22(pinDHT22);

/*
 * This is bad coding.. but idc
 */
unsigned long currMillis = 0;
unsigned long prevMillis = 0;
int prevYear = 0;
uint8_t prevMonth = 0;
uint8_t prevDay = 0;
uint8_t prevHour = 0;
uint8_t prevMin = 0;
uint8_t prevSec = 0;
float prevTemp = 0;
float prevHumidity = 0;
float prevPercent = 0;
int prevBattery = 0;
int prevHundreds = 0;
int prevTens = 0;
int prevUnits = 1;
int prevDecimal = 0;
int currYear = 0;
uint8_t currMonth = 0;
uint8_t currDay = 0;
uint8_t currHour = 0;
uint8_t currMin = 0;
uint8_t currSec = 0;
float currTemp = 0;
float currHumidity = 0;
float currPercent = 0;
int currBattery = 100;
long int calibration = 0;
int currHundreds = 0;
int currTens = 0;
int currUnits = 0;
int currDecimal = 0;
int battMillsStart = 0;
int battMillsEnd = 0;

void tftDiag(){
	
	// read diagnostics (optional but can help debug problems)
	uint8_t x = tft.readcommand8(ILI9341_RDMODE);
	Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDMADCTL);
	Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDPIXFMT);
	Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDIMGFMT);
	Serial.print("Image Format: 0x"); Serial.println(x, HEX);
	x = tft.readcommand8(ILI9341_RDSELFDIAG);
	Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
}

void drawGrid(){
	
	// Draw static parts of screen
	
	tft.drawLine(0, 25, 320, 25, ILI9341_BLUE);
	tft.drawLine(0, 215, 320, 215, ILI9341_BLUE);

	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setTextSize(2);

	tft.setCursor(58,5);
	tft.print('/');
	tft.setCursor(95,5);
	tft.print('/');
		
	tft.setCursor(163, 5);
	tft.print(':');

	tft.setCursor(10, 220);
	tft.print("Temp: ");

	tft.setCursor(130, 220);
	tft.print('C');

	tft.setCursor(230, 220);
	tft.print("RH: ");

	tft.setCursor(300, 220);
	tft.print('%');
	
	tft.setTextSize(10);
	tft.setCursor(120, 80);
	tft.print('.');
	tft.setCursor(250, 80);
	tft.print('%');


}

void calculateUnits(){
	int working = 0;

	working = currPercent * 10;
	currHundreds = working / 1000;
	working = working - (currHundreds * 1000);
	currTens = working / 100;
	working = working - (currTens * 100);
	currUnits = working / 10;
	working = working - (currUnits * 10);
	currDecimal = working;
}

unsigned long printDisplay(){

	unsigned long start = micros();
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setTextSize(2);

// Print Date:
	if(currYear != prevYear){
		tft.setCursor(10,5);
		tft.print(currYear);
	}
	if(currMonth != prevMonth){
		tft.setCursor(70,5);
		if(currMonth < 10){
			tft.print('0');
		}
		tft.print(currMonth);
	}

	if(currDay != prevDay){
		tft.setCursor(106, 5);
		if(currDay < 10){
			tft.print('0');
		}	
		tft.print(currDay);
	}

//Print Time:

	if(currHour == prevHour){}
	else{
		tft.setCursor(140, 5);
		if(currHour < 10){
			tft.print('0');
		}
		tft.print(currHour);
	}
	if(currMin == prevMin){}
	else{
		tft.setCursor(174, 5);
		if(currMin < 10){
			tft.print('0');
		}
		tft.print(currMin);
	}
//Print Battery Percentage:

	if(currBattery == prevBattery){}
	else{
		tft.setCursor(265, 5);
		if(currBattery > 59){
			tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
		}
		else if(currBattery > 15 && currBattery < 60){
			tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
		}
		else {
			tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
		}
	
		if(currBattery < 100){
			tft.print(" ");
			tft.print(currBattery);
			tft.print('%');
		}
		else if(currBattery < 10){
			tft.print("	");
			tft.print(currBattery);
			tft.print('%');
		}
		else{
			tft.print(currBattery);
			tft.print('%');
		}
	}
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

// Print Temperature & Humidity

	if(currTemp == prevTemp){}
	else{
		tft.setCursor(70, 220);
		tft.print(currTemp, 1);
	
		tft.setCursor(270, 220);
		tft.print(currHumidity, 0);
	}

// Print Percentage
	calculateUnits();
	if (currPercent != prevPercent) {

		tft.setCursor(10, 80);
		tft.setTextSize(10);

		if (currPercent < 18.0)
		{
			tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
		}
		else if (currPercent > 21.9)
		{
			tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
		}
		else
		{
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
		}
	}
		// Do stuff here to improve draw speed


		
		tft.setCursor(10, 80);
		if (currHundreds != prevHundreds) {
			if (currHundreds >= 1) {
				tft.setCursor(10, 80);
				tft.print(currPercent, 0);
			}
		}
		
		if (currTens != prevTens) {
			tft.setCursor(10, 80);
			if (currTens == 0) {
				tft.print(' ');
			}
			tft.print(currTens);
		}

		if (currUnits != prevUnits) {
			tft.setCursor(70, 80);
			tft.print(currUnits);
		}

		if (currDecimal != prevDecimal) {
			tft.setCursor(180, 80);
			tft.print(currDecimal);
		}
		
	//set current values as previous values
	prevYear = currYear;
	prevMonth = currMonth;
	prevDay = currDay;
	prevHour = currHour;
	prevMin = currMin;
	prevBattery = currBattery;
	prevTemp = currTemp;
	prevHumidity = currHumidity;
	prevPercent = currPercent;
	prevHundreds = currHundreds;
	prevTens = currTens;
	prevUnits = currUnits;
	prevDecimal = currDecimal;

	return micros() - start;
}

void readRTC(){

	rtc.refresh();
	
	currYear = rtc.year();
	currYear = currYear + 2000;
	currMonth = rtc.month();
	currDay = rtc.day();
	currHour = rtc.hour();
	currMin = rtc.minute();
	currSec = rtc.second();

}

void readDHT(){
	
	int err = SimpleDHTErrSuccess;
	if ((err = dht22.read2(&currTemp, &currHumidity, NULL)) != SimpleDHTErrSuccess) {
		Serial.print("Read DHT22 failed, err="); 
		Serial.println(err);
		return;
	}	
}

void readSensor(){

	float reading = 0;
	reading = ads1115.readADC_Differential_0_1();
	if(reading < 0){
		reading *= -1;
	}
	currPercent = round(( reading / calibration ) * 20.9 * 10) / 10.0;
}

void readBattery() {

	batteryRA.addValue(battery.level());
}

void calibrateSensor(){
		
	for(int i=0; i<10; i++)
	{
		calibration = calibration + ads1115.readADC_Differential_0_1();
		delay(100);
	}

	calibration = calibration / 10;
	if(calibration < 0){
		calibration *= -1;
	}
}

void setup() {

	Serial.begin(115200);
	Wire.begin();
	battery.begin(5000, 1.0, &sigmoidal);
	ads1115.setGain(GAIN_SIXTEEN);
	ads1115.begin();
	tft.begin();

	tftDiag();

	tft.setRotation(1);
	tft.fillScreen(ILI9341_BLACK);
	
	drawGrid();

	calibrateSensor();
}

void loop() {
	/*
	 *	1: Read Sensors
	 *	2: Update Display
	 *	3: Write to Serial
	 */
	currMillis = millis();
	unsigned long displayUpdateTime = 0;
	
	readRTC();
	readSensor();

	if (currMillis - prevMillis > 5000)
	{
		prevMillis = currMillis;
		readBattery();
		currBattery = batteryRA.getAverage();

		readDHT();
	}

	displayUpdateTime = printDisplay();
}
