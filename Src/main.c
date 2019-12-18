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
#include "game.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "fonts.h"


#define SAVES_COUNT 10
// game speed (IT'S NOT TRUE, speed is ( 10 - speedwagon) % 10 )
int speedwagon = 1;

uint8_t prev = 112;
uint8_t data = 1;
uint8_t default_data = 112;
bool in_game = false;
saves_t saves;

void SystemClock_Config(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
enum KEY
{
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

enum MENU_TYPE
{
	MAIN = 0,
	SAVE = 1,
	LOAD = 2,
	SETTINGS = 4,
	NEW_GAME = 8
};

// todo make struct from meta
// meta is some info about display
// for MAIN it is position of cursor
// for settings it is value of speedwagon
void print_menu(int meta, enum MENU_TYPE menu_type)
{
	char new_game[50];
	char load[50];
	char save[50];
	char settings[50];
	oled_Fill(Black);

	switch (menu_type)
	{
	case MAIN:
		sprintf(new_game, "%cNew Game", meta == 1 ? '>' : ' ');
		sprintf(load, "%cLoad", meta == 2 ? '>' : ' ');
		sprintf(save, "%cSave", meta == 3 ? '>' : ' ');
		sprintf(settings, "%cSettings", meta == 4 ? '>' : ' ');
		oled_SetCursor(0, 0);
		oled_WriteString(new_game, Font_7x10, White);
		oled_SetCursor(0, 15);
		oled_WriteString(load, Font_7x10, White);
		oled_SetCursor(0, 30);
		oled_WriteString(save, Font_7x10, White);
		oled_SetCursor(0, 45);
		oled_WriteString(settings, Font_7x10, White);
		break;
	case SETTINGS:
		sprintf(settings, "Speed: < %d >", meta);
		oled_SetCursor(0, 0);
		oled_WriteString(settings, Font_7x10, White);
		break;
	case LOAD:
        sprintf(settings, "Name: < Save%d >", meta);
        oled_SetCursor(0, 0);
        oled_WriteString(settings, Font_7x10, White);
        break;
	case SAVE:
	default:
		oled_SetCursor(0, 0);
		oled_WriteString("Wrong menu type", Font_7x10, White);
	}
	oled_UpdateScreen();
}

typedef struct {
    field_t[SAVES_COUNT] saves;
    int current_saves_count;
} saves_t;

uint8_t read_symbol(uint8_t row)
{
	uint8_t configuration_comman = 0x70;
	uint8_t command_c = 0;
	uint8_t r_symbol = 0;

	HAL_I2C_Mem_Write(&hi2c1, 0xE2, 0x03, 1, &configuration_comman, 1, 100);
	HAL_I2C_Mem_Write(&hi2c1, 0xE2, 0x01, 1, &command_c, 1, 100);
	HAL_I2C_Mem_Read(&hi2c1, 0xE3, 0x00, 1, &r_symbol, 1, 100);

	uint8_t configuration_command = 0x70;
	uint8_t command = 0xF7 >> (3 - row);
	uint8_t read_symbol = 0;

	HAL_I2C_Mem_Write(&hi2c1, 0xE2, 0x03, 1, &configuration_command, 1, 100);
	HAL_I2C_Mem_Write(&hi2c1, 0xE2, 0x01, 1, &command, 1, 100);
	HAL_I2C_Mem_Read(&hi2c1, 0xE3, 0x00, 1, &read_symbol, 1, 100);

	return read_symbol;
}

enum KEY get_key()
{
	uint8_t symbol = read_symbol(0);
	if (symbol == ONE)
		return ONE;
	if (symbol == TWO)
		return TWO;
	if (symbol == THREE)
		return THREE;

	symbol = read_symbol(1);
	if (symbol == FOUR)
		return FOUR;
	if (symbol == FIVE)
		return FIVE;
	if (symbol == SIX)
		return SIX;

	symbol = read_symbol(2);
	if (symbol == SEVEN)
		return SEVEN;
	if (symbol == EIGHT)
		return EIGHT;
	if (symbol == NINE)
		return NINE;

	symbol = read_symbol(3);
	if (symbol == TEN)
		return TEN;
	if (symbol == ELEVEN)
		return ELEVEN;
	if (symbol == TWELVE)
		return TWELVE;

	return NONE;
}

enum MENU_TYPE get_menu_type(int position)
{
	switch (position)
	{
	case 1:
		return NEW_GAME;
	case 2:
		return LOAD;
	case 3:
		return SAVE;
	case 4:
		return SETTINGS;
	default:
		return MAIN;
	}
}

int get_new_position(int position, enum MENU_TYPE menu_type, enum KEY current_key)
{
	switch (menu_type)
	{
	case MAIN:
		switch (current_key)
		{
		case TWO:
			position += 2;
		case EIGHT:
			position = position % 4;
			position++;
			break;
		default:
			break;
		}
	case SETTINGS:
		break;
	case LOAD:
        switch (current_key)
        {
            case FOUR:
                position = position == 1 ? 1 : position - 1;
            case SIX:
                position = position == saves.current_saves_count ? saves.current_saves_count : position + 1;
                break;
            default:
                break;
        }
	    break;
	default:
		return 0;
	}

	return position;
}

static void print_field(field_t *field)
{
	char line[WIDTH];
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			line[j] = field->plane[i][j];
		}

		oled_SetCursor(0, i * 4);
		oled_WriteString(line, Font_4x4, White);
	}
	oled_UpdateScreen();
}

// foo declarations

static void show_menu(field_t *field);

//

// todo: delete, just fill field for debug
static void set_start_field(field_t *field)
{
	field->plane[1][4] = L_CELL;
	field->plane[2][4] = L_CELL;
	field->plane[3][4] = L_CELL;
	field->plane[3][3] = L_CELL;
	field->plane[2][2] = L_CELL;
}

// settings
static void apply_setting(int position, enum KEY current_key)
{
	switch (position)
	{
	case 1: // speed section
		if (current_key == FOUR)
			speedwagon = (speedwagon + 9) % 10;
		if (current_key == SIX)
			speedwagon = (speedwagon + 1) % 10;
		return;
	case 2: // shall we add something else?
		break;
	}
}

static void process_settings()
{
	int position = 1;
	while (1)
	{
		HAL_Delay(100);
		print_menu(speedwagon, SETTINGS);
		enum KEY current_key = get_key();

		if (current_key == FIVE)
			break;
		apply_setting(position, current_key);
		position = get_new_position(position, SETTINGS, current_key);
	}
}

static void toggle_cell(field_t *field, int x, int y)
{
	if (field->plane[y][x] == D_CELL)
		field->plane[y][x] = L_CELL;
	else
		field->plane[y][x] = D_CELL;
}

static void get_position_on_field(int *x, int *y, enum KEY current_key)
{
	switch (current_key)
	{
	case TWO:
		*y = *y + HEIGHT - 1 % HEIGHT;
		break;
	case FOUR:
		*x = *x + WIDTH - 1 % WIDTH;
		break;
	case SIX:
		*x = *x + 1 % WIDTH;
		break;
	case EIGHT:
		*y = *y + 1 % HEIGHT;
		break;
	}
}

static void process_pause(field_t *field)
{
	int x = 0;
	int y = 0;

	while (1)
	{
		HAL_Delay(100);
		print_field(field);
		enum KEY current_key = get_key();

		if (current_key == FIVE)
			toggle_cell(field, x, y);
		get_position_on_field(&x, &y, current_key);
	}
}

// process input during the game
static void process_input(field_t *field)
{
	enum KEY current_key = get_key();

	switch (current_key)
	{
	case NONE:
		return;
	case SEVEN:
		show_menu(field);
		break;
	case NINE:
		process_pause(field);
		break;
	}
}

// game
static void betta_game(field_t *field)
{

	while (1)
	{
		print_field(field);
		calc_step(field);
		process_input(field);
		HAL_Delay(100 * (10 - speedwagon));
	}
}

void copy_field(field_t* source, field_t* destination){
    for (int i = 0; i < source->n; ++i)
        for (int j = 0; j < source->m; ++j)
            destination->plane[i][j] = source->plane[i][j];
}

static void process_save(field_t* field){
    char string[50];
    if (saves.current_saves_count == SAVES_COUNT){
        sprintf(string, "limit exceeded.")
        oled_SetCursor(0, 15);
        oled_WriteString(string, Font_7x10, White);
        oled_UpdateScreen();
        return;
    }
    copy_field(field, saves.saves[saves.current_saves_count])
    saves.current_saves_count++;
    sprintf(string, "name: save%d", saves.current_saves_count + 1)
    oled_SetCursor(0, 15);
    oled_WriteString(string, Font_7x10, White);
    oled_UpdateScreen();
    HAL_Delay(1000);
}

static void process_load(field_t* field){
    int position = 1;
    while (1)
    {
        HAL_Delay(100);
        print_menu(position, LOAD);
        enum KEY current_key = get_key();

        if (current_key == FIVE) {
            copy_field(saves.saves[position - 1], field);
            break;
        }
        position = get_new_position(position, LOAD, current_key);
    }
}

static void process_menu_option(field_t *field, enum MENU_TYPE menu_type)
{
	switch (menu_type)
	{
	case MAIN:
	case NEW_GAME:
	    in_game = true;
		init_field(field, HEIGHT, WIDTH);
		set_start_field(field);
		betta_game(field);
		break;
	case SAVE:
	    process_save(field);
	    break;
		// мне максимально грустно
	case LOAD:
        process_load(field);
		// мне максимально грустно
	case SETTINGS:
		process_settings();
		break;
	}
}

//menu
static void show_menu(field_t *field)
{
	enum MENU_TYPE menu_type = MAIN;
	int position = 1;
	while (1)
	{
		HAL_Delay(100);
		print_menu(position, MAIN);
		enum KEY current_key = get_key();

		if (current_key == FIVE)
		{
			menu_type = get_menu_type(position);
			process_menu_option(field, menu_type);
		}

		position = get_new_position(position, MAIN, current_key);
	}
}

void init_default_fields(saves_t* saves){
    saves->saves[0]
}

int main(void)
{
	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	saves_t saves;
	init_default_fields(&saves);
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

	field_t field;
	int i = 0;
	while (1)
	{
		show_menu(&field);
		set_start_field(&field);
		betta_game(&field);

		/*
		This is test of cursor.
		We will use ! to show cursor.
		I already configure font_4x4 for this

		oled_SetCursor(0, i += 4);
		oled_WriteString("! ! # #", Font_4x4, White);

		oled_UpdateScreen();
		HAL_Delay(100); 
		*/

		/*
		This is test of values of key on keyboard.
		Some devices has a broken keyboard.

		char buff[50];
		enum KEY current_key = get_key();

		sprintf(buff, "%d", current_key);
		oled_SetCursor(0, 0);
		oled_WriteString(buff, Font_7x10, White);

		oled_UpdateScreen();
		HAL_Delay(100);
		*/

		// 		Not my, you can delete this
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
	}
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart6)
	{
		proceedInput();
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart6)
{
	__NOP();
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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
