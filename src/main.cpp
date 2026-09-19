#include <Arduino.h>
#include <AccelStepper.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>

#include "fns.h"

float pd_value = 0.0; // Photodiode value
float pd2_value = 0.0; // Photodiode value

char incoming_data[100];
String commands[10];

Adafruit_ADS1115 adc;
AccelStepper stepper(AccelStepper::DRIVER, PULP_PIN, DIRP_PIN);

bool is_accelerated;
int move_from, move_to, measure_separation, stabilization_time;
float motor_speed;
adsGain_t adc_gain;

int average_items = 1;						// Number of samples for average measurement of photodiodes

SystemState system_state = IDLE;

bool being_pressed = false;

void setup() {

	Serial.begin(115200);

	pinMode(DIRP_PIN, OUTPUT);
	pinMode(PULP_PIN, OUTPUT);

	pinMode(ENAP_PIN, OUTPUT);
	digitalWrite(ENAP_PIN, LOW); // LOW unlock - HIGH lock

	pinMode(DIRN_PIN, OUTPUT);
	pinMode(PULN_PIN, OUTPUT);
	pinMode(ENAN_PIN, OUTPUT);
	digitalWrite(DIRN_PIN, LOW);
	digitalWrite(PULN_PIN, LOW);
	digitalWrite(ENAN_PIN, LOW);

	pinMode(LS_START_PIN, INPUT_PULLDOWN);
	pinMode(LS_END_PIN, INPUT_PULLDOWN);

	stepper.setMaxSpeed(max_motor_speed);
	stepper.setAcceleration(max_motor_speed / 3);

	if (!adc.begin()) {Serial.println("ADC couldn't be initialized");}
	adc.setGain(GAIN_TWOTHIRDS);

	go_to_start(stepper);
}

void loop() {

	if (Serial.available()) { read_incoming_data(incoming_data, commands); }

	if (commands[0] == "execute") {
		if (stepper.currentPosition() != 0) go_to_start(stepper);
		system_state = RUNNING;

		max_motor_speed = commands[9].toInt();										// Receives an int for the new motor_speed
		move_from = commands[1].toInt();											// Receives start position in steps
		move_to = commands[2].toInt();												// Receives final position in steps
		motor_speed = commands[3].toFloat() * 0.01 * 0.15 * max_motor_speed;		// Receives an int[1, 100]
		measure_separation = commands[4].toInt();									// Receives the separation where measures will be taken in steps
		stabilization_time = commands[5].toInt();									// Receives an int
		is_accelerated = (bool)commands[6].toInt();									// Receives an int[0, 1]
		adc_gain = ADC_GAIN_OPTION[commands[7].toInt()];							// Receives an index for ADC_GAIN_OPTIONS[]
		average_items = commands[8].toInt();										// Receives an int for the iterations on per measure

		adc.setGain(adc_gain);
		delay(1000);
	}

	else if (commands[0] == "stop") {
		stepper.stop();
		system_state = IDLE;

		Serial.println("STOPPED");
	}

	else if (commands[0] == "go_to_start") {
		max_motor_speed = commands[1].toInt();
		stepper.setMaxSpeed(max_motor_speed);
		stepper.setAcceleration(max_motor_speed / 4);
		go_to_start(stepper);
	}

	else if (commands[0] == "go_to_end") {
		max_motor_speed = commands[1].toInt();
		stepper.setMaxSpeed(max_motor_speed);
		stepper.setAcceleration(max_motor_speed / 4);
		go_to_end(stepper);
	}

	switch(system_state) {
		case RUNNING:
			if (!is_accelerated) stepper.setSpeed(motor_speed);
			stepper.moveTo(move_from);
			stepper.runSpeedToPosition();

			if (stepper.currentPosition() == move_from) {
				
				if (stabilization_time != 0) delay(stabilization_time);

				pd_value = 0.0;
				pd2_value = 0.0;

				for (size_t i = 0; i < average_items; i++) {
					pd_value += adc.readADC_Differential_1_3();
					pd2_value += adc.readADC_Differential_2_3();
				}

				pd_value /= average_items;
				pd2_value /= average_items;

				print_data(pd_value, pd2_value, stepper);

				move_from = stepper.currentPosition() + measure_separation;
			}

			if (stepper.currentPosition() >= move_to || digitalRead(LS_END_PIN)) {
				stepper.stop();
				delay(stabilization_time);
				system_state = IDLE;
			}

			break;

		case IDLE:
		default:
			if (ADC_DEBUG) {
				print_adc_debug(adc);
				for (String& c : commands) {
						Serial.print(c);
						Serial.print(",");
				}
				Serial.println("");
				delay(100);
			}
			Serial.println("IDLE");
			delay(100);
			break;
	}
	
	memset(incoming_data, '\0', sizeof(incoming_data));
	memset(commands, '\0', sizeof(commands));
}