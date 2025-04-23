/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "stm32l4xx_hal.h"
#include "app.h"

/* Private define ------------------------------------------------------------*/
#define		LED_PORT			GPIOA
#define		LED_PIN				GPIO_PIN_5

#define 	PIR_PORT 			GPIOA
#define 	PIR_PIN 			GPIO_PIN_8

#define     BUTTON_PORT			GPIOC
#define		BUTTON_PIN			GPIO_PIN_13

/* Private function prototypes -----------------------------------------------*/
void ShowCommands(void);
void UART_TransmitString(UART_HandleTypeDef *p_huart, char a_string[], int newline);
void Servo_Angle(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t angle);


/* Extern global variables ---------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;


//Should be declared as volatile if variables' values are changed in ISR.
volatile int motion;
volatile int mode = 0;
uint32_t last_dispense_time = 0;
volatile char rxData;


void App_Init(void) {

	UART_TransmitString(&huart2, "-----------------", 1);
	UART_TransmitString(&huart2, "~ Final Project ~", 1);
	UART_TransmitString(&huart2, "-----------------", 1);

	ShowCommands();

	HAL_UART_Receive_IT(&huart2, (uint8_t*) &rxData, 1); //Start the Rx interrupt.

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

	last_dispense_time = HAL_GetTick();
}

void Servo_Angle(TIM_HandleTypeDef *htim, uint32_t channel, uint8_t angle){
	uint32_t pulse_length;
	if(angle > 180) angle = 180;
	pulse_length = 400 + (angle * (800-400) / 180);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse_length);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == BUTTON_PIN) {  // B1 button
        mode = !mode; // Toggle mode
        if (mode == 0) {
            UART_TransmitString(&huart2, "Switched to Home Mode", 1);
        } else {
            UART_TransmitString(&huart2, "Switched to Away Mode", 1);
            last_dispense_time = HAL_GetTick(); // Reset timer when switching to Away mode
        }
    }
}

void App_MainLoop(void) {

	if(mode == 0){
		motion = HAL_GPIO_ReadPin(PIR_PORT, PIR_PIN);
		if(motion == 1){
			HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
			UART_TransmitString(&huart2, "Food Dispensing", 1);
			Servo_Angle(&htim2, TIM_CHANNEL_2, 165);
			HAL_Delay(2500); //2.5 seconds to dispense
			Servo_Angle(&htim2, TIM_CHANNEL_2, 0);
			UART_TransmitString(&huart2, "Ready to dispense", 1);
			HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
			HAL_Delay(1000); //cool down time
		}
	}
	else {
		uint32_t current_time = HAL_GetTick();
		if((current_time - last_dispense_time) >= 600000){
			UART_TransmitString(&huart2, "Dispensing food (Away Mode)", 1);

			Servo_Angle(&htim2, TIM_CHANNEL_2,165);
			HAL_Delay(2500);

			Servo_Angle(&htim2, TIM_CHANNEL_2,0);
			UART_TransmitString(&huart2, "Ready to dispense", 1);
			HAL_Delay(1000);

			last_dispense_time = HAL_GetTick();
		}
	}
}



void ShowCommands(void) {
	UART_TransmitString(&huart2, "Welcome, Wave your hand to dispense rockets food!", 1);
	UART_TransmitString(&huart2, "Press B1 to switch between Home and Away mode", 1);
}

void UART_TransmitString(UART_HandleTypeDef *p_huart, char a_string[], int newline) {
	HAL_UART_Transmit(p_huart, (uint8_t*) a_string, strlen(a_string), HAL_MAX_DELAY);
	if (newline != 0) {
		HAL_UART_Transmit(p_huart, (uint8_t*) "\n\r", 2, HAL_MAX_DELAY);
	}
}

