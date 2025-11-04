#include <Arduino.h>

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

void print_data(float photo_diode_value, AccelStepper &stepper) {
	Serial.print(photo_diode_value);
	Serial.print(',');
	Serial.println(stepper.currentPosition());
}

void go_to_start(int lim_switch_start_pin, int max_motor_speed, AccelStepper &stepper) {

	while (!digitalRead(lim_switch_start_pin)) {
		stepper.move(-100000);
		stepper.setSpeed(-max_motor_speed);
		stepper.run();
	}

	stepper.stop();
	stepper.setCurrentPosition(0);
}

void go_to_end(int lim_switch_end_pin, int max_motor_speed, AccelStepper &stepper) {

	while (!digitalRead(lim_switch_end_pin)) {
		stepper.move(100000);
		stepper.setSpeed(max_motor_speed);
		stepper.run();
	}

	stepper.stop();
}