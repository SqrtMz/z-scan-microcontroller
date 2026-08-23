// #include <Arduino.h>
// #include <AccelStepper.h>
// #include <Adafruit_ADS1X15.h>
// #include <Wire.h>
// #include "fns.h"

// float pd_value = 0.0;
// float pd2_value = 0.0;
// float MAX_MOTOR_SPEED = 31000;

// char incoming_data[100];
// String commands[10];

// Adafruit_ADS1115 adc;
// AccelStepper stepper(AccelStepper::DRIVER, PUL_PIN, DIR_PIN);

// // State Machine Definition
// enum MotionState { IDLE, RUNNING, STABILIZING };
// MotionState currentState = IDLE;

// bool is_accelerated;
// float move_from, move_to, measure_separation, motor_speed, stabilization_time;
// adsGain_t adc_gain;
// unsigned long stabilizationStartTimer = 0;

// void setup() {
// 	Serial.begin(115200);
// 	pinMode(DIR_PIN, OUTPUT);
// 	pinMode(PUL_PIN, OUTPUT);
// 	pinMode(ENA_PIN, OUTPUT);
// 	digitalWrite(ENA_PIN, LOW); 

// 	pinMode(LS_START_PIN, INPUT_PULLDOWN);
// 	pinMode(LS_END_PIN, INPUT_PULLDOWN);

// 	stepper.setMaxSpeed(MAX_MOTOR_SPEED);
// 	stepper.setAcceleration(MAX_MOTOR_SPEED);

// 	if (!adc.begin()) { Serial.println("ADC couldn't be initialized"); }
// 	adc.setGain(GAIN_TWOTHIRDS);
// }

// void loop() {

// 	if (Serial.available()) {
// 		read_incoming_data(incoming_data, commands);
		
// 		if (commands[0] == "execute") {
// 			if (stepper.currentPosition() != 0) { 
// 				go_to_start(LS_START_PIN, MAX_MOTOR_SPEED, stepper); 
// 			}
			
// 			move_from = commands[1].toFloat();
// 			move_to = commands[2].toFloat();
// 			motor_speed = commands[3].toFloat() * MAX_MOTOR_SPEED * 0.01 * 0.15;
// 			measure_separation = commands[4].toFloat();
// 			stabilization_time = commands[5].toFloat();
// 			is_accelerated = (bool)commands[6].toInt();
// 			adc_gain = ADC_GAIN_OPTION[commands[7].toInt()];

// 			adc.setGain(adc_gain);
			
// 			stepper.moveTo(move_from);
// 			if (!is_accelerated) { stepper.setSpeed(motor_speed); }
// 			currentState = RUNNING;
// 		}
// 		else if (commands[0] == "stop") {
// 			stepper.stop();
// 			currentState = IDLE;
// 			Serial.println("Stopped");
// 		}
// 		else if (commands[0] == "go_to_start") {
// 			go_to_start(LS_START_PIN, MAX_MOTOR_SPEED, stepper);
// 		}
// 		else if (commands[0] == "go_to_end") {
// 			go_to_end(LS_END_PIN, MAX_MOTOR_SPEED, stepper);
// 		}

// 		memset(incoming_data, '\0', sizeof(incoming_data));
// 		memset(commands, '\0', sizeof(commands));
// 	}

// 	if (digitalRead(LS_START_PIN) || digitalRead(LS_END_PIN) || stepper.currentPosition() >= move_to) {
// 		if (currentState != IDLE) {
// 			stepper.stop();
// 			currentState = IDLE;
// 		}
// 	}

// 	switch (currentState) {
// 		case RUNNING:
// 			if (!is_accelerated) {
// 				stepper.runSpeedToPosition(); // Best for constant non-accelerated speeds
// 			} else {
// 				stepper.run();
// 			}

// 			// Arrived at measurement destination
// 			if (stepper.currentPosition() == move_from) {
// 				if (stabilization_time > 0) {
// 					stabilizationStartTimer = millis();
// 					currentState = STABILIZING;
// 				} else {
// 					// Read immediately if stabilization time is 0
// 					pd_value = adc.readADC_Differential_1_3();
// 					pd2_value = adc.readADC_Differential_2_3();
// 					print_data(pd_value, pd2_value, stepper);
					
// 					move_from += measure_separation;
// 					stepper.moveTo(move_from);
// 				}
// 			}
// 			break;

// 		case STABILIZING:
// 			// Non-blocking timer check: keep stepping/updating while waiting
// 			if (millis() - stabilizationStartTimer >= (unsigned long)stabilization_time) {
// 				pd_value = adc.readADC_Differential_1_3();
// 				pd2_value = adc.readADC_Differential_2_3();
// 				print_data(pd_value, pd2_value, stepper);
				
// 				move_from += measure_separation;
// 				stepper.moveTo(move_from);
// 				if (!is_accelerated) { stepper.setSpeed(motor_speed); }
// 				currentState = RUNNING;
// 			}
// 			break;

// 		case IDLE:
// 		default:
// 			break;
// 	}
// }