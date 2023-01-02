#include "BT817.h"
#include "mxc.h"
#include "max20303.h"

#include <stdint.h>

static void callback(void* req, int result) {

}

int SPI_Wr(uint32_t addr, const uint8_t* buffer, size_t bufferLen) {
	int status;
	mxc_spi_req_t transaction;

	addr = (addr & 0x003FFFFF) | 0x80000000;

    status = MXC_SPI_SetDataSize(BT817_SPI, 8);
    if (status) {
    	__BKPT(0);
    }

    status = MXC_SPI_SetWidth(BT817_SPI, SPI_WIDTH_STANDARD);
    if (status) {
    	__BKPT(0);
    }

    transaction.completeCB = callback;
    transaction.spi = BT817_SPI;
    transaction.ssDeassert = 0;
    transaction.txData = (uint8_t*)buffer;
    transaction.txLen = 1;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    // dummy transfer with CS high because MAX78000 first transfer contains glitches.
    MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    MXC_GPIO_OutClr(BT817_SS_GPIO, BT817_SS_GPIO_PIN);


    uint8_t addrBuff[3];
    addrBuff[0] = ((addr & 0xFF0000) >> 16) | 0x80;
    addrBuff[1] = (addr & 0xFF00) >> 8;
    addrBuff[2] = (addr & 0xFF) >> 0;

    transaction.ssDeassert = 0;
    transaction.txData = addrBuff;
    transaction.txLen = 3;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    status = MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    transaction.ssDeassert = 1;
    transaction.txData = (uint8_t*)buffer;
    transaction.txLen = bufferLen;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    status = MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }


    MXC_GPIO_OutSet(BT817_SS_GPIO, BT817_SS_GPIO_PIN);

    return 0;
}

int SPI_Rd(uint32_t addr, uint8_t* buffer, size_t bufferLen) {
	int status;
	mxc_spi_req_t transaction;

	addr &= 0x003FFFFF;

    status = MXC_SPI_SetDataSize(BT817_SPI, 8);
    if (status) {
    	__BKPT(0);
    }

    status = MXC_SPI_SetWidth(BT817_SPI, SPI_WIDTH_STANDARD);
    if (status) {
    	__BKPT(0);
    }

    transaction.completeCB = callback;
    transaction.spi = BT817_SPI;
    transaction.ssDeassert = 0;
    transaction.txData = buffer;
    transaction.txLen = 1;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    // dummy transfer with CS high because MAX78000 first transfer contains glitches.
    MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    MXC_GPIO_OutClr(BT817_SS_GPIO, BT817_SS_GPIO_PIN);

    uint8_t addrBuff[3];
    addrBuff[0] = (addr & 0xFF0000) >> 16;
    addrBuff[1] = (addr & 0xFF00) >> 8;
    addrBuff[2] = (addr & 0xFF) >> 0;

    // transmit addr (3 bytes)
    transaction.ssDeassert = 0;
    transaction.txData = addrBuff;
    transaction.txLen = 3;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    status = MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    // dummy byte read
    uint8_t dummy;
    transaction.ssDeassert = 0;
    transaction.txData = NULL;
    transaction.txLen = 0;
    transaction.rxData = &dummy;
    transaction.rxLen = 1;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    status = MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    transaction.ssDeassert = 1;
    transaction.txData = NULL;
    transaction.txLen = 0;
    transaction.rxData = buffer;
    transaction.rxLen = bufferLen;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    status = MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }


    MXC_GPIO_OutSet(BT817_SS_GPIO, BT817_SS_GPIO_PIN);

    return 0;

}

int SPI_WrHostCommand(uint8_t cmd, uint8_t param) {
	int status;
	mxc_spi_req_t transaction;

    status = MXC_SPI_SetDataSize(BT817_SPI, 8);
    if (status) {
    	__BKPT(0);
    }

    status = MXC_SPI_SetWidth(BT817_SPI, SPI_WIDTH_STANDARD);
    if (status) {
    	__BKPT(0);
    }

    transaction.completeCB = callback;
    transaction.spi = BT817_SPI;
    transaction.ssDeassert = 0;
    transaction.txData = &cmd;
    transaction.txLen = 1;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    // dummy transfer with CS high because MAX78000 first transfer contains glitches.
    MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    MXC_GPIO_OutClr(BT817_SS_GPIO, BT817_SS_GPIO_PIN);

    transaction.ssDeassert = 0;
    transaction.txData = &cmd;
    transaction.txLen = 1;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    status = MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    transaction.ssDeassert = 0;
    transaction.txData = &param;
    transaction.txLen = 1;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    uint8_t zero = 0;

    status = MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    transaction.ssDeassert = 1;
    transaction.txData = &zero;
    transaction.txLen = 1;
    transaction.rxData = NULL;
    transaction.rxLen = 0;
    transaction.ssIdx = 0;
    transaction.txCnt = 0;
    transaction.rxCnt = 0;

    status = MXC_SPI_MasterTransaction(&transaction);
    if (status) {
    	__BKPT(0);
    }

    MXC_GPIO_OutSet(BT817_SS_GPIO, BT817_SS_GPIO_PIN);

    return 0;
}


static int(*wrFunction)(uint32_t, const uint8_t*, size_t) = SPI_Wr;
static int(*rdFunction)(uint32_t, uint8_t*, size_t) = SPI_Rd;
static int(*wrHostCommandFunction)(uint8_t, uint8_t) = SPI_WrHostCommand;

static uint32_t ramDlPtr = BT817_RAM_DL;

int BT817_Wr8(uint32_t address, uint8_t data) {
	return wrFunction(0x800000 | (address & 0x3FFFFF), &data, sizeof(data));
}

int BT817_Wr16(uint32_t address, uint16_t data) {
	return wrFunction(0x800000 | (address & 0x3FFFFF), (uint8_t*)&data, sizeof(data));
}

int BT817_Wr32(uint32_t address, uint32_t data) {
	return wrFunction(0x800000 | (address & 0x3FFFFF), (uint8_t*)&data, sizeof(data));
}

int BT817_WrBuffer(uint32_t address, uint8_t* data, size_t dataSize) {
	return wrFunction(0x800000 | (address & 0x3FFFFF), data, dataSize);
}

int BT817_Rd8(uint32_t address, uint8_t* data) {
	return rdFunction(address & 0x3FFFFF, data, sizeof(*data));
}

int BT817_Rd16(uint32_t address, uint16_t* data) {
	return rdFunction(address & 0x3FFFFF, (uint8_t*)data, sizeof(*data));
}

int BT817_Rd32(uint32_t address, uint32_t* data) {
	return rdFunction(address & 0x3FFFFF, (uint8_t*)data, sizeof(*data));
}

int BT817_RdBuffer(uint32_t address, uint8_t* data, size_t dataSize) {
	return rdFunction(address & 0x3FFFFF, data, dataSize);
}

int BT817_HostCmd(uint8_t cmd, uint8_t param) {
	return wrHostCommandFunction(cmd, param);
}

int BT817_Rd8Until(uint32_t address, uint8_t expectedValue, uint32_t timeout, uint32_t delayBetweenAttemps) {
	int status;

	uint8_t actualValue;

	while (timeout--) {
		status = BT817_Rd8(address, &actualValue);
		if (status) {
			__BKPT(0);
			//UART_PrintString("BT817_Rd8 in BT817_Rd8Until failed.\r\n");
			return status;
		}

		if (actualValue == expectedValue) {
			return 0;
		}

		MXC_Delay(delayBetweenAttemps * 1000);
	}

	return 5;
}

int BT817_Rd16Until(uint32_t address, uint16_t expectedValue, uint32_t timeout, uint32_t delayBetweenAttemps) {
	int status;

	uint16_t actualValue;

	while (timeout--) {
		status = BT817_Rd16(address, &actualValue);
		if (status) {
			__BKPT(0);
			//UART_PrintString("BT817_Rd16 in BT817_Rd16Until failed.\r\n");
			return status;
		}

		if (actualValue == expectedValue) {
			return 0;
		}

		MXC_Delay(delayBetweenAttemps * 1000);
	}

	return 5;
}

int BT817_Rd32Until(uint32_t address, uint32_t expectedValue, uint32_t timeout, uint32_t delayBetweenAttemps) {
	int status;

	uint32_t actualValue;

	while (timeout--) {
		status = BT817_Rd32(address, &actualValue);
		if (status) {
			__BKPT(0);
			//UART_PrintString("BT817_Rd32 in BT817_Rd32Until failed.\r\n");
			return status;
		}

		if (actualValue == expectedValue) {
			return 0;
		}

		MXC_Delay(delayBetweenAttemps * 1000);
	}

	return 5;
}

void BT817_PowerCycle() {
	int status;

	mxc_gpio_cfg_t pd;
	pd.port = MXC_GPIO1;
	pd.mask = MXC_GPIO_PIN_0;
	pd.func = MXC_GPIO_FUNC_OUT;
	pd.vssel = MXC_GPIO_VSSEL_VDDIOH;
	pd.pad = MXC_GPIO_PAD_NONE;
	status = MXC_GPIO_Config(&pd);
	if (status) {
		__BKPT(0);
	}

	MXC_GPIO_OutClr(MXC_GPIO1, MXC_GPIO_PIN_0);
	MXC_Delay(20000);

	MXC_GPIO_OutSet(MXC_GPIO1, MXC_GPIO_PIN_0);
	MXC_Delay(20000);

	wrFunction = SPI_Wr;
	rdFunction = SPI_Rd;
	wrHostCommandFunction = SPI_WrHostCommand;
}

int BT817_Init() {
	int status;

	mxc_gpio_cfg_t ss;
	ss.port = BT817_SS_GPIO;
	ss.mask = BT817_SS_GPIO_PIN;
	ss.func = MXC_GPIO_FUNC_OUT;
	ss.vssel = MXC_GPIO_VSSEL_VDDIOH;
	ss.pad = MXC_GPIO_PAD_NONE;
	status = MXC_GPIO_Config(&ss);
	if (status) {
		__BKPT(0);
	}

	MXC_GPIO_OutSet(BT817_SS_GPIO, BT817_SS_GPIO_PIN);

	BT817_PowerCycle();

	BT817_HostCmd(BT817_HOST_CMD_CLKEXT);
	BT817_HostCmd(BT817_HOST_CMD_CLKSEL);
	BT817_HostCmd(BT817_HOST_CMD_ACTIVE);

	MXC_Delay(300000);

	status = BT817_Rd8Until(BT817_REG_ID, 0x7C, 1000, 10);
	if (status) {
		__BKPT(0);
		//UART_PrintString("BT817_Rd8Until(BT817_REG_ID) failed.\r\n");
		return status;
	}

	status = BT817_Rd16Until(BT817_REG_CPURESET, 0x00, 1000, 10);
	if (status) {
		__BKPT(0);
		//UART_PrintString("BT817_Rd16Until(BT817_REG_CPURESET) failed.\r\n");
		return status;
	}

	status = BT817_Wr32(BT817_REG_FREQUENCY, 72000000);
	if (status) {
		__BKPT(0);
		//UART_PrintString("BT817_Wr32(BT817_REG_FREQUENCY) failed.\r\n");
		return status;
	}

	/*
	status = BT817_Wr8(BT817_REG_SPI_WIDTH, 0x06);
	if (status) {
		UART_PrintString("BT817_Wr8(BT817_REG_SPI_WIDTH) failed.\r\n");
		return status;
	}

	wrFunction = OSPI_QSPI_Wr;
	rdFunction = OSPI_QSPI_Rd;
	wrHostCommandFunction = OSPI_QSPI_WrHostCommand;
	*/

	/*
	BT817 known values:
	REG_FREQUENCY: 72MHz
	Resolution: 1280x120
	Refresh rate:83Hz
	*/
	status = 0;
	status |= BT817_Wr16(BT817_REG_HCYCLE, 1344);
	status |= BT817_Wr16(BT817_REG_HOFFSET, 160);
	status |= BT817_Wr16(BT817_REG_HSYNC0, 0);
	status |= BT817_Wr16(BT817_REG_HSYNC1, 100);
	status |= BT817_Wr16(BT817_REG_VCYCLE, 635);
	status |= BT817_Wr16(BT817_REG_VOFFSET, 23);
	status |= BT817_Wr16(BT817_REG_VSYNC0, 0);
	status |= BT817_Wr16(BT817_REG_VSYNC1, 10);
	status |= BT817_Wr8(BT817_REG_SWIZZLE, 0);
	status |= BT817_Wr8(BT817_REG_PCLK_POL, 1);
	status |= BT817_Wr8(BT817_REG_CSPREAD, 0);
	//status |= BT817_Wr16(BT817_REG_HSIZE, 1280);
	status |= BT817_Wr16(BT817_REG_HSIZE, 1024);
	status |= BT817_Wr16(BT817_REG_VSIZE, 600);
	if (status) {
		__BKPT(0);
		//UART_PrintString("Writing resolution configuration failed.\r\n");
		return status;
	}

	status = 0;


	/*
	status |= BT817_Wr32(BT817_RAM_DL + 0, 0x02FFFF00); // CLEAR_COLOR_RGB(0,0,0)
	status |= BT817_Wr32(BT817_RAM_DL + 4, 0x26000007); // CLEAR(1,1,1)
	status |= BT817_Wr32(BT817_RAM_DL + 8, 0x04FF0000); // CLEAR_COLOR_RGB(0,0,0)
	status |= BT817_Wr32(BT817_RAM_DL + 12, 0x1F000009); // BEGIN(RECTS)
	status |= BT817_Wr32(BT817_RAM_DL + 16, 0x40000000 | ((100 * 16) << 15) | 100 * 16); // VERTEX2F()
	status |= BT817_Wr32(BT817_RAM_DL + 20, 0x40000000 | ((200 * 16) << 15) | 200 * 16); // VERTEX2F()
	status |= BT817_Wr32(BT817_RAM_DL + 24, 0x00000000); // DISPLAY()
	status |= BT817_Wr8(BT817_REG_DLSWAP, 0x02);
	 */

	status |= BT817_Wr32(BT817_RAM_DL + 0, 0x02000000); // CLEAR_COLOR_RGB(0,0,0)
	status |= BT817_Wr32(BT817_RAM_DL + 4, 0x26000007); // CLEAR(1,1,1)
	status |= BT817_Wr32(BT817_RAM_DL + 8, 0x00000000); // DISPLAY()
	status |= BT817_Wr8(BT817_REG_DLSWAP, 0x02);

	status |= BT817_Wr16(BT817_REG_GPIOX_DIR, 0xFFFF);
	status |= BT817_Wr16(BT817_REG_GPIOX, 0xFFFF);
	status |= BT817_Wr8(BT817_REG_PCLK, 2);

	return 0;
}

int BT817_InitTouchController() {
	int status;

	status = BT817_Wr8(BT817_REG_CPURESET, 0x02);
	if (status) {
		__BKPT(0);
		//UART_PrintString("Asserting touch controller reset failed.\r\n");
		return status;
	}

	MXC_Delay(10000);

	/*
	status = BT817_WrBuffer(BT817_RAM_JTBOOT, touch_capcs_815, sizeof(touch_capcs_815));
	if (status) {
		UART_PrintString("Writing touch controller firmware failed.\r\n");
		return status;
	}
	*/

	status = BT817_Wr8(BT817_REG_CPURESET, 0x00);
	if (status) {
		__BKPT(0);
		//UART_PrintString("Deasserting touch controller reset failed.\r\n");
		return status;
	}

	// Set calibration data. This data fits only to my unit and my display. 
	// Use SampleApp_MSVC demo to obtain them. Set breakpùint in Calibration_Save
	// funtion and copy values there.

	status |= BT817_Wr32(BT817_REG_TOUCH_TRANSFORM_A, BT817_TOUCH_CALIBRATION_A);
	status |= BT817_Wr32(BT817_REG_TOUCH_TRANSFORM_B, BT817_TOUCH_CALIBRATION_B);
	status |= BT817_Wr32(BT817_REG_TOUCH_TRANSFORM_C, BT817_TOUCH_CALIBRATION_C);
	status |= BT817_Wr32(BT817_REG_TOUCH_TRANSFORM_D, BT817_TOUCH_CALIBRATION_D);
	status |= BT817_Wr32(BT817_REG_TOUCH_TRANSFORM_E, BT817_TOUCH_CALIBRATION_E);
	status |= BT817_Wr32(BT817_REG_TOUCH_TRANSFORM_F, BT817_TOUCH_CALIBRATION_F);
	if (status) {
		__BKPT(0);
		//UART_PrintString("Writing touch controller calibration data failed.\r\n");
		return status;
	}

	BT817_Wr8(BT817_REG_CTOUCH_EXTENDED, 0);

	return 0;
}

/*
int BT817_InitInterrupt(EXTI_HandleTypeDef* interruptExti, uint32_t interruptMask) {
	int status;

	__GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef gpio;
	gpio.Alternate = 0;
	gpio.Mode = GPIO_MODE_IT_FALLING;
	gpio.Pin = GPIO_PIN_2;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &gpio);

	NVIC_SetPriority(EXTI2_IRQn, 2);
	NVIC_EnableIRQ(EXTI2_IRQn);

	status |= BT817_Wr32(BT817_REG_INT_MASK, interruptMask);
	status |= BT817_Wr32(BT817_REG_INT_EN, 1);
	if (status) {
		UART_PrintString("Error while configuring interrupts.\r\n");
		return status;
	}

	return HAL_OK;
}
*/

uint32_t dl[2048];

int BT817_WriteDl(uint32_t command) {
	//int status;

	if (ramDlPtr >= BT817_RAM_DL + 8192) {
		__BKPT(0);
		//UART_PrintString("Cannod add command to display list because capacity was exhausted.\r\n");
		return 1;
	}

	/*
	status = BT817_Wr32(ramDlPtr, command);
	if (status) {
		__BKPT(0);
		//UART_PrintString("Adding command to display list failed.\r\n");
		return status;
	}

	*/

	dl[(ramDlPtr - BT817_RAM_DL) / 4] = command;

	ramDlPtr += 4;

	return 0;
}

int BT817_SwapDl() {
	int status;

	status = BT817_Rd32Until(BT817_REG_DLSWAP, 0, 100, 10);
	if (status) {
		__BKPT(0);
	}

	BT817_WrBuffer(BT817_RAM_DL, (uint8_t*)dl, ramDlPtr - BT817_RAM_DL);

	uint32_t verifyBuffer[2048];
	memset(verifyBuffer, 0, sizeof(verifyBuffer));

	BT817_RdBuffer(BT817_RAM_DL, (uint8_t*)verifyBuffer, ramDlPtr - BT817_RAM_DL);

	for (int i = 0; i < (ramDlPtr - BT817_RAM_DL) / 4; i++) {
		if (verifyBuffer[i] != dl[i]) {
			//__BKPT(0);
		}
	}

	status = BT817_Wr8(BT817_REG_DLSWAP, 0x02);
	if (status) {
		__BKPT(0);
		//UART_PrintString("Swapping display list failed.\r\n");
		return status;
	}

	ramDlPtr = BT817_RAM_DL;

	return 0;
}

int BT817_GetTouch(int* isTouched, uint32_t* x, uint32_t* y) {
	int status = 0;

	uint32_t touches[5];

	status |= BT817_Rd32(BT817_REG_CTOUCH_TOUCH_XY, touches + 0);
	status |= BT817_Rd32(BT817_REG_CTOUCH_TOUCH1_XY, touches + 1);
	status |= BT817_Rd32(BT817_REG_CTOUCH_TOUCH2_XY, touches + 2);
	status |= BT817_Rd32(BT817_REG_CTOUCH_TOUCH3_XY, touches + 3);

	uint16_t touch4X, touch4Y;

	status |= BT817_Rd16(BT817_REG_CTOUCH_TOUCH4_X, &touch4X);
	status |= BT817_Rd16(BT817_REG_CTOUCH_TOUCH4_Y, &touch4Y);

	if (status) {
		__BKPT(0);
		//UART_PrintString("Error while reading touch data.\r\n");
		return status;
	}

	touches[4] = (((uint32_t)touch4X) << 16) | (uint32_t)touch4Y;

	for (size_t i = 0; i < 5; i++) {
		if (touches[i] == 0x80008000) {
			continue;
		}

		uint32_t _x = (touches[i] & 0xFFFF0000) >> 16;
		uint32_t _y = touches[i] & 0xFFFF;

		if (_x > 1024 || _y > 600) {
			continue;
		}

		*x = _x;
		*y = _y;
		*isTouched = 1;
	}

	return 0;
}
