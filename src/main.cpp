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
AccelStepper stepper(AccelStepper::DRIVER, PUL_PIN, DIR_PIN);

SystemState system_state = IDLE;
bool is_accelerated = false;
bool switch_pressed = false;
float move_from, move_to, measure_separation, motor_speed, stabilization_time;
adsGain_t adc_gain;

void setup() {

	Serial.begin(115200);

	pinMode(DIR_PIN, OUTPUT);
	pinMode(PUL_PIN, OUTPUT);

	pinMode(ENA_PIN, OUTPUT);
	digitalWrite(ENA_PIN, LOW); // LOW unlock - HIGH lock

	pinMode(LS_START_PIN, INPUT_PULLDOWN);
	pinMode(LS_END_PIN, INPUT_PULLDOWN);

	stepper.setMaxSpeed(MAX_MOTOR_SPEED);
	stepper.setAcceleration(MAX_MOTOR_SPEED);
	stepper.setSpeed(motor_speed);

	if (!adc.begin()) {Serial.println("ADC couldn't be initialized");}
	adc.setGain(GAIN_TWOTHIRDS);

	go_to_start(stepper);
}

void loop() {

	if (Serial.available()) read_incoming_data(incoming_data, commands);

	if (commands[0] == "execute") {
		if (stepper.currentPosition() != 0) {
			go_to_start(stepper);
			delay(100);
		}

		system_state = RUNNING;

		move_from = commands[1].toFloat();											// Receives start position in steps
		move_to = commands[2].toFloat();											// Receives final position in steps
		motor_speed = commands[3].toFloat() * MAX_MOTOR_SPEED * 0.01 * 0.15;		// Receives an int[1, 100]
		measure_separation = commands[4].toFloat();									// Receives the separation where measures will be taken in steps
		stabilization_time = commands[5].toFloat();									// Receives an int
		is_accelerated = (bool)commands[6].toInt();									// Receives an int[0, 1]
		adc_gain = ADC_GAIN_OPTION[commands[7].toInt()];							// Receives an index for ADC_GAIN_OPTIONS[]

		adc.setGain(adc_gain);
	}

	else if (commands[0] == "stop") {
		stepper.stop();
		system_state = IDLE;

		Serial.println("STOPPED");
	}

	else if (commands[0] == "go_to_start") go_to_start(stepper);
	else if (commands[0] == "go_to_end") go_to_end(stepper);

	switch (system_state) {
		case RUNNING:
			stepper.moveTo(move_from);
			if (!is_accelerated && stepper.currentPosition() != 0) stepper.setSpeed(motor_speed);
			stepper.run();

			if (stepper.currentPosition() == move_from) {
				
				if (stabilization_time != 0) delay(stabilization_time);

				if (ADC_DEBUG) print_adc_debug(adc);
				else {

					pd_value = 0.0;
					pd2_value = 0.0;

					for (size_t i = 0; i < AVERAGE_ITEMS; i++) {
						pd_value += adc.readADC_Differential_1_3();
						pd2_value += adc.readADC_Differential_2_3();
					}

					pd_value /= AVERAGE_ITEMS;
					pd2_value /= AVERAGE_ITEMS;

					print_data(pd_value, pd2_value, stepper);

				}

				move_from = stepper.currentPosition() + measure_separation;
			}
			
			if (stepper.currentPosition() >= move_to) {
				stepper.stop();
				system_state = IDLE;
			}

			if (!switch_pressed)
				if (digitalRead(LS_START_PIN) || digitalRead(LS_END_PIN)) switch_pressed = true;
			else {
				stepper.stop();
				system_state = IDLE;
				switch_pressed = false;
			}
			
			break;

		case IDLE:
		default:
			break;
	}
	
	memset(incoming_data, '\0', sizeof(incoming_data));
	memset(commands, '\0', sizeof(commands));
}