#ifndef __BT817_H
#define __BT817_H

#include <stdint.h>
#include <stddef.h>

#define BT817_SPI					MXC_SPI0

#define BT817_SS_GPIO				MXC_GPIO2
#define BT817_SS_GPIO_PIN			MXC_GPIO_PIN_3

#define BT817_TOUCH_CALIBRATION_A	36716
#define BT817_TOUCH_CALIBRATION_B	601
#define BT817_TOUCH_CALIBRATION_C	4294877585
#define BT817_TOUCH_CALIBRATION_D	101
#define BT817_TOUCH_CALIBRATION_E	37185
#define BT817_TOUCH_CALIBRATION_F	492507

#define BT817_HOST_CMD_CLKEXT		0x44, 0x00
#define BT817_HOST_CMD_CLKSEL		0x61, 0x06
#define BT817_HOST_CMD_ACTIVE		0x00, 0x00

#define BT817_RAM_DL				0x300000
#define BT817_RAM_JTBOOT			0x30B000

#define BT817_REG_ID				0x302000
#define BT817_REG_FREQUENCY			0x30200C
#define BT817_REG_CPURESET			0x302020
#define BT817_REG_DLSWAP			0x302054
#define BT817_REG_PCLK				0x302070
#define BT817_REG_GPIOX_DIR			0x302098
#define BT817_REG_GPIOX				0x30209C
#define BT817_REG_INT_FLAGS			0x3020A8
#define BT817_REG_INT_EN			0x3020AC
#define BT817_REG_INT_MASK			0x3020B0
#define BT817_REG_SPI_WIDTH			0x302188

#define BT817_REG_HCYCLE			0x30202C
#define BT817_REG_HOFFSET			0x302030
#define BT817_REG_HSYNC0			0x302038
#define BT817_REG_HSYNC1			0x30203C
#define BT817_REG_VCYCLE			0x302040
#define BT817_REG_VOFFSET			0x302044
#define BT817_REG_VSYNC0			0x30204C
#define BT817_REG_VSYNC1			0x302050
#define BT817_REG_SWIZZLE			0x302064
#define BT817_REG_PCLK_POL			0x30206C
#define BT817_REG_CSPREAD			0x302068
#define BT817_REG_HSIZE				0x302034
#define BT817_REG_VSIZE				0x302048

#define BT817_REG_CTOUCH_MODE		0x302104
#define BT817_REG_CTOUCH_EXTENDED	0x302108
#define BT817_REG_CTOUCH_TOUCH1_XY	0x30211C
#define BT817_REG_CTOUCH_TOUCH4_Y	0x302120
#define BT817_REG_CTOUCH_TOUCH_XY	0x302124
#define BT817_REG_CTOUCH_TAG_XY		0x302128
#define BT817_REG_CTOUCH_TAG		0x30212C
#define BT817_REG_CTOUCH_TAG1_XY	0x302130
#define BT817_REG_CTOUCH_TAG1		0x302134
#define BT817_REG_CTOUCH_TAG2_XY	0x302138
#define BT817_REG_CTOUCH_TAG2		0x30213C
#define BT817_REG_CTOUCH_TAG3_XY	0x302140
#define BT817_REG_CTOUCH_TAG3		0x302144
#define BT817_REG_CTOUCH_TAG4_XY	0x302148
#define BT817_REG_CTOUCH_TAG4		0x30214C
#define BT817_REG_CTOUCH_TOUCH4_X	0x30216C
#define BT817_REG_CTOUCH_TOUCH2_XY	0x30218C
#define BT817_REG_CTOUCH_TOUCH3_XY	0x302190

#define BT817_REG_TOUCH_TRANSFORM_A	0x302150
#define BT817_REG_TOUCH_TRANSFORM_B	0x302154
#define BT817_REG_TOUCH_TRANSFORM_C	0x302158
#define BT817_REG_TOUCH_TRANSFORM_D	0x30215C
#define BT817_REG_TOUCH_TRANSFORM_E	0x302160
#define BT817_REG_TOUCH_TRANSFORM_F	0x302164

#define BT817_ROM_CHIPID			0x0C0000

int BT817_Wr8(uint32_t address, uint8_t data);
int BT817_Wr16(uint32_t address, uint16_t data);
int BT817_Wr32(uint32_t address, uint32_t data);
int BT817_WrBuffer(uint32_t address, uint8_t* data, size_t dataSize);

int BT817_Rd8(uint32_t address, uint8_t* data);
int BT817_Rd16(uint32_t address, uint16_t* data);
int BT817_Rd32(uint32_t address, uint32_t* data);
int BT817_RdBuffer(uint32_t address, uint8_t* data, size_t dataSize);

int BT817_HostCmd(uint8_t cmd, uint8_t param);

int BT817_Init();
int BT817_InitTouchController();
//int BT817_InitInterrupt(EXTI_HandleTypeDef* interruptExti, uint32_t interruptMask);

int BT817_WriteDl(uint32_t command);
int BT817_SwapDl();

int BT817_GetTouch(int* isTouched, uint32_t* x, uint32_t* y);

#endif
