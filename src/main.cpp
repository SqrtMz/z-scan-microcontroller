#include <Arduino.h>
#include <AccelStepper.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include "fns.h"

int ena_pin = 25;
int dir_pin = 26;
int pul_pin = 27;

int lim_switch_start_pin = 32;
int lim_switch_end_pin = 35;

int photo_diode_pin = 33;
float photo_diode_value = 0.0;

float max_motor_speed = 31000; // 32 - 6400
// float max_motor_speed = 14500; // 16 - 3200
// float max_motor_speed = 7500; // 8 - 1600

// float max_motor_speed = 4500; // 4 - 800
// float max_motor_speed = 1850; // 2 - 400

char incoming_data[100];
String commands[10];

Adafruit_ADS1115 ads;
AccelStepper stepper(AccelStepper::DRIVER, pul_pin, dir_pin);

bool is_moving, is_accelerated;
float move_from, move_to, measure_separation, motor_speed, stabilization_time;

void setup() {

	Serial.begin(115200);
	pinMode(photo_diode_pin, INPUT);

	pinMode(dir_pin, OUTPUT);
	pinMode(pul_pin, OUTPUT);

	pinMode(ena_pin, OUTPUT);
	digitalWrite(ena_pin, LOW); // LOW unlock - HIGH lock

	pinMode(lim_switch_start_pin, INPUT_PULLDOWN);
	pinMode(lim_switch_end_pin, INPUT_PULLDOWN);

	stepper.setMaxSpeed(max_motor_speed);
	stepper.setAcceleration(max_motor_speed);
	stepper.setSpeed(motor_speed);

	ads.begin();
	ads.setGain(GAIN_EIGHT);

	// go_to_start();
}

void loop() {

	if (Serial.available()) {read_incoming_data(incoming_data, commands);}

	if (commands[0] == "execute") {
		if (stepper.currentPosition() != 0) { go_to_start(lim_switch_start_pin, max_motor_speed, stepper); delay(1000);}
		is_moving = true;

		move_from = commands[1].toFloat();
		move_to = commands[2].toFloat();
		motor_speed = commands[3].toFloat() * max_motor_speed * 0.01 * 0.15;
		measure_separation = commands[4].toFloat();
		stabilization_time = commands[5].toFloat();
		is_accelerated = (bool)commands[6].toInt();
	}

	else if (commands[0] == "stop") {
		stepper.stop();
		is_moving = false;
		
		Serial.println("Stopped");
	}

	else if (commands[0] == "go_to_start") {go_to_start(lim_switch_start_pin, max_motor_speed, stepper);}
	else if (commands[0] == "go_to_end") {go_to_end(lim_switch_end_pin, max_motor_speed, stepper);}
	else if (commands[0] == "stop") {stepper.stop();}

	if (is_moving) {

		stepper.moveTo(move_from);
		if (!is_accelerated && stepper.currentPosition() != 0) {stepper.setSpeed(motor_speed);}
		stepper.run();

		if (stepper.currentPosition() == move_from) {
			
			if (stabilization_time != 0) {delay(stabilization_time);}
			move_from = stepper.currentPosition() + measure_separation;
			
			// photo_diode_value = ads.readADC_Differential_0_1();
			// photo_diode_value = ads.computeVolts(photo_diode_value);

			float photo_diode_value = 0.0;
			for (size_t i = 0; i < 10; i++)
			{
				photo_diode_value += analogRead(photo_diode_pin);
			}
			photo_diode_value /= 10;

			print_data(photo_diode_value, stepper);
		}
	}

	if (stepper.currentPosition() >= move_to || digitalRead(lim_switch_start_pin) || digitalRead(lim_switch_end_pin)) {
		stepper.stop();
		delay(stabilization_time);
		is_moving = false;
	}
	
	memset(incoming_data, '\0', sizeof(incoming_data));
	memset(commands, '\0', sizeof(commands));
}