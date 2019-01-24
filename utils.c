#include "utils.h"
#include "misc.h"

static __IO uint32_t _delay;

void rtc_setup(void) {
	// Based on http://www.stm32.eu/node/97 and AN2821

	// Enable PWR and BKP clocks
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	// Enable access to BKP
	PWR_BackupAccessCmd (ENABLE);
	// BKP de-init
	BKP_DeInit();
	// Start editing RTC configuration
	RTC_EnterConfigMode();
	// Enable LSE
	RCC_LSEConfig (RCC_LSE_ON);  // RCC_LSICmd(ENABLE);
	// Wait for LSE
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
	}  // while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {}
	   // RTCCLK = LSE = 32.768 kHz
	RCC_RTCCLKConfig (RCC_RTCCLKSource_LSE); // RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
	// Turn on RTC clock
	RCC_RTCCLKCmd(ENABLE);
	// Wait for synchronization
	RTC_WaitForSynchro();
	// Wait until operation finish
	RTC_WaitForLastTask();
	// Enable RTC second
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	RTC_WaitForLastTask();
	// Set Prescaler to 32768 ticks
	// RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
	RTC_SetPrescaler(32767);
	RTC_WaitForLastTask();
	// Exit configuration editing mode
	RTC_ExitConfigMode();
	// Set RTC counter (time) to 12:00:00
	RTC_SetCounter(12 * 60 * 60);

	// Disable default Tamper Pin
//	BKP_TamperPinCmd (DISABLE);

	// Enable calibration data on Tamper Pin
//    BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);

	// Enable seconds indicator on Tamper Pin
//	BKP_RTCOutputConfig (BKP_RTCOutputSource_Second);

	// Enable RTC interrupts
	NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Interrupt every 1 second
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	RTC_WaitForLastTask();
}

void setup_delay_timer(TIM_TypeDef *timer) {
	TIM_DeInit(timer);
	// Enable Timer clock
	if (timer == TIM2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	} else if (timer == TIM3) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	} else if (timer == TIM4) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	} else {
		// TODO: not implemented
		while (1) {
		}
	}

	// Configure timer
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStructure.TIM_Prescaler = SystemCoreClock / 1000000 - 1;
	TIM_InitStructure.TIM_Period = 10000 - 1; // Update event every 10000 us (10 ms)
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_InitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(timer, &TIM_InitStructure);

	TIM_Cmd(timer, ENABLE);
}

/*void delay(__IO uint32_t nTime)
{
	_delay = nTime;

	while(_delay != 0);
}
*/
void delay_decrement(void) {
	if (_delay--) {
	}
}

/*void delay_us(TIM_TypeDef *timer, unsigned int time) {
	timer->CNT = 0;
	time -= 3;
	while (timer->CNT <= time) {
	}
}

void delay_ms(TIM_TypeDef *timer, unsigned int time) {
	while (time--) {
		delay_us(timer, 1000);
	}
}
*/
// Saturated add functions for 8 / 16 / 32 unsigned integers
inline uint8_t sadd8(uint8_t a, uint8_t b) {
	return (a > 0xFF - b) ? 0xFF : a + b;
}

inline uint16_t sadd16(uint16_t a, uint16_t b) {
	return (a > 0xFFFF - b) ? 0xFFFF : a + b;
}

inline uint32_t sadd32(uint32_t a, uint32_t b) {
	return (a > 0xFFFFFFFF - b) ? 0xFFFFFFFF : a + b;
}
