/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "stm32l4xx_hal.h"
#include "app.h"

/* Private define ------------------------------------------------------------*/
#define 	PIR_PORT 			GPIOA
#define 	PIR_PIN 			GPIO_PIN_8

/* Private function prototypes -----------------------------------------------*/
void ShowCommands(void);
void UART_TransmitString(UART_HandleTypeDef *p_huart, char a_string[], int newline);
void Servo_Angle(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t angle);


/* Extern global variables ---------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;

extern UART_HandleTypeDef huart2;


//Should be declared as volatile if variables' values are changed in ISR.
volatile int inmotion;
volatile char rxData;

void App_Init(void) {

	UART_TransmitString(&huart2, "-----------------", 1);
	UART_TransmitString(&huart2, "~ Final Project ~", 1);
	UART_TransmitString(&huart2, "-----------------", 1);

	ShowCommands();

	HAL_UART_Receive_IT(&huart2, (uint8_t*) &rxData, 1); //Start the Rx interrupt.

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

}

void Servo_Angle(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t angle){
	uint32_t pulse_length;
	if(angle > 180) angle = 180;
	pulse_length = 400 + (angle * (800-400) / 180);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse_length);
}


void App_MainLoop(void) {


	static uint8_t stop_detection = 0;
	static uint8_t prev_State = 0;

	motion = HAL_GPIO_ReadPin(PIR_PORT, PIR_PIN);

	if(motion == 1 && stop_detection == 0){

		stop_detection = 1; //this prevents the sensor from re-triggering when a hand is held over the sensor AKA even if the hand stays there food only dispenses once

		UART_TransmitString(&huart2, "Food Dispensing", 1);

		Servo_Angle(&htim2, TIM_CHANNEL_2, 90);
		HAL_Delay(5000); //5 seconds to dispense

		Servo_Angle(&htim2, TIM_CHANNEL_2, 0);
		HAL_Delay(2000); //cool down time
	}

	else if(motion == 0 && stop_detection == 1){
		stop_detection = 0; //this resets trigger when there is no motion
	}

	if(motion == 1 && prev_State == 1){ //stops ready to dispense from printing infinitely
		 UART_TransmitString(&huart2, "Ready to dispense", 1);
	}
	prev_State = motion;
}



void ShowCommands(void) {
	UART_TransmitString(&huart2, "Welcome, Wave your hand to dispense rockets food!", 1);
}

void UART_TransmitString(UART_HandleTypeDef *p_huart, char a_string[], int newline) {
	HAL_UART_Transmit(p_huart, (uint8_t*) a_string, strlen(a_string), HAL_MAX_DELAY);
	if (newline != 0) {
		HAL_UART_Transmit(p_huart, (uint8_t*) "\n\r", 2, HAL_MAX_DELAY);
	}
}

