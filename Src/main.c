/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "fonts.h"

uint8_t rxByte[1];
char inputStr[18];
uint8_t inputStrSize = 0;
char modes[20][8] = { "gnrnyn", "gn", "gry", "ynynyn" };
uint8_t seqLengths[20] = { 6, 2, 3, 6 };
uint8_t freqMultiplier[20] = { 1, 1, 1, 1 };
uint8_t modesTotal = 4;
uint8_t currMode = 0;
uint8_t isFreqInput = 0;
uint8_t isInterruptOn = 0;
uint8_t btn = 1;
uint8_t timer = 0;
char currLightState = 'n';

uint8_t TRANSMIT_DELAY = 10;
uint8_t RECEIVE_DELAY = 100;
uint8_t byteReceiveStatus = -1;

uint8_t prev = 112;
uint8_t data = 1;
uint8_t default_data = 112;

typedef struct {
	char current[16][32];
	char buffer[16][32];
} matrix;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
void transmit(char buf[], uint8_t buffSize) {
	char bufWithEndOfLine[buffSize + 1];
	for (int i = 0; i < buffSize; i++) {
		bufWithEndOfLine[i] = buf[i];
	}
	bufWithEndOfLine[buffSize] = 13; // enter
	if (isInterruptOn == 1) {
		HAL_UART_Transmit_IT(&huart6, bufWithEndOfLine, buffSize + 1);
	} else {
		HAL_UART_Transmit(&huart6, bufWithEndOfLine, buffSize + 1,
				TRANSMIT_DELAY);
	}
}

void errorOutput(uint8_t num) {
	char str[] = "[ 0]Wrong command";
	if (num < 10) {
		str[2] = '0' + num;
	} else {
		str[2] = '0' + num / 10;
		str[1] = '0' + num % 10;
	}

	transmit(str, 17);
}

void lightsLogic() {
	int newBtn = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
	if (btn == 0 && newBtn == 1) {
		currMode = (currMode + 1) % modesTotal;
	}
	btn = newBtn;

	int fase = (timer / freqMultiplier[currMode]) % seqLengths[currMode];
	if (timer % freqMultiplier[currMode] == 0) {
		// change lights state
		switch (currLightState) {
		case 'r':
			switch (modes[currMode][fase]) {
			case 'g':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
				break;
			case 'r':
				break;
			case 'y':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				break;
			case 'n':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
				break;
			}
			break;
		case 'g':
			switch (modes[currMode][fase]) {
			case 'g':
				break;
			case 'r':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
				break;
			case 'y':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				break;
			case 'n':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
				break;
			}
			break;
		case 'y':
			switch (modes[currMode][fase]) {
			case 'g':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
				break;
			case 'r':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
				break;
			case 'y':
				break;
			case 'n':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				break;
			}
			break;
		case 'n':
			switch (modes[currMode][fase]) {
			case 'g':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
				break;
			case 'r':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
				break;
			case 'y':
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				break;
			case 'n':
				break;
			}
			break;
		}
		currLightState = modes[currMode][fase];
	}
	timer += 1;
}

void proceedInput() {
	if (rxByte[0] != 0x0D) {
		if (inputStrSize < 18) {
			if (isInterruptOn == 1
					|| (isInterruptOn == 0 && byteReceiveStatus == HAL_OK)) {
				inputStr[inputStrSize] = rxByte[0];
				inputStrSize++;
				transmit("                 ", 18);
				transmit(inputStr, inputStrSize);
			}
		}
	} else {
		char strSet[] = "set ";
		char strNew[] = "new ";
		char strSetInterruptsOn[] = "set interrupts on";
		char strSetInterruptsOff[] = "set interrupts off";

		// set
		if (strncmp(inputStr, strSet, 4) == 0 && inputStrSize == 5) {
			uint8_t mode = inputStr[4] - '0';
			if (mode < 0 || mode > 9 || mode >= modesTotal) {
				errorOutput(1);
				goto endOfCommandChecking;
			}

			currMode = mode;

			transmit(inputStr, 5);

			// new
		} else if (strncmp(inputStr, strNew, 4) == 0 && inputStrSize > 4) {
			int i = 4;
			while (i < inputStrSize && i < 12) {
				if (inputStr[i] == 'g' || inputStr[i] == 'r'
						|| inputStr[i] == 'y' || inputStr[i] == 'n') {
					modes[modesTotal][i - 4] = inputStr[i];
				} else {
					errorOutput(2);
					goto endOfCommandChecking;
				}
				i++;
			}
			seqLengths[modesTotal] = i - 4;
			isFreqInput = 1;

			transmit("Input frequency multiplier from 1 to 4", 38);

			// input frequency for 'new' command
		} else if (isFreqInput == 1) {
			isFreqInput = 0;
			if (inputStrSize != 1) {
				errorOutput(3);
				goto endOfCommandChecking;
			}
			uint8_t freq = inputStr[0] - '0';
			if (freq < 1 || freq > 4) {
				errorOutput(4);
				goto endOfCommandChecking;
			}
			freqMultiplier[modesTotal] = 5 - freq;
			modesTotal++;

			transmit(inputStr, inputStrSize);

			// set interrupts on
		} else if (strncmp(inputStr, strSetInterruptsOn, 17) == 0) {
			if (inputStrSize != 17) {
				errorOutput(5);
				goto endOfCommandChecking;
			}
			if (isInterruptOn != 0) {
				errorOutput(6);
				goto endOfCommandChecking;
			}
			isInterruptOn = 1;
			transmit("set interrupts on", 17);

			// set interrupts off
		} else if (strncmp(inputStr, strSetInterruptsOff, 18) == 0) {
			if (inputStrSize != 18) {
				errorOutput(7);
				goto endOfCommandChecking;
			}
			if (isInterruptOn != 1) {
				errorOutput(8);
				goto endOfCommandChecking;
			}
			isInterruptOn = 0;
			transmit("set interrupts off", 18);

			// unknown command
		} else {
			transmit(inputStr, inputStrSize);
			errorOutput(9);
		}

		endOfCommandChecking: inputStrSize = 0;
	}

	if (isInterruptOn == 1) {
		HAL_UART_Receive_IT(&huart6, rxByte, 1);
	} else {
		rxByte[0] = '+';
		byteReceiveStatus = -1;
	}
}

//void receive(char buf[], uint8_t buffSize) {
//	if (isInterruptOn == 1) {
//		HAL_UART_Receive_IT(&huart6, buf, buffSize);
//	} else {
//		HAL_UART_Receive(&huart6, buf, buffSize, RECEIVE_DELAY);
//	}
//}

print_menu(int position) {
	char new_game[50];
	char load[50];
	char save[50];
	char settings[50];
	oled_Fill(Black);

	oled_Fill(Black);
	sprintf(new_game, "%cNew Game", position == 1 ? '>' : ' ');
	sprintf(load, "%cLoad", position == 2 ? '>' : ' ');
	sprintf(save, "%cSave", position == 3 ? '>' : ' ');
	sprintf(settings, "%cSettings", position == 4 ? '>' : ' ');
	oled_SetCursor(0, 0);
	oled_WriteString(new_game, Font_7x10, White);
	oled_SetCursor(0, 15);
	oled_WriteString(load, Font_7x10, White);
	oled_SetCursor(0, 30);
	oled_WriteString(save, Font_7x10, White);
	oled_SetCursor(0, 45);
	oled_WriteString(settings, Font_7x10, White);
	oled_UpdateScreen();
}

void write_symbol(uint8_t symbol) {
	HAL_StatusTypeDef write_status;
	do {
		write_status = HAL_UART_Transmit(&huart6, &symbol, 1, 10000);
	} while (write_status != HAL_OK);
}

void write_str(uint8_t* str) {
	long length = strlen(str);
	HAL_StatusTypeDef write_status;
	do {
		write_status = HAL_UART_Transmit(&huart6, str, length, 10000);
	} while (write_status != HAL_OK);
}

void write_new_line() {
	write_str("\n\r");
}

void write_line(uint8_t* str) {
	write_str(str);
	write_new_line();
}

enum key {
	ONE = 110,
	TWO = 94,
	THREE = 62,
	FOUR = 109,
	FIVE = 93,
	SIX = 61,
	SEVEN = 107,
	EIGHT = 91,
	NINE = 59,
	TEN = 103,
	ELEVEN = 87,
	TWELVE = 55,
	NONE = 0
};

uint8_t read_symbol(uint8_t row) {
	uint8_t configuration_comman = 0x70;
	uint8_t command_c = 0;
	uint8_t r_symbol = 0;

	HAL_I2C_Mem_Write(&hi2c1,0xE2,0x03,1,&configuration_comman,1,100);
	HAL_I2C_Mem_Write(&hi2c1,0xE2,0x01,1,&command_c,1,100);
	HAL_I2C_Mem_Read(&hi2c1,0xE3,0x00,1,&r_symbol,1,100);


	uint8_t configuration_command = 0x70;
	uint8_t command = 0xF7 >> (3 - row);
	uint8_t read_symbol = 0;

	HAL_I2C_Mem_Write(&hi2c1,0xE2,0x03,1,&configuration_command,1,100);
	HAL_I2C_Mem_Write(&hi2c1,0xE2,0x01,1,&command,1,100);
	HAL_I2C_Mem_Read(&hi2c1,0xE3,0x00,1,&read_symbol,1,100);

	return read_symbol;
}

enum key get_key() {


	uint8_t symbol = read_symbol(0);
	if (symbol == ONE) {
		return ONE;
	}
	if (symbol == TWO) {
		return TWO;
	}
	if (symbol == THREE) {
		return THREE;
	}

	symbol = read_symbol(1);
	if (symbol == FOUR) {
		return FOUR;
	}
	if (symbol == FIVE) {
		return FIVE;
	}
	if (symbol == SIX) {
		return SIX;
	}


	symbol = read_symbol(2);
	if (symbol == SEVEN) {
		return SEVEN;
	}
	if (symbol == EIGHT) {
		return EIGHT;
	}
	if (symbol == NINE) {
		return NINE;
	}

	symbol = read_symbol(3);
	if (symbol == TEN) {
		return TEN;
	}
	if (symbol == ELEVEN) {
		return ELEVEN;
	}
	if (symbol == TWELVE) {
		return TWELVE;
	}
	return NONE;
}

int main(void) {
	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_USART6_UART_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	oled_Init();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	//HAL_UART_Receive_IT(&huart6, rxByte, 1);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	int position = 1;
	int prev_data = default_data;
	while (1) {
//		print_menu(position);
//
//		HAL_I2C_Mem_Read(&hi2c1, 0xe3, 0x00, I2C_MEMADD_SIZE_8BIT, &data,
//				sizeof(uint8_t), 10);
//		if (data != prev_data) {
//			prev_data = data;
//			data -= default_data;
//			if (data == 1 || data == 4) {
//				if (data == 1)
//					position += 2;
//
//				position = position % 4;
//				position++;
//			}
//		}

		 HAL_Delay(100);
		 enum key current_key = get_key();
		/*
			  uint8_t symbol = read_symbol();
			  	  	  if (symbol == 's' || symbol == 'c' || symbol == 't') {
			  	  	  	  oled_Fill(Black);
			  	  	  	  oled_SetCursor(56, 24);
			  	  	  	  oled_WriteChar(symbol, Font_Geometric_16x16, White);
			  	  	  	  oled_UpdateScreen();
			  	  	  }*/
	}

	/* USER CODE END 3 */

}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE()
	;
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart6) {
		proceedInput();
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart6) {
	__NOP();
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
