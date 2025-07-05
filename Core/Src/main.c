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
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "CLCD.h"
#include <stdio.h>
#include "7SEG.h"
#include <math.h>// DAC sin wave
#include "stdbool.h"
#include "rtc.h"
#include <time.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum{
   ZERO,
   YEAR,
   MONTH,
   DATE,
   AMPM,
   HOUR,
   MINUTE,
   SECOND, //6
}_SetTime;
typedef enum{ // use only alarm setting
aMONTH,
aDATE,
//aAMPM,
aHOUR,
aMINUTE,
}_SetAlarm ;
typedef enum{
   NO_SET,
   TIME_SET,
   ALARM_SET,
}_MODE;

typedef enum{
   VIEW_VIEW,
   VIEW_NORMAL,
   VIEW_ALARM,
   VIEW_TEST,
   VIEW_CHANGING,
   //BELL,//3
}_VIEW;
typedef enum{
   ALARM_ZERO,
   ALARM_ONE,
   ALARM_TWO,
}_ALARMCALL;
typedef enum{
   NONE,
   SW_ONE,
   SW_TWO,
   SW_THREE,
   SW_FOUR,
}_SW_BUTT0N;
//-=-=-=-------------------------------rebuild timer code-----------//
typedef enum{
   d_setmonths,//0
    d_January, // 1
    d_February,
    d_March,
    d_April,
    d_May,
    d_June,
    d_July,
    d_August,
    d_September,
    d_October,
    d_November,
    d_December, //12
    d_Overmonths, //13
}D_MONTHS;
//----------------------------------------main mode change test by switch------------------//
typedef enum{
   switch_normal, //0
   switch_normalTimeSet,
   switch_alarm,
   switch_alarmTimeSet,
   switch_test,
   switch_viewchanging,
}_SWITCH_VIEWMODE;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint32_t time_sec = 0;
uint8_t toggle=0;  //test

uint8_t AMPM_Change = 0;
uint8_t moter_Change = 0;

uint8_t alarm_flag = 0;

uint8_t tmp = 0;

uint32_t Moter14_PWM_Leval = 0;
uint32_t Moter23_PWM_Leval = 0;
uint32_t view_MoterPower14 =0;
uint32_t view_MoterPower23 =0;

uint8_t testNum = 0;
uint8_t testNum2 = 7;
uint8_t testNum3 = 0;

uint8_t MoterIOset = 0;

uint16_t adcval[1]; // adc1 setting
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//uint8_t rx3_data; // UART

//-----------------------------RTC Input code-------------------------------------------------//
RTC_TimeTypeDef sTime;  //set
RTC_TimeTypeDef aTime; //alarm
//RTC_InitTypeDef sInit;//########################333333

RTC_DateTypeDef sDate; //set
RTC_DateTypeDef aDate; //alarm


//-------------------------------------------------------------------------------------------//


char moter_state[2][3] = {"on", "off"};
char ampm[2][3] = {"AM", "PM"}; //2차원 문자 배열// AM = 0, PM = 1 ?

_SetTime set_time = ZERO; //setmode 0
_MODE mode=NO_SET;  //mode set
_VIEW view=VIEW_NORMAL;
_ALARMCALL alarmcall=ALARM_ZERO;
_SetAlarm setalarm =aMONTH;
_SW_BUTT0N sw_button=NONE;
_SW_BUTT0N direction;
_SW_BUTT0N sw_button_before=NONE;

D_MONTHS d_months = d_setmonths;

_SWITCH_VIEWMODE switch_viewmode = switch_normal;
//-------------------------------------ctrl c+v-----https://eteo.tistory.com/91----------------//
uint32_t old_tick=0;
uint32_t current_tick=0;
uint32_t current_BtnTick=0;

uint32_t old_alarm_tick=0;
uint32_t current_alarm_tick=0;

uint32_t btn_tick=0;

uint8_t alarm_on=0;

uint32_t abuzzer_tick1 = 0;
uint32_t abuzzer_tick2 = 0;
uint32_t abuzzer_tick3 = 0;
uint32_t abuzzer_tick4 = 0;

uint8_t prealarm_state = 0;
uint8_t alarm_state = 0;

int alarmCalled = 0;
int water_change_complete = 0;
int water_change_complete_buzzer = 0;


uint8_t for_test = 0;

//--------------------------------------btn on/off check
typedef struct{
   uint8_t ch;
   uint8_t state;
   uint32_t repeat_time;
   uint32_t pre_time;
}button_obj_t;

typedef struct{
   uint8_t Year;
   uint8_t Months;
   uint8_t Date;
   uint8_t Hour;
   uint8_t Min;
   uint8_t Sec;
}set_daytime;
//suho begin-----------------------------------------//
typedef struct{
   uint8_t aMonths;
   uint8_t aDate;
   uint8_t aHour;
   uint8_t aMin;
}set_alarmtime;
//suho end------------------------------------------//
typedef struct{
   uint8_t m_Year;
   uint8_t m_Months;
   uint8_t m_Date;
   uint8_t m_Hour;
   uint8_t m_Min;
   uint8_t m_Sec;
}set_M_time;

enum notes{ // arr = 168
   do4 = 1915, //261hz
   re4 = 1706, //293,
   mi4 = 1519,//329,
   fa4 = 1432,//349,
   sol4 =1278,//391,
   la4 = 1136,//440,
   ti4 = 1014,//493,
};

typedef struct{
   uint32_t MoterPower14;
   uint32_t MoterPower23;
   bool MoterStart23;
   bool MoterStart14;
   uint8_t PWM_l;
}MoterControl;

//----------------------------InOut System------------//
typedef struct{
	uint8_t Turbidity_1;
	uint8_t AlarmSet_1;
	uint8_t MoterOn_PushSW_1;
}MoterInOutSystem;
//------------------------------------------------------------------------------------------//

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void getBtn();
void move_cur_time(set_M_time *m_time, _SW_BUTT0N  sw_button);
//--------------------------------------------------------------------------btn on/off check
void buttonObjCreate(button_obj_t *p_obj, uint32_t repeat_time);
bool buttonObjGetClicked(button_obj_t *p_obj, uint32_t pressed_time);

void dayDateCreate(set_daytime *t_obj, uint16_t t_year, uint16_t t_months, uint16_t t_date);
void daytimeCreate(set_daytime *t_obj, uint32_t t_hour, uint32_t t_min, uint32_t t_sec);
void daytimeview(set_daytime *t_obj);

void AlarmDateCreate(set_alarmtime *t_obj, uint16_t t_months, uint16_t t_date);
void AlarmtimeCreate(set_alarmtime *t_obj, uint32_t t_hour, uint32_t t_min);

void Set_Move_time(set_daytime *t_obj, set_M_time *m_time);
void Load_timeSet(set_daytime *t_obj, set_M_time *m_time);

void Set_Move_alarm(set_alarmtime *t_obj, set_M_time *m_time);//$$$$$$$$$$$$$$$$$$$$$$
void Load_alarmSet(set_alarmtime *t_obj, set_M_time *m_time);

void MoterCreate(MoterControl *M_obj);
void MoterStart1(MoterControl *M_obj);
void MoterMoving(MoterControl *M_obj);

void ChangeMstate14F(MoterControl *M_obj);
void ChangeMstate14T(MoterControl *M_obj);
void ChangeMstate23F(MoterControl *M_obj);
void ChangeMstate23T(MoterControl *M_obj);

int sound_start(int arr[], int len, int giye, int number, int *play_condition);

void MoterSystemCreate(MoterInOutSystem *MIO_S);

void WaterIOS_PushSw(MoterInOutSystem *MIO_S);
void WaterIOS_Turbidity(MoterInOutSystem *MIO_S);
void WaterIOS_AlarmSet(MoterInOutSystem *MIO_S);

void WaterInOutsystem(MoterInOutSystem *MIO_S, MoterControl *M_obj); // senser, alram, direct SW
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//------------------------------------------------------------uart??------------------------//
/*int _write(int file, char* p, int len)
{
   HAL_UART_Transmit(&huart3,  p,  len, 10);
   return len;
}
*/
//-------------------------------------------------------------------------------------------//

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
   //volatile uint16_t adcval[4];  // adc control ?
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
  MX_USART3_UART_Init();
  MX_TIM7_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM10_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM13_Init();
  MX_TIM14_Init();
  MX_TIM6_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  //---------------------------------------------------------right led --//use PMW -> TIM3_CH1,2,3-----//
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
  //---------------------------------------------------------left led---//use PMW -> TIM4_CH1,2,3-----//
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
  //----------------------------------------------------------------------------------------------//
    //HAL_UART_Receive_IT(&huart3, &rx3_data, 1);  //lesson 2-3 make uart Intterupt
    HAL_TIM_Base_Start_IT(&htim7);// system clock?--------------------------TIM7----1.0s------------//

    HAL_TIM_Base_Start_IT(&htim6);
    HAL_TIM_Base_Start_IT(&htim13);
    HAL_TIM_Base_Start_IT(&htim14);
    //---------------------CLCD Setting--------------------------------------------------------//
    CLCD_GPIO_Init();
    CLCD_Init();
    //------------------------------------------------------------------------------------------//
    //CLCD_Puts(0,0, "Welcome to");
    //CLCD_Puts(0,1, "M-HIVE");
    CLCD_Clear();

    _7SEG_GPIO_Init();// _7SEG_GPIO Setting-----------------------------------------------//

 //-------------------------------------------led PWM Control---------if use this, go down "TIM3,4 Interrupt"------//
    /*
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  //R led
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);  //L led
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
    */
 //-----------------------------------------------------------------------------------------------------------//
    //HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); //buzzer
     //HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);  // dc moter
     //HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4); // dc moter

    //HAL_ADC_Start_DMA(&hadc1, adcval[0], 4);
    //HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
    HAL_ADC_Start_DMA(&hadc1, &adcval[0], 1);
 //----------------------------------new time code start-------------------------------------------------//
    button_obj_t push_btn; //+++++++
    buttonObjCreate(&push_btn, 1000); // (button_obj_t *p_obj, uint32_t repeat_time);@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    set_daytime d_date;
    dayDateCreate(&d_date, 24, 12, 31);
    daytimeCreate(&d_date, 23, 59, 58);

    set_alarmtime a_date; //suho
    AlarmDateCreate(&a_date, 01, 02);
    AlarmtimeCreate(&a_date, 03, 00);

   MoterControl m_con;
   MoterCreate(&m_con);

	MoterInOutSystem MIOS;
	MoterSystemCreate(&MIOS);
    //=========================================================rebuild timer========//
    set_M_time M_Time;
//------------------------------------------------------------------------------------------------//
    uint8_t alarm=1;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    uint8_t str[20]; // used CLCD
    uint8_t tmpDate[20]; // CLCD
    uint8_t tmpTime[20]; // CLCD
    uint8_t tmpMode[20];
    uint8_t tmpCLCD[20];

    uint8_t pretmp = 1;
    //uint8_t arr = TIM2 -> ARR;

    enum notes A[] = {mi4, re4, do4, re4, mi4, mi4, mi4, re4, re4, re4, mi4, mi4, mi4};
    int notes_num[] = {2,7,1,7,0,7,1,7,2,7,2,7,2,7,1,7,1,7,1,7,2,7,2,7,2,8};
    int aa = sizeof(notes_num)/sizeof(int);
    int test_num = 0;

    //uint16_t ccr = 0;   //led pwm control
    //uint16_t psc = 1000; //??
    //uint8_t ud_flag = 0; //??
    //uint16_t dacval = 0;//dac control

    uint8_t alarm_from_nowon = 0;
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$44//
    uint8_t old_Sec = d_date.Sec;
    uint8_t k =0;

    int l = 0;
    int j = notes_num[l];
    int na = 0;
    uint16_t turbidity = adcval[0]; // turbidity value in
  while (1)
  {
	  MoterStart1(&m_con); //moter14 and 23 start/stop
	  MoterMoving(&m_con);// moter power level up/down
	  current_BtnTick=HAL_GetTick();// need buttonObjGetClicked
	  daytimeview(&d_date);// timer starting
//--switch changing code test start---------------------------------------------------------------//
	  switch (switch_viewmode) { // was view and mode change
	  case switch_normal :
//case 0=switch_normal================================================================//
		  sprintf(tmpDate, "TIME 20%02d.%02d.%02d ", d_date.Year, d_date.Months, d_date.Date); //CLCD 16
		  CLCD_Puts(0,0, tmpDate);
		  sprintf(tmpTime, "       %02d:%02d    ", d_date.Hour, d_date.Min); // [7], 5, [4] = 16
		  CLCD_Puts(0,1, tmpTime);

		  if(old_Sec != d_date.Sec){ //_7SEG_led *
			  _7SEG_SetNumber(DGT1, d_date.Sec/10, OFF);
			  _7SEG_SetNumber(DGT2, d_date.Sec%10, ON);
			  old_Sec = d_date.Sec;
		  }

//-----------------------------------------------------btn on/off check start------------------------------------//
		  getBtn();// What Btn?
		  if(buttonObjGetClicked(&push_btn, 50) == true) //pressed_time
		  {
			  move_cur_time(&M_Time, sw_button);
		  }
//-----------------------------------------------------btn on/off check end------------------------------------//
		  if(sw_button==SW_ONE){// butten push long ,
			  current_tick=HAL_GetTick();// new time set Continue
			  if(current_tick-old_tick > 2000){// 2.0s over
				old_tick=current_tick;//old_tick Time reset
				Load_timeSet(&d_date, &M_Time);
				sw_button=NONE;

				switch_viewmode = switch_normalTimeSet; // = case 1
				}
		  }
		  break;

	  case switch_normalTimeSet :
//1=switch_normalTimeSet================================================================//
		getBtn();
		if(buttonObjGetClicked(&push_btn, 50) == true) //pressed_time
		{
		   move_cur_time(&M_Time, sw_button);
		}
// -----set blinking-----------------------------------------------//
		if(toggle){ // toggle is 0.5s -> 0 or 1 change
		  sprintf(tmpDate, "TIME 20%02d.%02d.%02d ", M_Time.m_Year, M_Time.m_Months, M_Time.m_Date); //CLCD 16
		  CLCD_Puts(0,0, tmpDate);
		  sprintf(tmpTime, "SET %s %02d:%02d    ", ampm[AMPM_Change], M_Time.m_Hour, M_Time.m_Min); // 12, [4] = 16
		  CLCD_Puts(0,1, tmpTime);

		}else{ //toggle = 0
			if(set_time==YEAR){
				sprintf(tmpDate,"TIME 20  .%02d.%02d ", M_Time.m_Months, M_Time.m_Date); //YEAR place [ ]
			}else if(set_time==MONTH){
				sprintf(tmpDate,"TIME 20%02d.  .%02d ", M_Time.m_Year, M_Time.m_Date);
			}else if(set_time==DATE){
				sprintf(tmpDate,"TIME 20%02d.%02d.   ", M_Time.m_Year, M_Time.m_Months);
			}else if(set_time==AMPM){
				sprintf(tmpTime,"SET    %02d:%02d    ", M_Time.m_Hour, M_Time.m_Min);
			}else if(set_time==HOUR){
				sprintf(tmpTime,"SET %s   :%02d    ", ampm[AMPM_Change], M_Time.m_Min);
			}else if(set_time==MINUTE){
				sprintf(tmpTime,"SET %s %02d:      ", ampm[AMPM_Change], M_Time.m_Hour);
			}
		}
		CLCD_Puts(0,0, tmpDate);
        CLCD_Puts(0,1, tmpTime);
//---------------------------------------------blinking END-------------------------------------------------//
        if(old_Sec != d_date.Sec){
			 _7SEG_SetNumber(DGT1, d_date.Sec/10, OFF);
			 _7SEG_SetNumber(DGT2, d_date.Sec%10, ON);
			 old_Sec = d_date.Sec;
        }

        if(sw_button==SW_ONE){// butten push long ,
        	current_tick=HAL_GetTick();// new time set Continue
        	if(current_tick-old_tick > 2000){// 2.0s over
			  old_tick=current_tick;//old_tick Time reset
			  Set_Move_time(&d_date, &M_Time); // d_date <- m_Time
			  CLCD_Clear();
			  switch_viewmode = switch_normal; // = case 0
        	}
        }
        break;
	  case switch_alarm :
//2=switch_alarm================================================================//
	         if(alarmcall==ALARM_ZERO){ // how many alarms? one, two
	            sprintf(tmpMode,"ALARM%d OFF %02d.%02d",alarm, a_date.aMonths, a_date.aDate);
	            CLCD_Puts(0,0, tmpMode);
	            sprintf(tmpTime,"A    %02d:%02d    ", a_date.aHour, a_date.aMin);
	            CLCD_Puts(0,1, tmpTime);
	         }else if(alarmcall==ALARM_ONE){ // is alarm setting after view
	            sprintf(tmpMode,"ALARM%d ON  %02d.%02d",alarm, a_date.aMonths, a_date.aDate);
	            CLCD_Puts(0,0, tmpMode);
	            sprintf(tmpTime,"A    %02d:%02d    ", a_date.aHour, a_date.aMin);
	            CLCD_Puts(0,1, tmpTime);
	         }
	         getBtn();// What Btn?
	         if(buttonObjGetClicked(&push_btn, 50) == true)
	         {
	              move_cur_time(&M_Time, sw_button);
	         }

	           if(sw_button==SW_ONE){// butten push long ,
	              current_tick=HAL_GetTick();// new time set Continue
	            if(current_tick-old_tick > 2000){// 2.0s over
	               old_tick=current_tick;//old_tick Time reset
	               Load_alarmSet(&a_date, &M_Time);
	               switch_viewmode = switch_alarmTimeSet; // = case 3
	            }
	         }
		  break;
	  case switch_alarmTimeSet :
//3=switch_alarmTimeSet================================================================//
	  getBtn();
      if(buttonObjGetClicked(&push_btn, 50) == true) //pressed_time
        {
           move_cur_time(&M_Time, sw_button);
        }
//---------------------------------------------------------------------------------------//
        if(toggle){//blink_time
           sprintf(tmpMode,"ALARM%d OFF %02d.%02d",alarm, M_Time.m_Months, M_Time.m_Date);
         CLCD_Puts(0,0, tmpMode);
         sprintf(tmpTime,"SET  %02d:%02d    ", M_Time.m_Hour, M_Time.m_Min);
         CLCD_Puts(0,1, tmpTime);
        }else{ //toggle = 0
			 if(setalarm==aMONTH){
				sprintf(tmpMode,"ALARM%d OFF   .%02d", alarm, M_Time.m_Date);
			 }else if(setalarm==aDATE){
				sprintf(tmpMode,"ALARM%d OFF %02d.  ", alarm, M_Time.m_Months);
			 }else if(setalarm==aHOUR){
				sprintf(tmpTime,"SET    :%02d    ", M_Time.m_Min);
			 }else if(setalarm==aMINUTE){
				sprintf(tmpTime,"SET  %02d:      ", M_Time.m_Hour);
			 }
        }
          CLCD_Puts(0,0, tmpMode);
          CLCD_Puts(0,1, tmpTime);
//---------------------------------------------------------------------------------------//
        if(old_Sec != d_date.Sec){
           _7SEG_SetNumber(DGT1, d_date.Sec/10, OFF);
           _7SEG_SetNumber(DGT2, d_date.Sec%10, ON);
           old_Sec = d_date.Sec;
        }
//---------------------------------------------------------------------------------------//
          if(sw_button==SW_ONE){
             current_tick=HAL_GetTick();
             if(current_tick-old_tick > 2000){ // if pressed 1.0s over
                old_tick=current_tick;
                Set_Move_alarm(&a_date, &M_Time);
              alarmcall=ALARM_ONE;
              prealarm_state = 0; // just 1 music
              switch_viewmode = switch_alarm; // = case 2
              CLCD_Clear();
             }
          }
		  break;
	  case switch_test :
//4=switch_test================================================================//
		  getBtn();// What Btn?
		   if(buttonObjGetClicked(&push_btn, 50) == true)
		   {
				move_cur_time(&M_Time, sw_button);
		   }

	        if(old_Sec != d_date.Sec){
	           _7SEG_SetNumber(DGT1, d_date.Sec/10, OFF);
	           _7SEG_SetNumber(DGT2, d_date.Sec%10, ON);
	           old_Sec = d_date.Sec;
	        }

	        if(testNum3 == 1){
	        	WaterIOS_PushSw(&MIOS);
	        	MoterIOset = 5; // 5 is motersystem start setting
	        }

//		  sprintf(tmpDate,"    %d %d ", turbidity, testNum2);
//		  CLCD_Puts(0,0, tmpDate);
	        sprintf(str,"%d    %d  ", turbidity, testNum2);
	        CLCD_Puts(0,0, str);
//	        sprintf(str,"%d   ", testNum3);
//	        CLCD_Puts(0,1, str);
	        break;

	  case switch_viewchanging :
//5=switch_viewchanging================================================================//
			 getBtn();// What Btn?
			 abuzzer_tick4 = HAL_GetTick();
			 if(abuzzer_tick4 - abuzzer_tick3 == 3000){
				 switch_viewmode = switch_normal; // = case 0
			 }
		  break;
	  }
//----switch changing code test end- 2024-05-09------------------------------------------------------------------//

//-alarm time == date time -> alarm start--------------------------------------------------------------------//
//       if(a_date.aMonths == d_date.Months &&
//             a_date.aDate == d_date.Date&&
//           a_date.aHour == d_date.Hour&&
//           a_date.aMin == d_date.Min &&
//           alarm_state == prealarm_state){// if alarm_state = 0, alarm on and prealarm_state go to 0 -> 3
//         CLCD_Clear();
//          alarm_on = 1;
//        prealarm_state =3;
//        abuzzer_tick1 = HAL_GetTick(); // keep going
//        abuzzer_tick2 = HAL_GetTick(); // 5s ckecking and 0 to 5
//        abuzzer_tick3 = HAL_GetTick();
//
//          sprintf(tmpCLCD, "changing water");
//          CLCD_Puts(0, 0, tmpCLCD);
//
//          HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
//
//          WaterIOS_AlarmSet(&MIOS);
//          MoterIOset = 5; // 5 is motersystem start setting
//       }
//
////----------------------------------------------------------------------------------------------//
//	  if(alarm_on == 1){
//		  //l -> 0,1,2,3,4~ 25// IF l= 25, no sound and l = 0;
//		  l = sound_start(notes_num, aa, notes_num[l], l, &alarm_on);
//		  // array of melody, len of array, giye name, up count
//	  }

 //---------------------------------------------------------------------------------------//

	  if(turbidity > 3500) {
		  if (MoterIOset == 0) {
//			  sprintf(str,"%d   ", testNum3);
//			  CLCD_Puts(0,1, str);
			  sprintf(str,"%d    ", turbidity);
			  CLCD_Puts(0,0, str);
			  WaterIOS_Turbidity(&MIOS);
			  		  MoterIOset = 5;
		  }
	  }
//	 if (turbidity < 3500) {
		//ChangeMstate14T(&m_con); //water out start
		//OutMotor = 1;
//	 WaterIOS_Turbidity(&MIOS);
//	 }

//	 if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) { // set = 1(LED on), reset = 0
//		ChangeMstate14F(&m_con); //water out stop
//		ChangeMstate23T(&m_con); //water in start
//		//OutMotor = 0;
//		//InMotor = 1;
//	 }
//	 if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_SET) {
//		ChangeMstate23F(&m_con); //water in stop
//		//InMotor = 0;
//	 }

//	  WaterInOutsystem(&MIOS, &m_con);
//****************************************END WHILE****************************--------------move code CV end---------------//

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }//WHILE END
    //========================================================================================================//
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* TIM7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM7_IRQn);
  /* EXTI15_10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  /* EXTI4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  /* EXTI3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
  /* TIM8_TRG_COM_TIM14_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
  /* TIM8_UP_TIM13_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
  /* TIM6_DAC_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

/* USER CODE BEGIN 4 */
//--------------------------------------------------------------------rebuild timer----------------//
void dayDateCreate(set_daytime *t_obj, uint16_t t_year, uint16_t t_months, uint16_t t_date)
{
   t_obj -> Year = t_year;

   d_months = t_months;
   t_obj -> Months = d_months;

   t_obj -> Date = t_date;
}

void daytimeCreate(set_daytime *t_obj, uint32_t t_hour, uint32_t t_min, uint32_t t_sec)
{
   t_obj -> Hour = t_hour;
   t_obj -> Min = t_min;

   time_sec = t_sec;
   //t_obj -> sec = time_sec;
}

void AlarmDateCreate(set_alarmtime *t_obj, uint16_t t_months, uint16_t t_date)
{
   t_obj -> aMonths = t_months;
   t_obj -> aDate = t_date;
}

void AlarmtimeCreate(set_alarmtime *t_obj, uint32_t t_hour, uint32_t t_min)
{
   t_obj -> aHour = t_hour;
   t_obj -> aMin = t_min;

   //time_sec = t_sec;
   //t_obj -> sec = time_sec;
}


void daytimeview(set_daytime *t_obj) // timer starting
{
   t_obj -> Sec = time_sec;
   if(t_obj -> Sec >= 60){
      t_obj -> Sec = 0;
      time_sec=0;
      t_obj -> Min++;
   }
   if(t_obj -> Min >= 60){
      t_obj -> Min = 0;
      t_obj -> Hour++;
   }
   if(t_obj -> Hour > 23){
      t_obj -> Hour = 0;
      t_obj -> Date++;
   }
   if(t_obj ->Months ==1||t_obj ->Months ==3||t_obj ->Months ==5||t_obj ->Months ==7||t_obj ->Months ==8||t_obj ->Months ==10||t_obj ->Months ==12){
      if(t_obj -> Date > 31){
         t_obj -> Date = 1;
         d_months++;
         if(d_months > 12){
            d_months = 1;
            t_obj ->Year++;
         }
      }

   }else if(t_obj ->Months ==4||t_obj ->Months ==6||t_obj ->Months ==9||t_obj ->Months ==11){
      if(t_obj -> Date > 30){
         t_obj -> Date = 1;
         d_months++;
      }
   }else if(t_obj ->Months ==2){
      if(t_obj ->Date > 28){
         t_obj -> Date = 1;
         d_months++;
      }
   }
   t_obj ->Months = d_months;

}

void Set_Move_time(set_daytime *t_obj, set_M_time *m_time)
{
   t_obj -> Year = m_time -> m_Year;
   t_obj -> Months = m_time -> m_Months;
   t_obj -> Date = m_time -> m_Date;
   t_obj -> Hour = m_time -> m_Hour;
   t_obj -> Min = m_time -> m_Min;
}

void Load_timeSet(set_daytime *t_obj, set_M_time *m_time)
{
   m_time -> m_Year = t_obj -> Year;
   m_time -> m_Months= t_obj -> Months;
   m_time -> m_Date = t_obj -> Date;
   m_time -> m_Hour = t_obj -> Hour;
   m_time -> m_Min = t_obj -> Min;
}

void Set_Move_alarm(set_alarmtime *t_obj, set_M_time *m_time) // suho
{
   //t_obj -> aYear = m_time -> m_Year;
   t_obj -> aMonths = m_time -> m_Months;
   t_obj -> aDate = m_time -> m_Date;
   t_obj -> aHour = m_time -> m_Hour;
   t_obj -> aMin = m_time -> m_Min;
}

void Load_alarmSet(set_alarmtime *t_obj, set_M_time *m_time)
{
   //m_time -> m_Year = t_obj -> aYear;
   m_time -> m_Months= t_obj -> aMonths;
   m_time -> m_Date = t_obj -> aDate;
   m_time -> m_Hour = t_obj -> aHour;
   m_time -> m_Min = t_obj -> aMin;
}
//--------------------------------------------------------------------Btn on/off check---------------//
void buttonObjCreate(button_obj_t *p_obj, uint32_t repeat_time)
{
   //p_obj -> ch = ch;
   p_obj -> pre_time = current_BtnTick;
   p_obj -> state = 0;
   p_obj -> repeat_time = repeat_time;
}
bool buttonObjGetClicked(button_obj_t *p_obj, uint32_t pressed_time)//push_btn, 100
{
   bool ret = false;

   switch(p_obj->state)
   {
   case 0:
      if(sw_button==SW_ONE||sw_button==SW_TWO||sw_button==SW_THREE||sw_button==SW_FOUR)
      {
         p_obj->state = 1;
         p_obj->pre_time = current_BtnTick;// = Gettick;
      }else{
         //HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
      }
      break;

   case 1:
      if(sw_button==SW_ONE||sw_button==SW_TWO||sw_button==SW_THREE||sw_button==SW_FOUR)
      {
         if(current_BtnTick-p_obj->pre_time >=pressed_time){//pressed_time
            ret = true; //------------------------------------clicked

            p_obj->state = 2;
            p_obj->pre_time = current_BtnTick; //
            //HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); //buzzer
         }
         //btn_tick = current_tick;

      }
      else
      {
         p_obj->state = 0;
      }
      break;

   case 2:
      if(sw_button==SW_ONE||sw_button==SW_TWO||sw_button==SW_THREE||sw_button==SW_FOUR)
      {
         if(current_BtnTick-p_obj->pre_time >= p_obj->repeat_time) //
         {
            p_obj->state = 1;
            p_obj->pre_time = current_BtnTick;
         }

      }
      else
      {
         p_obj->state = 0;
      }
      break;
   }

   return ret;
}
//=----------------------------my---------------------------------------code butten------//
void getBtn(){
   if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)){
      sw_button=SW_ONE;
   }else if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15)){
      sw_button=SW_TWO;
   }else if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4)){
      sw_button=SW_THREE;
   }else if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10)){
      sw_button=SW_FOUR;
   }else{
      sw_button=NONE;
   }
}

//-----sound function begin----//

// array of melody, n = 0, i = array_of_melody[n]
int sound_start(int arr[], int len, int giye, int number, int *play_condition){
   switch(giye){
       case 0 :
          TIM2 -> PSC = 1915; // do
           abuzzer_tick2 = HAL_GetTick();

           if(abuzzer_tick2-abuzzer_tick1 >500){
              number++;
               giye=arr[number];
               abuzzer_tick1 = abuzzer_tick2;
           }break;

       case 1 :
          TIM2 -> PSC = 1706; //re
          abuzzer_tick2 = HAL_GetTick();

          if(abuzzer_tick2-abuzzer_tick1 >500){
             number++;
             giye=arr[number];
             abuzzer_tick1 = abuzzer_tick2;
          }break;

       case 2 :
          TIM2 -> PSC = 1519; //mi
          abuzzer_tick2 = HAL_GetTick();

          if(abuzzer_tick2-abuzzer_tick1 >500){
             number++;
             giye=arr[number];
             abuzzer_tick1 = abuzzer_tick2;
          }break;

       case 3 :
          TIM2 -> PSC = 1432; //fa
          abuzzer_tick2 = HAL_GetTick();

          if(abuzzer_tick2-abuzzer_tick1 >500){
             number++;
             giye=arr[number];
             abuzzer_tick1 = abuzzer_tick2;
          }break;

       case 4 :
          TIM2 -> PSC = 1278; //sol
          abuzzer_tick2 = HAL_GetTick();

          if(abuzzer_tick2-abuzzer_tick1 >500){
             number++;
             giye=arr[number];
             abuzzer_tick1 = abuzzer_tick2;
          }break;

       case 5 :
          TIM2 -> PSC = 1136; //la
          abuzzer_tick2 = HAL_GetTick();

          if(abuzzer_tick2-abuzzer_tick1 >500){
             number++;
             giye=arr[number];
             abuzzer_tick1 = abuzzer_tick2;
          }break;

       case 6 :
          TIM2 -> PSC = 1014; //ti
          abuzzer_tick2 = HAL_GetTick();

          if(abuzzer_tick2-abuzzer_tick1 >500){
             number++;
             giye=arr[number];
             abuzzer_tick1 = abuzzer_tick2;
          }break;

       case 7 :
          HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1); //not sound
          abuzzer_tick2 = HAL_GetTick();

          if(abuzzer_tick2-abuzzer_tick1 >100){
             HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
             number++;
             giye=arr[number];
             abuzzer_tick1 = abuzzer_tick2;
          }break;

       case 8 :
          number = 0;
            //prealarm_state = 1;
            HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
            *play_condition = 0; // sound finish

            abuzzer_tick1 == 0;
            abuzzer_tick2 == 0;
	} // switch end
   return number;
}//void funtion end

//-----sound function end----//
//-------------------------------Water inOutsystem-Start----------------//
void MoterSystemCreate(MoterInOutSystem *MIO_S)
{
	MIO_S -> Turbidity_1 = 0;
	MIO_S -> AlarmSet_1 = 0;
	MIO_S -> MoterOn_PushSW_1 = 0;
}
void WaterIOS_PushSw(MoterInOutSystem *MIO_S)
{
	MIO_S -> MoterOn_PushSW_1 = 1;
}
void WaterIOS_Turbidity(MoterInOutSystem *MIO_S)
{
	MIO_S -> Turbidity_1 = 1;
}
void WaterIOS_AlarmSet(MoterInOutSystem *MIO_S)
{
	MIO_S -> AlarmSet_1 = 1;
}

void WaterInOutsystem(MoterInOutSystem *MIO_S, MoterControl *M_obj) // senser, alram, direct SW
{
	switch(MoterIOset){
	case 5 :
		testNum2 = 5;
		if(MIO_S -> MoterOn_PushSW_1 == 1 || MIO_S -> Turbidity_1 == 1 || MIO_S -> AlarmSet_1 == 1){ // push butten to waterChange
				MoterIOset = 1;
				testNum = 4;
			}
		break;
	case 1 : //Out Moter ON
		testNum2 = 1;
		//ChangeMstate23T(&m_con);
		M_obj -> MoterStart23 = true; // is Out Moter

		MoterIOset = 2;
		MIO_S -> MoterOn_PushSW_1 = 0;
		MIO_S -> Turbidity_1 = 0;
		MIO_S -> AlarmSet_1 = 0;
		break;
	case 2 : //Out Moter Off, In Moter On
		testNum2 = 2;
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) { // set = 1(LED on), reset = 0 //down
			//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);//led 0n
			//ChangeMstate23F(&m_con); //water out stop
			M_obj -> MoterStart23 = false;
			//ChangeMstate14T(&m_con); //water in start
			M_obj -> MoterStart14 = true;
			MoterIOset = 3;
			}
				//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);//led off
		break;
	case 3 : // In Moter off, #if inWater not go case 4
		testNum2 = 3;
		//------END MoterChange, reset-------------------------------------------//
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET) { //up
			//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);//led 0n
			//ChangeMstate14F(&m_con); //water in stop
			M_obj -> MoterStart14 = false;
			MoterIOset = 0;

		// end MoterChange, reset-------------------------------------------------//

		//#if inWater not, go case 4
		}else{
			//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);//led 0n
		}

		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5) == GPIO_PIN_SET) {
			//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);//led 0n
			//ChangeMstate14F(&m_con); //water in start
			M_obj -> MoterStart14 = false;
			MoterIOset = 4;
		}else{
			//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);//led OFF
		}
		break;
	case 4 : //#if inWater is, go case 3
		testNum2 = 4;
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5) == GPIO_PIN_RESET) {
			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);//led OFF
			//ChangeMstate14T(&m_con); //water in start
			M_obj -> MoterStart14 = true;
			MoterIOset = 3;
		}else{
			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);//led 0n
		}
		break;
	}
}

//-------------------------------Water inOutsystem-End----------------//
void move_cur_time(set_M_time *m_time, _SW_BUTT0N  sw_button){
      switch(sw_button){
      case SW_ONE:
         //if(mode==TIME_SET)
		 if(switch_viewmode == switch_normalTimeSet){
            set_time++;
            if(set_time > SECOND) set_time = YEAR;
         }
		 //else if(mode==ALARM_SET)
		 else if(switch_viewmode == switch_alarmTimeSet)
		 {
            setalarm++;
            if(setalarm > aMINUTE) setalarm = aMONTH;
         }

         //if(view == VIEW_TEST)
		 if(switch_viewmode == switch_test)
         {
			testNum3++;
            if(testNum3 > 4)
            {testNum3 = 0;}
         }
         break;

      case SW_TWO:
         //if(mode==TIME_SET)
    	 if(switch_viewmode ==switch_normalTimeSet)
         {
            if(set_time==AMPM){
               AMPM_Change ^= 1;
            }else if(set_time==HOUR){
               m_time -> m_Hour++;
               if(m_time -> m_Hour >23) m_time -> m_Hour = 0;
            }else if(set_time==MINUTE){
               m_time -> m_Min++;
               if(m_time -> m_Min >= 60) m_time -> m_Min = 0;
            }else if(set_time==YEAR){
               m_time ->m_Year++;
               if(m_time ->m_Year > 99) m_time ->m_Year = 0;
            }else if(set_time==MONTH){
               d_months++;
               if(d_months > 12) d_months = 1;
               m_time ->m_Months = d_months;
            }else if(set_time==DATE){
               m_time -> m_Date++;
               if(m_time -> m_Date > 31) m_time -> m_Date = 1;  //%%%%%%%%%% error error error=========  !!!!!!!!!!!!!==============//
            }
         }

         //if(mode==ALARM_SET)
    	 if(switch_viewmode == switch_alarmTimeSet)
         {
            if(setalarm==aHOUR){
               m_time -> m_Hour++;
               if(m_time -> m_Hour >23) m_time -> m_Hour = 0;
            }else if(setalarm==aMINUTE){
               m_time -> m_Min++;
               if(m_time -> m_Min >= 60) m_time -> m_Min = 0;
            }else if(setalarm==aMONTH){
               //d_months++;
               m_time -> m_Months++;
               //if(d_months > 12) d_months = 1;
               if(m_time -> m_Months > 12) m_time -> m_Months = 1;
               //m_time ->m_Months = d_months;
            }else if(setalarm==aDATE){
               m_time -> m_Date++;
               if(m_time -> m_Date > 31) m_time -> m_Date = 1;  //%%%%%%%%%% error error error=========  !!!!!!!!!!!!!==============//
            }
         }
         break;

      case SW_THREE:
         //if(mode==TIME_SET)
    	  if(switch_viewmode ==switch_normalTimeSet)
         {
            if(set_time==AMPM){
               AMPM_Change ^= 1;
            }else if(set_time==HOUR){
               m_time -> m_Hour--;
               if(!(m_time -> m_Hour >= 0 && m_time -> m_Hour < 24)) m_time -> m_Hour = 23;
            }else if(set_time==MINUTE){
               m_time -> m_Min--;
               if(!(m_time -> m_Min >= 0 && m_time -> m_Min < 60)) m_time -> m_Min = 59;
            }else if(set_time==YEAR){
               m_time ->m_Year--;
               if(!(m_time ->m_Year >= 0&& m_time ->m_Year <= 99)) m_time ->m_Year = 99;
            }else if(set_time==MONTH){
               d_months--;
               if(d_months < 1) d_months = 12;
               m_time ->m_Months = d_months;
            }else if(set_time==DATE){
               m_time -> m_Date--;
               if(m_time -> m_Date < 1) m_time -> m_Date = 31;  //%%%%%%%%%% error error error=========  !!!!!!!!!!!!!==============//
            }
         }

         //if(mode==ALARM_SET)
    	  if(switch_viewmode == switch_alarmTimeSet)
         {
            if(setalarm==aHOUR){
               m_time -> m_Hour--;
               if(!(m_time -> m_Hour >= 0 && m_time -> m_Hour < 24)) m_time -> m_Hour = 23;
            }else if(setalarm==aMINUTE){
               m_time -> m_Min--;
               if(!(m_time -> m_Min >= 0 && m_time -> m_Min < 60)) m_time -> m_Min = 59;
            }else if(setalarm==aMONTH){
               //d_months--;
               m_time -> m_Months--;
               //if(d_months < 1) d_months = 12;
               if(!(m_time -> m_Months >=1 && m_time -> m_Months <=12)) m_time -> m_Months = 12;
               //m_time ->m_Months = d_months;
            }else if(setalarm==aDATE){
               m_time -> m_Date--;
               if(m_time -> m_Date < 1) m_time -> m_Date = 31;  //%%%%%%%%%% error error error=========  !!!!!!!!!!!!!==============//
            }
         }
         break;
      case SW_FOUR:
         //if(mode==NO_SET)
//    	 if(switch_viewmode == 0 ||
//    			 switch_viewmode == 1 ||
//				 switch_viewmode == 2 ||
//				 switch_viewmode == 3 ||
//				 switch_viewmode == 4 ||
//				 switch_viewmode == 5)
//         { 														// view , normal, alarm, test
            //view++;
    		if(switch_viewmode == switch_normal){
    			switch_viewmode = switch_alarm;
    		}else if(switch_viewmode == switch_alarm){
				switch_viewmode = switch_test;
			}else if(switch_viewmode == switch_test){
				switch_viewmode = switch_normal;
			}

//            if(view == VIEW_VIEW) view = VIEW_NORMAL;
//            if(view > VIEW_TEST) view = VIEW_NORMAL;

         //}
         break;
      case NONE:
         break;
      }// end switch
      sw_button=NONE;
   }

//---------------------------------moter test----------------//
void MoterCreate(MoterControl *M_obj)
{
   M_obj -> MoterPower14 = 0;
   M_obj -> MoterPower23 = 0;

   M_obj -> MoterStart14 = false;
   M_obj -> MoterStart23 = false;
   M_obj -> PWM_l = 200;
}
void ChangeMstate14F(MoterControl *M_obj){
   M_obj -> MoterStart14 = false;
}
void ChangeMstate14T(MoterControl *M_obj){
   M_obj -> MoterStart14 = true;
}
void ChangeMstate23F(MoterControl *M_obj){
   M_obj -> MoterStart23 = false;
}
void ChangeMstate23T(MoterControl *M_obj){
   M_obj -> MoterStart23 = true;
}
void MoterStart1(MoterControl *M_obj)
{
   if(M_obj -> MoterStart14 == true && M_obj -> MoterPower14 ==0){//M_obj -> MoterPower ==0
      HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);  // dc moter
      HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);  // dc moter

   }else if(M_obj -> MoterStart14 == false  && M_obj -> MoterPower14 ==0){
      HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_1);  // dc moter
      HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_4);  // dc moter

   }
   if(M_obj -> MoterStart23 == true  && M_obj -> MoterPower23 ==0){ //Moter23_PWM_Leval
      HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);  // dc moter
      HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);  // dc moter
      //test_num6 = 10;
   }else if(M_obj -> MoterStart23 == false  && M_obj -> MoterPower23 ==0){
      HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_2);  // dc moter
      HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_3);  // dc moter
      //test_num6 = 20;
   }
}
void MoterMoving(MoterControl *M_obj)
{
   if(M_obj -> MoterStart14 == true && M_obj -> MoterPower14 < 9900){
      M_obj -> MoterPower14 += M_obj -> PWM_l;
      __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, M_obj -> MoterPower14);

   }else if(M_obj ->MoterStart14 == false && M_obj -> MoterPower14 >0){
      M_obj -> MoterPower14 -= M_obj -> PWM_l;
      __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, M_obj -> MoterPower14);
   }

   if(M_obj -> MoterStart23 == true && M_obj -> MoterPower23 < 9900){
      M_obj -> MoterPower23 += M_obj -> PWM_l;
      __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, M_obj -> MoterPower23);

   }else if(M_obj ->MoterStart23 == false && M_obj -> MoterPower23 >0){
      M_obj -> MoterPower23 -= M_obj -> PWM_l;
      __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_2, M_obj -> MoterPower23);
   }
   view_MoterPower14 = M_obj -> MoterPower14;
   view_MoterPower23 = M_obj -> MoterPower23;
}

//_________----------------------------------------moter test end-------//

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
   if(huart->Instance == USART3)
   {
      //HAL_UART_Receive_IT(&huart3,  &rx3_data, 1);
      //HAL_UART_Transmit(&huart3, &rx3_data, 1, 10);
   }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
   static unsigned char cnt = 0;
   uint8_t blink_time=0;
   //static unsigned char blink_cnt = 0;
   if(htim->Instance == TIM7)  //1.0s
   {
      //HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);

      //cnt++;
      //test_num7++;

      //if(alarm_flag == 0)
      //{alarm_flag = 1;}
      //else if(alarm_flag == 1)
      //{alarm_flag = 0;}

   }

   if(htim -> Instance == TIM6) //0.5s
   {
      //test_num6++;
   }

   if(htim -> Instance == TIM13)//1s
   {

   }

   if(htim -> Instance == TIM14)//1s
   {

   }


}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) // if switch in action
{
   old_tick=HAL_GetTick(); // just pressed time
   current_tick=HAL_GetTick();// continue check now time = Current time qudtls

   if(GPIO_Pin == GPIO_PIN_3)  // SW1 mode change
   {
//--------------------------------------------------------------------------------------------------------------------//sw2
   }
   if(GPIO_Pin == GPIO_PIN_15) //SW2 time up // AMPM Change
      {
//--------------------------------------------------------------------------------------------------------------------//sw3
      }
   if(GPIO_Pin == GPIO_PIN_4) //SW3 time down
      {

      }
//--------------------------------------------------------------------------------------------------------------------//sw4
   if(GPIO_Pin == GPIO_PIN_10) //SW4
      {

      }
}
uint32_t HAL_GetTick(void)
{
  return uwTick; //0.001s = 1ms
}
//-======================================================//


void time_inc(void)
{
   static uint32_t _msec = 0;
   static uint32_t _msecP2 = 0;
   _msec++;
   _msecP2++;
   if (_msec >= 1000)
   {
      _msec = 0;
      time_sec++; //1s
      //i++;
   }

   if (_msecP2 >= 500)
   {
      _msecP2 = 0;
      toggle ^= 1;
   }
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
  __disable_irq();
  while (1)
  {
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
