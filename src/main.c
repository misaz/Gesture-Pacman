#include "MAX25405.h"
#include "BT817.h"
#include "board.h"
#include "GameRenderer.h"
#include "GestureDetect.h"
#include "Game.h"
#include "SplashScreen.h"

#include "mxc.h"
#include "mxc_device.h"
#include "nvic_table.h"
#include "max20303.h"
#include "max20303.c"

#include <string.h>

static Game game;

static int isNewGestureSensorDataAvailable = 0;
static int isTimeToStep = 0;

void MAX25405_HandleInterrupt() {
	MXC_GPIO_ClearFlags(MXC_GPIO0, MXC_GPIO_PIN_11);
	isNewGestureSensorDataAvailable = 1;
}

void MAX25405_InitInterrupt() {
	int status;

	mxc_gpio_cfg_t intLine;
	intLine.port = MXC_GPIO0;
	intLine.mask = MXC_GPIO_PIN_11;
	intLine.func = MXC_GPIO_FUNC_IN;
	intLine.pad = MXC_GPIO_PAD_NONE;
	intLine.vssel = MXC_GPIO_VSSEL_VDDIOH;

	status = MXC_GPIO_Config(&intLine);
	if (status) {
	  	__BKPT(0);
	}

	NVIC_ClearPendingIRQ(GPIO0_IRQn);
	NVIC_SetPriority(GPIO0_IRQn, 0);
	MXC_NVIC_SetVector(GPIO0_IRQn, MAX25405_HandleInterrupt);

	status = MXC_GPIO_IntConfig(&intLine, MXC_GPIO_INT_FALLING);
	if (status) {
	  	__BKPT(0);
	}

	MXC_GPIO_EnableInt(MXC_GPIO0, MXC_GPIO_PIN_11);
}

void Timer_InterruptHandler() {
	isTimeToStep = 1;
    MXC_TMR_ClearFlags(MXC_TMR0);
	MXC_GPIO_OutToggle(MXC_GPIO2, MXC_GPIO_PIN_2);
}

void Timer_Init() {
	int status;

	mxc_gpio_cfg_t led;
	led.port = MXC_GPIO2;
	led.mask = MXC_GPIO_PIN_2;
	led.func = MXC_GPIO_FUNC_OUT;
	led.pad = MXC_GPIO_PAD_NONE;
	led.vssel = MXC_GPIO_VSSEL_VDDIO;

	status = MXC_GPIO_Config(&led);
	if (status) {
	  	__BKPT(0);
	}

	MXC_GPIO_OutSet(MXC_GPIO2, MXC_GPIO_PIN_2);

	NVIC_ClearPendingIRQ(TMR0_IRQn);
	NVIC_SetPriority(TMR0_IRQn, 3);
	NVIC_EnableIRQ(TMR0_IRQn);
	MXC_NVIC_SetVector(TMR0_IRQn, Timer_InterruptHandler);

	mxc_tmr_cfg_t cfg;
	cfg.bitMode = TMR_BIT_MODE_32;
	cfg.clock = MXC_TMR_8M_CLK;
	cfg.mode = TMR_MODE_CONTINUOUS;
	cfg.cmp_cnt = 750000;
	cfg.pol = 0;
	cfg.pres = TMR_PRES_8;

	status = MXC_TMR_Init(MXC_TMR0, &cfg, 0);
	if (status) {
		__BKPT(0);
	}

	MXC_TMR_EnableInt(MXC_TMR0);
	MXC_TMR_Start(MXC_TMR0);
}

int main() {
	int iStatus;
    MAX25405_Status mStatus;
    MAX25405_Device max25405dev;
    int lastGesture;

    // select MAX25405 interface: SPI (SEL pin LOW, HIGH mean I2C)
    mxc_gpio_cfg_t sel;
    sel.port = MXC_GPIO3;
    sel.mask = MXC_GPIO_PIN_1;
    sel.func = MXC_GPIO_FUNC_OUT;
    sel.pad = MXC_GPIO_PAD_NONE;
    sel.vssel = MXC_GPIO_VSSEL_VDDIOH;
    iStatus = MXC_GPIO_Config(&sel);
    if (iStatus) {
    	__BKPT(0);
    }

    mStatus = MAX25405_InitSPI(&max25405dev, 0);
    if (mStatus) {
    	__BKPT(0);
    }

	BT817_Init();

	SplashScreen_Show(8);

    mStatus = MAX25405_Reset(&max25405dev);
    if (mStatus) {
    	__BKPT(0);
    }

    MXC_Delay(10000);

    // waste possible Power on Reset interrupt
	MAX25405_Interrupt ints;
    MAX25405_GetPendingInterrupts(&max25405dev, &ints);

    mxc_gpio_cfg_t btn;
    btn.port = MXC_GPIO2;
    btn.mask = MXC_GPIO_PIN_4;
    btn.func = MXC_GPIO_FUNC_IN;
    btn.pad = MXC_GPIO_PAD_PULL_UP;
    btn.vssel = MXC_GPIO_VSSEL_VDDIOH;
    iStatus = MXC_GPIO_Config(&btn);
    if (iStatus) {
    	__BKPT(0);
    }

    MAX25405_Configuration config;
    MAX25405_GetDefaultConfiguration(&config);
    config.mainConfig.modeOfOperation = MAX25405_ModeOfOperation_TrackingMode;
    config.ledConfig.ledDrive = MAX25405_LedDrive_PWM_16_16;
    config.ledConfig.enableDrivePwmOutput = 1;
    config.ledConfig.columnGainMode = MAX25405_ColumnGainModeSelection_Internal;
    config.mainConfig.enableEndOfConversionInterrupt = 1;
    config.sequencingConfig.endOfConversionDelay = MAX25405_EndOfConversionDelay_3_12ms;
    config.sequencingConfig.integrationTime = MAX25405_IntegrationTime_25us;
    config.sequencingConfig.numberOfCoherentDoubleSamples = MAX25405_NumberOfCoherentDoubleSamples_8;

    mStatus = MAX25405_SetConfiguration(&max25405dev, &config);
    if (mStatus) {
    	__BKPT(0);
    }

    MAX25405_Configuration configCheck;
    mStatus = MAX25405_GetConfiguration(&max25405dev, &configCheck);
    if (mStatus) {
    	__BKPT(0);
    }

    MAX25405_InitInterrupt();

    GestureDetect_Init();
    Timer_Init();

	Game_Init(&game);
	GameRenderer_RenderGame(&game);
	Game_Start(&game);

    while (1) {

    	int btnState = !!MXC_GPIO_InGet(MXC_GPIO2, MXC_GPIO_PIN_4);

    	if (isNewGestureSensorDataAvailable) {
    		isNewGestureSensorDataAvailable = 0;

        	int16_t data[60];

            mStatus = MAX25405_GetAllPixelData(&max25405dev, data);
            if (mStatus) {
            	__BKPT(0);
            }

            lastGesture = GestureDetect_AddDataAndGetGesture(data);

            if (lastGesture != -1) {
            	if (lastGesture == DIRECTION_UP) {
            		game.pacman.direction.x = 0;
            		game.pacman.direction.y = -1;
            	} else if (lastGesture == DIRECTION_DOWN) {
            		game.pacman.direction.x = 0;
            		game.pacman.direction.y = 1;
            	} else if (lastGesture == DIRECTION_LEFT) {
            		game.pacman.direction.x = -1;
            		game.pacman.direction.y = 0;
            	} else if (lastGesture == DIRECTION_RIGHT) {
            		game.pacman.direction.x = 1;
            		game.pacman.direction.y = 0;
            	}
            }

            if (btnState == 0) { // button pressed
            	GestureDetect_RenderDebugScreen();
            }
    	}

    	if (isTimeToStep) {
    		isTimeToStep = 0;

			Game_DoStep(&game);

            if (btnState == 1) { // button released or unconnected
            	GameRenderer_RenderGame(&game);
            }
    	}

    }
}
