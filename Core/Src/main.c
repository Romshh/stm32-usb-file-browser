/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "touch.h"
#include <stdio.h>
#include <string.h>

#include "lvgl/lvgl.h"
#include "ui.h"
#include "actions.h"
#include "tusb.h"
#include "ff.h"
#include "diskio.h"
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

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart3;

HCD_HandleTypeDef hhcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

#define LVGL_BUFFER_PIXELS (LCD_W * (LCD_H / 4))
static uint16_t buffer_pix[LVGL_BUFFER_PIXELS]__attribute__((aligned(4)));
static lv_display_t* disp;
static FATFS fs;

#define FM_VIEW_BYTES 4096

static char fm_pathbuffer[512] = "0:";
static char fm_fullpath[768];
static char fm_filebuf[FM_VIEW_BYTES + 1];
static FIL fm_file;
static lv_obj_t* fm_path_label;
static lv_obj_t* fm_wait_label;
static char fm_volname[24] = "USB";
static bool fm_ready = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_HCD_Init(void);
/* USER CODE BEGIN PFP */
void lcd_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);
void lcd_indev(lv_indev_t * indev, lv_indev_data_t * data);
static void fm_list_fill(void);
static void fm_ui_init(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	lv_indev_t* touch_indev;
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_HCD_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();
  lcd_rect_fill(0, 0, LCD_W, LCD_H, BLACK);
  lv_init();
  lv_tick_set_cb(HAL_GetTick);

  disp = lv_display_create(LCD_W, LCD_H);
  lv_display_set_buffers(disp, buffer_pix, NULL, sizeof(buffer_pix), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, lcd_flush);


  touch_indev = lv_indev_create();
  lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(touch_indev, lcd_indev);

  ui_init();
  fm_ui_init();

  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_SET);
  HAL_Delay(500);
  tusb_rhport_init_t host_init;
  host_init.role = TUSB_ROLE_HOST;
  host_init.speed = TUSB_SPEED_FULL;
  tusb_rhport_init(BOARD_TUH_RH_PORT, &host_init);




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  lv_timer_handler();
	  tuh_task();
	  ui_tick();
	  HAL_Delay(5);

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_HCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hhcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hhcd_USB_OTG_FS.Init.Host_channels = 8;
  hhcd_USB_OTG_FS.Init.speed = HCD_SPEED_FULL;
  hhcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hhcd_USB_OTG_FS.Init.phy_itface = HCD_PHY_EMBEDDED;
  hhcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  if (HAL_HCD_Init(&hhcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
  GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
  GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin PB6 LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|GPIO_PIN_6|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_TXD1_Pin */
  GPIO_InitStruct.Pin = RMII_TXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_TX_EN_Pin RMII_TXD0_Pin */
  GPIO_InitStruct.Pin = RMII_TX_EN_Pin|RMII_TXD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static bool fm_build_full_path(const char* name){
	int written = snprintf(fm_fullpath, sizeof(fm_fullpath), "%s/%s", fm_pathbuffer, name);

	return (written > 0 && (size_t) written < sizeof(fm_fullpath));
}

static void fm_open_viewer(const char* name){
	UINT readcount;

	if(fm_build_full_path(name) == false){
		return;
	}

	if(f_open(&fm_file, fm_fullpath, FA_READ) != FR_OK){
		return;
	}

	if(f_read(&fm_file, fm_filebuf, FM_VIEW_BYTES, &readcount) != FR_OK){
		f_close(&fm_file);
		return;
	}

	if(f_size(&fm_file) > FM_VIEW_BYTES){
		lv_obj_remove_flag(objects.viewer_note, LV_OBJ_FLAG_HIDDEN);
	}
	else{
		lv_obj_add_flag(objects.viewer_note, LV_OBJ_FLAG_HIDDEN);
	}

	f_close(&fm_file);

	fm_filebuf[readcount] = '\0';

	for(UINT i = 0; i < readcount; i++){
		unsigned char c = (unsigned char) fm_filebuf[i];

		if(c != '\n' && c != '\r' && c != '\t' && (c < 0x20 || c > 0x7E)){
			fm_filebuf[i] = '.';
		}
	}

	lv_label_set_text(objects.viewer_title, name);
	lv_textarea_set_text(objects.viewer_text, fm_filebuf);
	loadScreen(SCREEN_ID_VIEWER);
}

static void fm_ui_init(void){
	fm_path_label = lv_label_create(objects.browser_top_bar);
	lv_label_set_long_mode(fm_path_label, LV_LABEL_LONG_MODE_SCROLL);
	lv_obj_set_width(fm_path_label, 170);
	lv_obj_set_style_text_align(fm_path_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
	lv_obj_align(fm_path_label, LV_ALIGN_CENTER, 0, 0);

	fm_wait_label = lv_label_create(objects.waiting);
	lv_label_set_text(fm_wait_label, "Waiting for USB");
	lv_obj_align(fm_wait_label, LV_ALIGN_CENTER, 0, 70);
}

static void fm_row_event(lv_event_t *e){

	lv_obj_t* row = (lv_obj_t*) lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	if(code == LV_EVENT_CLICKED){
		const char* name = lv_list_get_button_text(objects.browser_list, row);

		if(lv_obj_has_flag(row, LV_OBJ_FLAG_USER_1)){
			size_t len = strlen(fm_pathbuffer);
			size_t room = sizeof(fm_pathbuffer) - len;
			int written = snprintf(fm_pathbuffer + len, room, "/%s", name);

			if(written < 0 || (size_t) written >= room){
				fm_pathbuffer[len] = '\0';
				return;
			}

			fm_list_fill();
			return;
		}

		fm_open_viewer(name);
	}
}
static void fm_menu_event(lv_event_t * e){
	lv_obj_t* button = (lv_obj_t*) lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	if(code == LV_EVENT_CLICKED){
		lv_obj_t* icon = lv_obj_get_child(button, 0);
		lv_obj_set_style_text_color(icon, lv_color_hex(0xEB2525), LV_PART_MAIN);
	}
}
static void fm_list_fill(void)
{
    lv_obj_clean(objects.browser_list);

    if(strcmp(fm_pathbuffer, "0:") == 0){
    	lv_label_set_text(fm_path_label, fm_volname);
    	lv_obj_add_state(objects.browser_back_button, LV_STATE_DISABLED);
    }
    else{
    	lv_label_set_text(fm_path_label, fm_pathbuffer);
    	lv_obj_remove_state(objects.browser_back_button, LV_STATE_DISABLED);
    }

    if(fm_ready == false){
    	lv_obj_remove_flag(objects.waiting, LV_OBJ_FLAG_HIDDEN);
    	return;
    }

    lv_obj_add_flag(objects.waiting, LV_OBJ_FLAG_HIDDEN);

    static DIR dirvar;
    static FILINFO filinfovar;
    static FRESULT fresultvar;

    fresultvar = f_opendir(&dirvar,fm_pathbuffer);

    if(fresultvar != FR_OK){
    	return;
    }

    while(f_readdir(&dirvar, &filinfovar) == FR_OK && filinfovar.fname[0] != 0){
    	if(filinfovar.fattrib & (AM_HID | AM_SYS)){
    		continue;
    	}

    	if(filinfovar.fname[0] == '.'){
    		continue;
    	}

    	const void *icon = (filinfovar.fattrib & AM_DIR) ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE;

    	lv_obj_t *row = lv_list_add_button(objects.browser_list, icon, filinfovar.fname);

    	lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    	if(filinfovar.fattrib & AM_DIR){
    		lv_obj_add_flag(row, LV_OBJ_FLAG_USER_1);
    	}

    	lv_obj_add_event_cb(row, fm_row_event, LV_EVENT_CLICKED,      NULL);
    	lv_obj_t * button1 = lv_button_create(row);

    	lv_obj_set_style_bg_opa(button1, LV_OPA_TRANSP, LV_PART_MAIN);
    	lv_obj_set_style_shadow_width(button1, 0, LV_PART_MAIN);
    	lv_obj_set_size(button1, 30,30);
    	lv_obj_add_event_cb(button1, fm_menu_event, LV_EVENT_CLICKED, NULL);

    	lv_obj_t * label1 = lv_label_create(button1);
    	lv_label_set_text(label1, LV_SYMBOL_BARS);
    	lv_obj_set_style_text_color(label1, lv_color_hex(0x000000), LV_PART_MAIN);
    	lv_obj_center(label1);
    }

    f_closedir(&dirvar);

}

void action_open_browser(lv_event_t * e){
	fm_list_fill();
	loadScreen(SCREEN_ID_BROWSER);
}

void action_close_pressed(lv_event_t * e){
	strcpy(fm_pathbuffer, "0:");
	loadScreen(SCREEN_ID_MAIN);
}

void action_viewer_back_pressed(lv_event_t * e){
	loadScreen(SCREEN_ID_BROWSER);
}

void action_back_pressed(lv_event_t * e){
	char* lastslash = strrchr(fm_pathbuffer, '/');

	if(lastslash == NULL){
		return;
	}

	*lastslash = '\0';
	fm_list_fill();

}


void lcd_indev(lv_indev_t * indev, lv_indev_data_t * data){
	static uint16_t touch_x, touch_y, touched;
	touched = touch_get(&touch_x,&touch_y);
	data->point.x = touch_x;
	data->point.y = touch_y;
	if(touched ==1){
		data->state = LV_INDEV_STATE_PRESSED;
	}
	else{
		data->state = LV_INDEV_STATE_RELEASED;
	}
}

void lcd_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map){
	int32_t area_height, area_width;
	area_height = lv_area_get_height(area);
	area_width = lv_area_get_width(area);
	lcd_image(area->x1, area->y1, area_width, area_height, (const uint16_t*) px_map);
	lv_display_flush_ready(disp);
}

void tuh_mount_cb(uint8_t daddr){
	char charbuffer[80];
	uint32_t length;
	length = sprintf(charbuffer,"text daddr:%u \r\n",daddr);
	HAL_UART_Transmit(&huart3, (const uint8_t*) charbuffer, length, 100);

}
void tuh_msc_mount_cb(uint8_t dev_addr){
	char charbuffer[80];
	uint32_t length;
	uint32_t var1 = tuh_msc_get_block_count(dev_addr, 0);
	uint32_t var2 = tuh_msc_get_block_size (dev_addr, 0);
	length = sprintf(charbuffer,"text msc var1:%lu var2:%lu dev_addr:%u \r\n",var1,var2,dev_addr);
	HAL_UART_Transmit(&huart3, (const uint8_t*) charbuffer, length, 100);
	FRESULT fr = f_mount(&fs, "0:", 1);
	length = sprintf(charbuffer,"f_mount rc:%d \r\n", fr);
	HAL_UART_Transmit(&huart3, (const uint8_t*) charbuffer, length, 100);

	if(fr == FR_OK){
		if(f_getlabel("0:", fm_volname, NULL) != FR_OK || fm_volname[0] == '\0'){
			strcpy(fm_volname, "USB");
		}

		fm_ready = true;
		fm_list_fill();
	}


}

uint32_t tusb_time_millis_api(void){
	uint32_t return_time;
	return_time = HAL_GetTick();
	return return_time;
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

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
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
