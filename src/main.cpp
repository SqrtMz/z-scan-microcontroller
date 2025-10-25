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

float max_motor_speed = 25600;

char incoming_data[100];
String commands[10];

Adafruit_ADS1115 ads;
AccelStepper stepper(AccelStepper::DRIVER, pul_pin, dir_pin);

bool is_moving = false;

float move_from, move_to, motor_speed, measure_separation, stabilization_time;

void print_data() {

	Serial.print(photo_diode_value);
	Serial.print(',');
	Serial.println(stepper.currentPosition());

}

void go_to_start() {

	while (!digitalRead(lim_switch_start_pin)) {
		stepper.setSpeed(-max_motor_speed);
		stepper.move(-1);
		stepper.run();
	}

	stepper.stop();
	stepper.setCurrentPosition(0);
	stepper.setSpeed(motor_speed);
}

void go_to_end() {

	while (!digitalRead(lim_switch_end_pin)) {
		stepper.setSpeed(max_motor_speed);
		stepper.run();
	}

	stepper.stop();
}

void stop() {
	stepper.stop();
}

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

	ads.begin();
	ads.setGain(GAIN_EIGHT);

	// go_to_start();
}

void loop() {

	if (Serial.available()) {
		read_incoming_data(incoming_data, commands);

		for (size_t i = 0; i < 10; i++)
		{
			Serial.print(commands[i]);
			Serial.print(',');
		}

		Serial.println();
	}

	if (commands[0] == "execute") {
		if (stepper.currentPosition() != 0) { go_to_start(); }
		is_moving = true;
	}

	else if (commands[0] == "stop") {
		stepper.stop();
		is_moving = false;
		
		Serial.println("Stopped");
	}

	else if (commands[0] == "go_to_start") {go_to_start();}
	else if (commands[0] == "go_to_end") {go_to_end();}
	else if (commands[0] == "stop") {stop();}

	if (is_moving) {
		if (commands[0] == "execute") {
			move_from = commands[1].toFloat();
			move_to = commands[2].toFloat();
			motor_speed = ((commands[3].toFloat()) * 0.01) * max_motor_speed;
			measure_separation = commands[4].toFloat();
			stabilization_time = commands[5].toFloat();
		}
		
		stepper.moveTo(move_from);
		stepper.setSpeed(motor_speed);
		stepper.run();

		if (stepper.currentPosition() == move_from) {
			delay(stabilization_time);
			move_from = stepper.currentPosition() + measure_separation;
			
			// photo_diode_value = ads.readADC_Differential_0_1();
			// photo_diode_value = ads.computeVolts(photo_diode_value);
			photo_diode_value = analogRead(photo_diode_pin);
			print_data();
		}

		if (stepper.currentPosition() >= move_to || digitalRead(lim_switch_start_pin) || digitalRead(lim_switch_end_pin)) {
			stepper.stop();
			delay(stabilization_time);
			is_moving = false;
		}
	}

	memset(incoming_data, '\0', sizeof(incoming_data));
	memset(commands, '\0', sizeof(commands));
}