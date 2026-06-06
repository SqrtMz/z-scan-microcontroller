#include <Arduino.h>
#include <AccelStepper.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include "fns.h"

float pd_value = 0.0; // Photodiode value
float pd2_value = 0.0; // Photodiode value

float MAX_MOTOR_SPEED = 31000; // 32 - 6400
// float MAX_MOTOR_SPEED = 14500; // 16 - 3200
// float MAX_MOTOR_SPEED = 7500; // 8 - 1600

// float MAX_MOTOR_SPEED = 4500; // 4 - 800
// float MAX_MOTOR_SPEED = 1850; // 2 - 400

// float MAX_MOTOR_SPEED[5] = {1850, 4500, 7500, 14500, 31000};

char incoming_data[100];
String commands[10];

Adafruit_ADS1115 adc;
AccelStepper stepper(AccelStepper::DRIVER, PUL_PIN, DIR_PIN);

bool is_moving, is_accelerated;
float move_from, move_to, measure_separation, motor_speed, stabilization_time;
adsGain_t adc_gain;

void setup() {

	Serial.begin(115200);
	pinMode(PD_PIN, INPUT);

	pinMode(DIR_PIN, OUTPUT);
	pinMode(PUL_PIN, OUTPUT);

	pinMode(ENA_PIN, OUTPUT);
	digitalWrite(ENA_PIN, LOW); // LOW unlock - HIGH lock

	pinMode(LS_START_PIN, INPUT_PULLDOWN);
	pinMode(LS_END_PIN, INPUT_PULLDOWN);

	stepper.setMaxSpeed(MAX_MOTOR_SPEED);
	stepper.setAcceleration(MAX_MOTOR_SPEED);
	stepper.setSpeed(motor_speed);

	// GAIN_TWOTHIRDS 	±6.144 V
	// GAIN_ONE 		±4.096 V
	// GAIN_TWO 		±2.048 V
	// GAIN_FOUR 		±1.024 V
	// GAIN_EIGHT 		±0.512 V
	// GAIN_SIXTEEN 	±0.256 V

	if (!adc.begin()) {Serial.println("ADC couldn't be initialized");}
	// adc.setGain(GAIN_TWOTHIRDS);

	if (!DEBUG) go_to_start(LS_START_PIN, MAX_MOTOR_SPEED, stepper);
}

void loop() {

	if (Serial.available()) {read_incoming_data(incoming_data, commands);}

	if (commands[0] == "execute") {
		if (stepper.currentPosition() != 0) { go_to_start(LS_START_PIN, MAX_MOTOR_SPEED, stepper); delay(1000);}
		is_moving = true;

		move_from = commands[1].toFloat();											// Receives start position in steps
		move_to = commands[2].toFloat();											// Receives final position in steps
		motor_speed = commands[3].toFloat() * MAX_MOTOR_SPEED * 0.01 * 0.15;		// Receives an int[1, 100]
		measure_separation = commands[4].toFloat();									// Receives the separation where measures will be taken in steps
		stabilization_time = commands[5].toFloat();									// Receives an int
		is_accelerated = (bool)commands[6].toInt();									// Receives an int[0, 1]
		adc_gain = ADC_GAIN_OPTION[commands[7].toInt()];							// Receives an index for ADC_GAIN_OPTIONS[]

		// adc.setGain(adc_gain);
	}

	else if (commands[0] == "stop") {
		stepper.stop();
		is_moving = false;

		Serial.println("Stopped");
	}

	else if (commands[0] == "go_to_start") {go_to_start(LS_START_PIN, MAX_MOTOR_SPEED, stepper);}
	else if (commands[0] == "go_to_end") {go_to_end(LS_END_PIN, MAX_MOTOR_SPEED, stepper);}
	else if (commands[0] == "stop") {stepper.stop();}

	if (is_moving) {

		stepper.moveTo(move_from);
		if (!is_accelerated && stepper.currentPosition() != 0) {stepper.setSpeed(motor_speed);}
		stepper.run();

		if (stepper.currentPosition() == move_from) {
			
			if (stabilization_time != 0) {delay(stabilization_time);}

			// pd_value = adc.readADC_Differential_1_3();
			// pd2_value = adc.readADC_Differential_2_3();

			pd_value = 10000;
			pd2_value = analogRead(25);

			// float pd_value = 0.0;
			// for (size_t i = 0; i < 10; i++) {pd_value += adc.readADC_Differential_1_3();}
			// pd_value /= 10;

			print_data(pd_value, pd2_value, stepper);

			// Serial.print("DIFF 1 - 3: ");
			// Serial.println(adc.computeVolts(pd_value));
			
			// Serial.print("DIFF 2 - 3: ");
			// Serial.println(adc.computeVolts(pd2_value));
			
			// Serial.print("A1: ");
			// Serial.println(adc.computeVolts(adc.readADC_SingleEnded(1)));
			
			
			// Serial.print("A2: ");
			// Serial.println(adc.computeVolts(adc.readADC_SingleEnded(1)));
			
			// Serial.print("A3: ");
			// Serial.println(adc.computeVolts(adc.readADC_SingleEnded(3)));
			
			// Serial.println(adc_gain);
			// Serial.println("===========================");

			move_from = stepper.currentPosition() + measure_separation;
		}
	}

	if (stepper.currentPosition() >= move_to || digitalRead(LS_START_PIN) || digitalRead(LS_END_PIN)) {
		stepper.stop();
		delay(stabilization_time);
		is_moving = false;
	}
	
	memset(incoming_data, '\0', sizeof(incoming_data));
	memset(commands, '\0', sizeof(commands));
}