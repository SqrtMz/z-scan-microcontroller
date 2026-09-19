#pragma once

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

bool NO_SETUP_DEBUG = false;	// Changes the behaviour whether using the setup or just software debugging is needed
bool ADC_DEBUG = false;			// Enable printing information about the ADC pins

int ENAN_PIN = 34;				// Enable - pin
int ENAP_PIN = 35;				// Enable + pin
int DIRN_PIN = 32;				// Direction - pin
int DIRP_PIN = 33;				// Direction + pin
int PULN_PIN = 25;				// Pulse - pin
int PULP_PIN = 26;				// Pulse + pin

int LS_START_PIN = 14;			// Limit switch start pin
int LS_END_PIN = 27;			// Limit switch end pin

adsGain_t ADC_GAIN_OPTION[6] = {GAIN_TWOTHIRDS, GAIN_ONE, GAIN_TWO, GAIN_FOUR, GAIN_EIGHT, GAIN_SIXTEEN};

// GAIN_TWOTHIRDS 	±6.144 V
// GAIN_ONE 		±4.096 V
// GAIN_TWO 		±2.048 V
// GAIN_FOUR 		±1.024 V
// GAIN_EIGHT 		±0.512 V
// GAIN_SIXTEEN 	±0.256 V

int max_motor_speed = 1000;

// int MAX_MOTOR_SPEED = 31000; 	// 32 - 6400
// int MAX_MOTOR_SPEED = 15000; 	// 16 - 3200
// int MAX_MOTOR_SPEED = 7500; 		// 8 - 1600
// int MAX_MOTOR_SPEED = 4500; 		// 4 - 800
// int MAX_MOTOR_SPEED = 1850; 		// 2 - 400
// int MAX_MOTOR_SPEED = 1000; 		// 1 - 200

enum SystemState {
	IDLE,
	RUNNING
};

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

		if (c == '\0') break;
		else if (c == ',') {
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

void go_to_start(AccelStepper& stepper) {
	if (!NO_SETUP_DEBUG) {
		stepper.setSpeed(0);
		while (!digitalRead(LS_START_PIN)) {
			stepper.move(-100000);
			stepper.run();
		}
		stepper.stop();
		stepper.setSpeed(0);
		stepper.moveTo(stepper.currentPosition());
	}

	stepper.setCurrentPosition(0);
}

void go_to_end(AccelStepper& stepper) {
	if (!NO_SETUP_DEBUG) {
		stepper.setSpeed(0);
		while (!digitalRead(LS_END_PIN)) {
			stepper.move(100000);
			stepper.run();
		}
		stepper.stop();
		stepper.setSpeed(0);
		stepper.moveTo(stepper.currentPosition());
	}
}

void print_adc_debug(Adafruit_ADS1115& adc) {

	float phtd1_value = adc.readADC_Differential_1_3();
	float phtd2_value = adc.readADC_Differential_2_3();

	Serial.println("===========================");

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
	
	Serial.print("LS_START: ");
	Serial.println(digitalRead(LS_START_PIN));

	Serial.print("LS_END  : ");
	Serial.println(digitalRead(LS_END_PIN));
}