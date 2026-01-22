/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbh_hid.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CONTROLBYSTEPS 0 //Define si controlar los motores por pasos (para depurar desde el pc,
//controlar su posición, o si por el contrario ser controlado por el mando
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim11;

UART_HandleTypeDef huart3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for UartRx */
osThreadId_t UartRxHandle;
const osThreadAttr_t UartRx_attributes = {
  .name = "UartRx",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for controlMotor1 */
osThreadId_t controlMotor1Handle;
const osThreadAttr_t controlMotor1_attributes = {
  .name = "controlMotor1",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for controlMotor2 */
osThreadId_t controlMotor2Handle;
const osThreadAttr_t controlMotor2_attributes = {
  .name = "controlMotor2",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for controlMotor3 */
osThreadId_t controlMotor3Handle;
const osThreadAttr_t controlMotor3_attributes = {
  .name = "controlMotor3",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for controlMotor4 */
osThreadId_t controlMotor4Handle;
const osThreadAttr_t controlMotor4_attributes = {
  .name = "controlMotor4",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for inputChangeTask */
osThreadId_t inputChangeTaskHandle;
const osThreadAttr_t inputChangeTask_attributes = {
  .name = "inputChangeTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for rxQueue */
osMessageQueueId_t rxQueueHandle;
const osMessageQueueAttr_t rxQueue_attributes = {
  .name = "rxQueue"
};
/* Definitions for gamepadChangesQueue */
osMessageQueueId_t gamepadChangesQueueHandle;
const osMessageQueueAttr_t gamepadChangesQueue_attributes = {
  .name = "gamepadChangesQueue"
};
/* Definitions for stepperTrigger */
osEventFlagsId_t stepperTriggerHandle;
const osEventFlagsAttr_t stepperTrigger_attributes = {
  .name = "stepperTrigger"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM11_Init(void);
void StartDefaultTask(void *argument);
void uartRxDataManagement(void *argument);
void controlMotor1Task(void *argument);
void controlMotor2Task(void *argument);
void controlMotor3Task(void *argument);
void controlMotor4Task(void *argument);
void StartInputChangeTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#define buttonYBit 1
#define buttonBBit 1<<1
#define buttonABit 1<<2
#define buttonXBit 1<<3
#define buttonLBit 1<<4
#define buttonRBit 1<<5
#define buttonZLBit 1<<6
#define buttonZRBit 1<<7
#define buttonMinusBit 1<<8
#define buttonPlusBit 1<<9
#define buttonLJoyStickBit 1<<10
#define buttonRJoyStickBit 1<<11
#define buttonHomeBit 1<<12
#define buttonSelectBit 1<<13

typedef enum {
	NONE = 0b1111,
	UP = 0b0,
	UPR = 0b1,
	RIGHT = 0b10,
	DOWNR = 0b11,
	DOWN = 0b100,
	DOWNL = 0b101,
	LEFT = 0b110,
	UPL = 0b111
} crossDir;

typedef enum {
	NOEVENT,
	//Hat
	GP_EVENT_DPAD_PRESS_CENTER,

	GP_EVENT_DPAD_PRESS_RIGHT,
	GP_EVENT_DPAD_HOLD_RIGHT,
	GP_EVENT_DPAD_RELEASE_RIGHT,
	GP_EVENT_DPAD_PRESS_LEFT,
	GP_EVENT_DPAD_HOLD_LEFT,
	GP_EVENT_DPAD_RELEASE_LEFT,

	GP_EVENT_DPAD_PRESS_UP,
	GP_EVENT_DPAD_HOLD_UP,
	GP_EVENT_DPAD_RELEASE_UP,
	GP_EVENT_DPAD_PRESS_DOWN,
	GP_EVENT_DPAD_HOLD_DOWN,
	GP_EVENT_DPAD_RELEASE_DOWN,

	//Buttons
	GP_EVENT_PRESS_A,
	GP_EVENT_HOLD_A,
	GP_EVENT_RELEASE_A,

	GP_EVENT_PRESS_B,
	GP_EVENT_HOLD_B,
	GP_EVENT_RELEASE_B,

	GP_EVENT_PRESS_X,
	GP_EVENT_HOLD_X,
	GP_EVENT_RELEASE_X,

	GP_EVENT_PRESS_Y,
	GP_EVENT_HOLD_Y,
	GP_EVENT_RELEASE_Y,

	GP_EVENT_PRESS_L,
	GP_EVENT_HOLD_L,
	GP_EVENT_RELEASE_L,

	GP_EVENT_PRESS_R,
	GP_EVENT_HOLD_R,
	GP_EVENT_RELEASE_R,

	GP_EVENT_PRESS_ZL,
	GP_EVENT_HOLD_ZL,
	GP_EVENT_RELEASE_ZL,

	GP_EVENT_PRESS_ZR,
	GP_EVENT_HOLD_ZR,
	GP_EVENT_RELEASE_ZR,

	GP_EVENT_PRESS_PLUS,
	GP_EVENT_HOLD_PLUS,
	GP_EVENT_RELEAS_PLUS,

	GP_EVENT_PRESS_MINUS,
	GP_EVENT_HOLD_MINUS,
	GP_EVENT_RELEASE_MINUS,

	GP_EVENT_PRESS_HOME,
	GP_EVENT_HOLD_HOME,
	GP_EVENT_RELEASE_HOME,

	GP_EVENT_PRESS_SELECT,
	GP_EVENT_HOLD_SELECT,
	GP_EVENT_RELEASE_SELECT,

	GP_EVENT_PRESS_JL,
	GP_EVENT_HOLD_JL,
	GP_EVENT_RELEASE_JL,

	GP_EVENT_PRESS_JR,
	GP_EVENT_HOLD_JR,
	GP_EVENT_RELEASE_JR,

	//Joysticks
	GP_EVENT_LJS_PRESS_CENTER,

	GP_EVENT_LJS_PRESS_RIGHT,
	GP_EVENT_LJS_HOLD_RIGHT,
	GP_EVENT_LJS_RELEASE_RIGHT,
	GP_EVENT_LJS_PRESS_LEFT,
	GP_EVENT_LJS_HOLD_LEFT,
	GP_EVENT_LJS_RELEASE_LEFT,

	GP_EVENT_LJS_PRESS_UP,
	GP_EVENT_LJS_HOLD_UP,
	GP_EVENT_LJS_RELEASE_UP,
	GP_EVENT_LJS_PRESS_DOWN,
	GP_EVENT_LJS_HOLD_DOWN,
	GP_EVENT_LJS_RELEASE_DOWN,

	GP_EVENT_RJS_PRESS_CENTER,

	GP_EVENT_RJS_PRESS_RIGHT,
	GP_EVENT_RJS_HOLD_RIGHT,
	GP_EVENT_RJS_RELEASE_RIGHT,
	GP_EVENT_RJS_PRESS_LEFT,
	GP_EVENT_RJS_HOLD_LEFT,
	GP_EVENT_RJS_RELEASE_LEFT,

	GP_EVENT_RJS_PRESS_UP,
	GP_EVENT_RJS_HOLD_UP,
	GP_EVENT_RJS_RELEASE_UP,
	GP_EVENT_RJS_PRESS_DOWN,
	GP_EVENT_RJS_HOLD_DOWN,
	GP_EVENT_RJS_RELEASE_DOWN,

} gamepadEvents;

/**
 * Estructura que almacena el estado actual del control (gamepad).
 *
 * Esta estructura se actualiza cuando llega un reporte HID desde el dispositivo USB.

 * Los valores de los joysticks se convierten desde un rango típico de 0..255
 * a un rango centrado en 0 mediante {@code valor - 128}.
 */
typedef struct {
	crossDir dir;
	int16_t buttons, joyStickLX, joyStickLY, joyStickRX, joyStickRY;
} GamepadReport;
GamepadReport currentReport, previousReport;

void USBH_HID_EventCallback(USBH_HandleTypeDef *phost) {
	uint8_t report[64];
	uint16_t len = USBH_HID_GetReportData(phost, report, sizeof(report));
	if (len > 0) {
		currentReport.dir = report[2];
		currentReport.buttons = (uint16_t) report[0]
				| ((uint16_t) report[1] << 8);
		currentReport.joyStickLX = (int8_t) ((int16_t) report[3] - 128);
		currentReport.joyStickLY = (int8_t) ((int16_t) report[4] - 128);
		currentReport.joyStickRX = (int8_t) ((int16_t) report[5] - 128);
		currentReport.joyStickRY = (int8_t) ((int16_t) report[6] - 128);

		//COMPROBAR LAS DIFERENCIAS Y ENCOLARLAS
		checkDifferencesInJoySticks();
		memcpy(&previousReport, &currentReport, sizeof(previousReport));
	}

}

void pushEventIntoQueue(gamepadEvents event, uint32_t timeout) {
	//El timeout la pongo como parametro para que eventos importantes no se dejen atras
	osMessageQueuePut(gamepadChangesQueueHandle, &event, 4, timeout);
}

void checkDifferencesInJoySticks() {
	//Joystick izq, eje X
	int currDir = (currentReport.joyStickLX > 0)
			- (currentReport.joyStickLX < 0); // 1=der, 0= centro, -1=izq
	int previousDir = (previousReport.joyStickLX > 0)
			- (previousReport.joyStickLX < 0);

	if (currDir == 1 && previousDir != 1)
		pushEventIntoQueue(GP_EVENT_LJS_PRESS_RIGHT, 0);
//	if(currDir == 1 && previousDir == 1) pushEventIntoQueue(GP_EVENT_LJS_HOLD_RIGHT, 0);
	if (currDir != 1 && previousDir == 1)
		pushEventIntoQueue(GP_EVENT_LJS_RELEASE_RIGHT, 0);

	if (currDir == -1 && previousDir != -1)
		pushEventIntoQueue(GP_EVENT_LJS_PRESS_LEFT, 0);
//	if(currDir == -1 && previousDir == -1) pushEventIntoQueue(GP_EVENT_LJS_HOLD_LEFT, 0);
	if (currDir != -1 && previousDir == -1)
		pushEventIntoQueue(GP_EVENT_LJS_RELEASE_LEFT, 0);
	//******************************************************************
	//Boton ZL para arranque y apagado motor 4
	if (currentReport.buttons & buttonZLBit
			&& !(previousReport.buttons & buttonZLBit))
		pushEventIntoQueue(GP_EVENT_PRESS_ZL, 0);
	if (!(currentReport.buttons & buttonZLBit)
			&& previousReport.buttons & buttonZLBit)
		pushEventIntoQueue(GP_EVENT_RELEASE_ZL, 0);
	//******************************************************************
	//Boton L para arranque y apagado motor 2
	if (currentReport.buttons & buttonLBit
			&& !(previousReport.buttons & buttonLBit))
		pushEventIntoQueue(GP_EVENT_PRESS_L, 0);
	if (!(currentReport.buttons & buttonLBit)
			&& previousReport.buttons & buttonLBit)
		pushEventIntoQueue(GP_EVENT_RELEASE_L, 0);
	//******************************************************************
	//Joystick izq, eje Y
	currDir = (currentReport.joyStickLY > 0) - (currentReport.joyStickLY < 0); // 1=arriba, 0= centro, -1=abajo
	previousDir = (previousReport.joyStickLY > 0)
			- (previousReport.joyStickLY < 0);

	if (currDir == 1 && previousDir != 1)
		pushEventIntoQueue(GP_EVENT_LJS_PRESS_UP, 0);
//	if(currDir == 1 && previousDir == 1) pushEventIntoQueue(GP_EVENT_LJS_HOLD_UP, 0);
	if (currDir != 1 && previousDir == 1)
		pushEventIntoQueue(GP_EVENT_LJS_RELEASE_UP, 0);

	if (currDir == -1 && previousDir != -1)
		pushEventIntoQueue(GP_EVENT_LJS_PRESS_DOWN, 0);
//	if(currDir == -1 && previousDir == -1) pushEventIntoQueue(GP_EVENT_LJS_HOLD_DOWN, 0);
	if (currDir != -1 && previousDir == -1)
		pushEventIntoQueue(GP_EVENT_LJS_RELEASE_DOWN, 0);
	//******************************************************************
	//Joystick der, eje Y
	currDir = (currentReport.joyStickRY > 0) - (currentReport.joyStickRY < 0); // 1=arriba, 0= centro, -1=abajo
	previousDir = (previousReport.joyStickRY > 0)
			- (previousReport.joyStickRY < 0);

	if (currDir == 1 && previousDir != 1)
		pushEventIntoQueue(GP_EVENT_RJS_PRESS_UP, 0);
//	if(currDir == 1 && previousDir == 1) pushEventIntoQueue(GP_EVENT_RJS_HOLD_UP, 0);
	if (currDir != 1 && previousDir == 1)
		pushEventIntoQueue(GP_EVENT_RJS_RELEASE_UP, 0);

	if (currDir == -1 && previousDir != -1)
		pushEventIntoQueue(GP_EVENT_RJS_PRESS_DOWN, 0);
//	if(currDir == -1 && previousDir == -1) pushEventIntoQueue(GP_EVENT_RJS_HOLD_DOWN, 0);
	if (currDir != -1 && previousDir == -1)
		pushEventIntoQueue(GP_EVENT_RJS_RELEASE_DOWN, 0);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of rxQueue */
  rxQueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &rxQueue_attributes);

  /* creation of gamepadChangesQueue */
  gamepadChangesQueueHandle = osMessageQueueNew (32, sizeof(int), &gamepadChangesQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of UartRx */
  UartRxHandle = osThreadNew(uartRxDataManagement, NULL, &UartRx_attributes);

  /* creation of controlMotor1 */
  controlMotor1Handle = osThreadNew(controlMotor1Task, NULL, &controlMotor1_attributes);

  /* creation of controlMotor2 */
  controlMotor2Handle = osThreadNew(controlMotor2Task, NULL, &controlMotor2_attributes);

  /* creation of controlMotor3 */
  controlMotor3Handle = osThreadNew(controlMotor3Task, NULL, &controlMotor3_attributes);

  /* creation of controlMotor4 */
  controlMotor4Handle = osThreadNew(controlMotor4Task, NULL, &controlMotor4_attributes);

  /* creation of inputChangeTask */
  inputChangeTaskHandle = osThreadNew(StartInputChangeTask, NULL, &inputChangeTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of stepperTrigger */
  stepperTriggerHandle = osEventFlagsNew(&stepperTrigger_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 384;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 47;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2042;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */

  /* USER CODE END TIM11_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM11_Init 1 */

  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 47;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 2042;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim11, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */

  /* USER CODE END TIM11_Init 2 */
  HAL_TIM_MspPostInit(&htim11);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, Dir2_Pin|EnableMotor2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, Step3_Pin|Enable4_Pin|Dir4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, Step1_Pin|EnableMotor1_Pin|USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, Dir1_Pin|EnableMotor3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Dir2_Pin EnableMotor2_Pin */
  GPIO_InitStruct.Pin = Dir2_Pin|EnableMotor2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : Step3_Pin Enable4_Pin Dir4_Pin */
  GPIO_InitStruct.Pin = Step3_Pin|Enable4_Pin|Dir4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : Dir3_Pin */
  GPIO_InitStruct.Pin = Dir3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Dir3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Step1_Pin EnableMotor1_Pin USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = Step1_Pin|EnableMotor1_Pin|USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Dir1_Pin EnableMotor3_Pin */
  GPIO_InitStruct.Pin = Dir1_Pin|EnableMotor3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
#define RX_BUFFER_SIZE 4
#define stepMultiplier 19

unsigned char rxBuffer[RX_BUFFER_SIZE];
int stepsToMakeMotor1 = 0;
int stepsToMakeMotor2 = 0;
int stepsToMakeMotor3 = 0;
int stepsToMakeMotor4 = 0;

/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	osMessageQueuePut(rxQueueHandle, &rxBuffer, 4, 0);
	// Reiniciar la recepción para recibir más datos
	HAL_UART_Receive_IT(&huart3, &rxBuffer, RX_BUFFER_SIZE);
}

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 5 */
	HAL_UART_Receive_IT(&huart3, &rxBuffer, RX_BUFFER_SIZE);

	/* Infinite loop */
//	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0, GPIO_PIN_RESET); //Motor1 encendido
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_1, GPIO_PIN_SET); //Disable motor 1
	HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, GPIO_PIN_SET); //Disable motor 2
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET); //Disable motor 3
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET); //Disable motor 4

	for (;;) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // Conmutar el estado del LED1
		osDelay(45);

	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_uartRxDataManagement */
/**
 * @brief Function implementing the UartRx thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_uartRxDataManagement */
void uartRxDataManagement(void *argument)
{
  /* USER CODE BEGIN uartRxDataManagement */
	/* Infinite loop */
	for (;;) {
		osMessageQueueGet(rxQueueHandle, &rxBuffer, 4, osWaitForever);
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7); // Conmutar el estado del LED2 azul
		if (CONTROLBYSTEPS) {
			switch (rxBuffer[0]) {
			case 'M': //Mover motor 1
				if (rxBuffer[1])
					stepsToMakeMotor1 += rxBuffer[2] * stepMultiplier; //Dir pos
				else
					stepsToMakeMotor1 -= rxBuffer[2] * stepMultiplier; //Dir neg

				osEventFlagsSet(stepperTriggerHandle, 0x0001U);
				break;

			case 'N': //Mover motor 2
				if (rxBuffer[1])
					stepsToMakeMotor2 += rxBuffer[2] * stepMultiplier; //Dir pos
				else
					stepsToMakeMotor2 -= rxBuffer[2] * stepMultiplier; //Dir neg

				osEventFlagsSet(stepperTriggerHandle, 2);
				break;

			case 'O': //Mover motor 3
				if (rxBuffer[1])
					stepsToMakeMotor3 += rxBuffer[2] * stepMultiplier; //Dir pos
				else
					stepsToMakeMotor3 -= rxBuffer[2] * stepMultiplier; //Dir neg

				osEventFlagsSet(stepperTriggerHandle, 4);
				break;

			case 'P': //Mover motor 4
				if (rxBuffer[1])
					stepsToMakeMotor4 += rxBuffer[2] * stepMultiplier; //Dir pos
				else
					stepsToMakeMotor4 -= rxBuffer[2] * stepMultiplier; //Dir neg

				osEventFlagsSet(stepperTriggerHandle, 8);
				break;

			case 'T': //Encender o apagar motor
				switch (rxBuffer[1]) {
				case 1:
					HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_1); //Conmutar motor 1
					break;
				case 2:
					HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9); //Conmutar motor 2
					break;
				case 3:
					HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_7); //Conmutar motor 3
					break;
				case 4:
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8); //Conmutar motor 4
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
			rxBuffer[0] = 0;
		}
	}
  /* USER CODE END uartRxDataManagement */
}

/* USER CODE BEGIN Header_controlMotor1Task */
//TODO
/* USER CODE END Header_controlMotor1Task */
void controlMotor1Task(void *argument)
{
  /* USER CODE BEGIN controlMotor1Task */
	/* Infinite loop */
	for (;;) {
		osEventFlagsWait(stepperTriggerHandle, 1, osFlagsWaitAny,
		osWaitForever);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //Led rojo

		for (;;) {
			if (stepsToMakeMotor1 > 0) {
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_SET);  //Dir pos
				stepsToMakeMotor1--;
			} else if (stepsToMakeMotor1 < 0) {
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_1, GPIO_PIN_RESET);  //Dir neg
				stepsToMakeMotor1++;
			} else {
				break;
			}

			HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_0); //Avanzar el servo medio step
			osDelay(1); //TODO cambiar este valor para cambiar la velocidad
			HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_0); //Avanzar el servo medio step
			osDelay(1); //TODO cambiar este valor para cambiar la velocidad
		}
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //Led rojo
		osEventFlagsClear(stepperTriggerHandle, 1);
	}
  /* USER CODE END controlMotor1Task */
}

/* USER CODE BEGIN Header_controlMotor2Task */
int motor2Speed = 5;
/* USER CODE END Header_controlMotor2Task */
void controlMotor2Task(void *argument)
{
  /* USER CODE BEGIN controlMotor2Task */
	/* Infinite loop */
	for (;;) {
		osEventFlagsWait(stepperTriggerHandle, 2, osFlagsWaitAny,
		osWaitForever);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); //Led rojo

		for (;;) {
			if (stepsToMakeMotor2 > 0) {
				HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_SET);  //Dir pos
				stepsToMakeMotor2--;
				motor2Speed = 75;
			} else if (stepsToMakeMotor2 < 0) {
				HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);  //Dir neg
				stepsToMakeMotor2++;
				motor2Speed = 5;
			} else {
				break;
			}

			HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_7); //Avanzar el servo medio step
			osDelay(1); //TODO cambiar este valor para cambiar la velocidad
			HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_7); //Avanzar el servo medio step
			osDelay(1); //TODO cambiar este valor para cambiar la velocidad

		}
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //Led rojo
		osEventFlagsClear(stepperTriggerHandle, 2);
	}
  /* USER CODE END controlMotor2Task */
}

/* USER CODE BEGIN Header_controlMotor3Task */
/**
 * @brief Function implementing the controlMotor3 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_controlMotor3Task */
void controlMotor3Task(void *argument)
{
  /* USER CODE BEGIN controlMotor3Task */
	/* Infinite loop */
	for (;;) {
		osEventFlagsWait(stepperTriggerHandle, 4, osFlagsWaitAny,
		osWaitForever);

		for (;;) {
			if (stepsToMakeMotor3 > 0) {
				HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_SET);  //Dir pos
				stepsToMakeMotor3--;
			} else if (stepsToMakeMotor3 < 0) {
				HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);  //Dir neg
				stepsToMakeMotor3++;
			} else {
				break;
			}

			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0); //Avanzar el servo medio step
			osDelay(75); //TODO cambiar este valor para cambiar la velocidad
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0); //Avanzar el servo medio step
			osDelay(75); //TODO cambiar este valor para cambiar la velocidad
		}
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); //Led rojo
		osEventFlagsClear(stepperTriggerHandle, 4);
	}
  /* USER CODE END controlMotor3Task */
}

/* USER CODE BEGIN Header_controlMotor4Task */
void funcM4() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1); //Led rojo
	if (stepsToMakeMotor4 > 0) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);  //Dir pos
		stepsToMakeMotor4--;
	} else if (stepsToMakeMotor4 < 0) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);  //Dir neg
		stepsToMakeMotor4++;
	} else {
		HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_4); // Detener el PWM en el canal 4
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0); //Led rojo
	}
}
/* USER CODE END Header_controlMotor4Task */
void controlMotor4Task(void *argument)
{
  /* USER CODE BEGIN controlMotor4Task */
	//TIM3->CCR4 = 1021; //Funcionaba con 2k
	TIM3->CCR4 = 1021;

	/* Infinite loop */
	for (;;) {
		osEventFlagsWait(stepperTriggerHandle, 8, osFlagsWaitAny,
		osWaitForever);
		HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_4);

		osEventFlagsClear(stepperTriggerHandle, 8);
	}
  /* USER CODE END controlMotor4Task */
}

/* USER CODE BEGIN Header_StartInputChangeTask */
/**
 * @brief Function implementing the inputChangeTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartInputChangeTask */
void StartInputChangeTask(void *argument)
{
  /* USER CODE BEGIN StartInputChangeTask */
	gamepadEvents gamepadEventsBuffer;
	TIM11->CCR1 = 2021;
	/* Infinite loop */
	for (;;) {
		osMessageQueueGet(gamepadChangesQueueHandle, &gamepadEventsBuffer, 0,
				osWaitForever);

		switch (gamepadEventsBuffer) {
		//MOTOR4
		case GP_EVENT_LJS_PRESS_RIGHT:
			//Activar el timer del motor4
			HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_4);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);  //Dir pos
			break;
		case GP_EVENT_LJS_RELEASE_RIGHT:
			//Desactivar el timer del motor4
			if (currentReport.joyStickLX == 0)
				HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_4); // Detener el PWM en el canal 4
			break;
		case GP_EVENT_LJS_PRESS_LEFT:
			//Activar el timer del motor4
			HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_4);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);  //Dir pos
			break;
		case GP_EVENT_LJS_RELEASE_LEFT:
			//Desactivar el timer del motor4
			if (currentReport.joyStickLX == 0)
				HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_4); // Detener el PWM en el canal 4
			break;
		case GP_EVENT_RELEASE_ZL:
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8); //Conmutar motor 4 (base)
			break;


		//MOTOR2
		case GP_EVENT_RELEASE_L:
			HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9); //Conmutar motor 2
			break;
		case GP_EVENT_RJS_PRESS_UP:
			HAL_TIM_PWM_Start_IT(&htim11, TIM_CHANNEL_1);
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_SET);  //Dir pos
			break;
		case GP_EVENT_RJS_RELEASE_UP:
			if (currentReport.joyStickLY == 0)
				HAL_TIM_PWM_Stop_IT(&htim11, TIM_CHANNEL_1); // Detener el PWM
			break;
		case GP_EVENT_RJS_PRESS_DOWN:
			HAL_TIM_PWM_Start_IT(&htim11, TIM_CHANNEL_1);
			HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);  //Dir neg
			break;
		case GP_EVENT_RJS_RELEASE_DOWN:
			if (currentReport.joyStickLY == 0)
				HAL_TIM_PWM_Stop_IT(&htim11, TIM_CHANNEL_1); // Detener el PWM
			break;


		case NOEVENT:
			break;
		}

	}
  /* USER CODE END StartInputChangeTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
	if (htim->Instance == TIM3) {
		funcM4();
	}
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
