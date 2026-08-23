#pragma once

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

bool DEBUG = true;				// Changes the behaviour whether using the setup or just software debugging is needed
bool ADC_DEBUG = false;			// Enable printing information about the ADC pins

int ENA_PIN = 16;				// Enable pin
int DIR_PIN = 17;				// Direction pin
int PUL_PIN = 5;				// Pulse pin

int LS_START_PIN = 18;			// Limit switch start pin
int LS_END_PIN = 19;			// Limit switch end pin

int AVERAGE_ITEMS = 1;			// Number of samples for average measurement of photodiodes

adsGain_t ADC_GAIN_OPTION[6] = {GAIN_TWOTHIRDS, GAIN_ONE, GAIN_TWO, GAIN_FOUR, GAIN_EIGHT, GAIN_SIXTEEN};

void read_incoming_data(char *incoming_data, String *commands) {

	int i = 0;
	while (Serial.available()) {
		char c = Serial.read();
		incoming_data[i] = c;
		i++;
	}

	i = 0;
	int j = 0;
	while (true) {
		char c = incoming_data[j];

		if (c == '\0') {
			break;
		} else if (c == ',') {
			i++;
			j++;
		} else {
			commands[i] += c;
			j++;
		}

	}
}

void print_data(float photo_diode_value1, float photo_diode_value2, AccelStepper& stepper) {
	Serial.print(photo_diode_value1);
	Serial.print(',');
	Serial.print(photo_diode_value2);
	Serial.print(',');
	Serial.println(stepper.currentPosition());
}

void go_to_start(int& lim_switch_start_pin, float& max_motor_speed, AccelStepper& stepper) {

	if (!DEBUG) {
		while (!digitalRead(lim_switch_start_pin)) {
			stepper.move(-100000);
			stepper.setSpeed(-max_motor_speed);
			stepper.run();
		}
	}

	stepper.stop();
	stepper.setCurrentPosition(0);
}

void go_to_end(int& lim_switch_end_pin, float& max_motor_speed, AccelStepper& stepper) {

	if (!DEBUG) {
		while (!digitalRead(lim_switch_end_pin)) {
			stepper.move(100000);
			stepper.setSpeed(max_motor_speed);
			stepper.run();
		}
	
		stepper.stop();
	}
}

void print_adc_debug(Adafruit_ADS1115& adc) {

	float phtd1_value = adc.readADC_Differential_1_3();
	float phtd2_value = adc.readADC_Differential_2_3();

	Serial.print("DIFF 1 - 3: ");
	Serial.println(adc.computeVolts(phtd1_value));
	
	Serial.print("DIFF 2 - 3: ");
	Serial.println(adc.computeVolts(phtd2_value));
	
	Serial.print("A1: ");
	Serial.println(adc.computeVolts(adc.readADC_SingleEnded(1)));
	
	
	Serial.print("A2: ");
	Serial.println(adc.computeVolts(adc.readADC_SingleEnded(1)));
	
	Serial.print("A3: ");
	Serial.println(adc.computeVolts(adc.readADC_SingleEnded(3)));
	
	Serial.println(digitalRead(LS_START_PIN));
	Serial.println("===========================");
}