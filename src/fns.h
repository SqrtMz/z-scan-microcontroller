#pragma once

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

bool NO_SETUP_DEBUG = false;				// Changes the behaviour whether using the setup or just software debugging is needed
bool ADC_DEBUG = false;						// Enable printing information about the ADC pins

int ENA_PIN = 16;				// Enable pin
int DIR_PIN = 17;				// Direction pin
int PUL_PIN = 5;				// Pulse pin

int LS_START_PIN = 18;			// Limit switch start pin
int LS_END_PIN = 19;			// Limit switch end pin

int AVERAGE_ITEMS = 1;			// Number of samples for average measurement of photodiodes

adsGain_t ADC_GAIN_OPTION[6] = {GAIN_TWOTHIRDS, GAIN_ONE, GAIN_TWO, GAIN_FOUR, GAIN_EIGHT, GAIN_SIXTEEN};

// GAIN_TWOTHIRDS 	±6.144 V
// GAIN_ONE 		±4.096 V
// GAIN_TWO 		±2.048 V
// GAIN_FOUR 		±1.024 V
// GAIN_EIGHT 		±0.512 V
// GAIN_SIXTEEN 	±0.256 V

float MAX_MOTOR_SPEED = 31000; 		// 32 - 6400
// float MAX_MOTOR_SPEED = 14500; 	// 16 - 3200
// float MAX_MOTOR_SPEED = 7500; 	// 8 - 1600

// float MAX_MOTOR_SPEED = 4500; 	// 4 - 800
// float MAX_MOTOR_SPEED = 1850; 	// 2 - 400

// float MAX_MOTOR_SPEED[5] = {1850, 4500, 7500, 14500, 31000};

enum SystemState {
	IDLE,
	RUNNING,
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

		bool start_was_pressed = digitalRead(LS_START_PIN);
		bool end_was_pressed = digitalRead(LS_END_PIN);

		while (true) {
			stepper.setSpeed(-MAX_MOTOR_SPEED);
			stepper.moveTo(-100000);
			stepper.run();

			bool start_pressed = digitalRead(LS_START_PIN);
			bool end_pressed = digitalRead(LS_END_PIN);

			bool new_press = (!start_was_pressed && start_pressed) || (!end_was_pressed && end_pressed);

			if (new_press) break;

			start_was_pressed = start_pressed;
			end_was_pressed = end_pressed;
		}

		stepper.stop();
	}

	stepper.setCurrentPosition(0);
}

void go_to_end(AccelStepper& stepper) {
	if (!NO_SETUP_DEBUG) {

		bool start_was_pressed = digitalRead(LS_START_PIN);
		bool end_was_pressed = digitalRead(LS_END_PIN);

		while (true) {
			stepper.setSpeed(MAX_MOTOR_SPEED);
			stepper.moveTo(100000);
			stepper.run();

			bool start_pressed = digitalRead(LS_START_PIN);
			bool end_pressed = digitalRead(LS_END_PIN);

			bool new_press = (!start_was_pressed && start_pressed) || (!end_was_pressed && end_pressed);

			if (new_press) break;

			start_was_pressed = start_pressed;
			end_was_pressed = end_pressed;
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