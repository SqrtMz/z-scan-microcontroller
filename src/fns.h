#include <Arduino.h>

bool DEBUG = true;		// Changes the behaviour whether using the setup or just software debugging is needed

int ENA_PIN = 25;		// Enable pin
int DIR_PIN = 26;		// Direction pin
int PUL_PIN = 27;		// Pulse pin

int LS_START_PIN = 32;	// Limit switch start pin
int LS_END_PIN = 35;	// Limit switch end pin
int PD_PIN = 33;		// Photodiode pin

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