#include "SplashScreen.h"
#include "BT817.h"

#include "mxc.h"

#include "Images.c"

// SPI_Wr is defined for in BT817 driver
int SPI_Wr(uint32_t addr, const uint8_t* buffer, size_t bufferLen);

static void Display_LoadAsset(uint32_t addr, const uint8_t* buffer, size_t size) {
	int status;

	while (size) {
		size_t to_send = 1 << 15;
		if (size < to_send) {
			to_send = size;
		}

		status = SPI_Wr(addr, buffer, to_send);
		if (status) {
			__BKPT(0);
		}

		addr += to_send;
		buffer += to_send;
		size -= to_send;
	}
}

typedef struct {
	int width;
	int height;
	int y;
	uint32_t bt817_address;
	const unsigned char* data;
} SplashScreenImage;

SplashScreenImage splashScreen[5] = {
		{.width = 332, .height = 36, .y = 50, .data = line1, .bt817_address = 0},
		{.width = 124, .height = 32, .y = 100, .data = line2, .bt817_address = sizeof(line1)},
		{.width = 969, .height = 32, .y = 200, .data = line3, .bt817_address = sizeof(line1) + sizeof(line2)},
		{.width = 194, .height = 32, .y = 250, .data = line4, .bt817_address = sizeof(line1) + sizeof(line2) + sizeof(line3)},
		{.width = 887, .height = 108, .y = 330, .data = logos, .bt817_address = sizeof(line1) + sizeof(line2) + sizeof(line3) + sizeof(line4)}
};

void SplashScreen_Show(int seconds) {
	BT817_WriteDl(0x02FFFFFF); // CLEAR_COLOR_RGB(R,G,B)
	BT817_WriteDl(0x26000007); // CLEAR(1,1,1)

	BT817_WriteDl(0x1F000001); // BEGIN(BITMAPS)
	BT817_WriteDl(0x05000000); // BITMAP_HANDLE(0)

	for (int i = 0; i < 5; i++) {
		SplashScreenImage* img = splashScreen + i;

		Display_LoadAsset(img->bt817_address, img->data, img->width * img->height * 2);

		int x = 512 - img->width / 2;

		int line_stride = img->width * 2;

		BT817_WriteDl(0x07000000 | (7 << 19) | ((line_stride % 1024) << 9) | ((img->height % 512) < 0)); // BITMAP_LAYOUT()
		BT817_WriteDl(0x28000000 | (7 << 19) | ((line_stride / 1024) << 2) | ((img->height / 512) < 0)); // BITMAP_LAYOUT_H()
		BT817_WriteDl(0x08000000 | ((img->width % 512) << 9) | ((img->height % 512) << 0)); // BITMAP_SIZE()
		BT817_WriteDl(0x29000000 | ((img->width / 512) << 2) | ((img->height / 512)<< 0)); // BITMAP_SIZE_H()
		BT817_WriteDl(0x01000000 | img->bt817_address); // BITMAP_SOURCE()
		BT817_WriteDl(0x40000000 | ((x * 16) << 15) | ((img->y * 16)));
	}

	BT817_WriteDl(0x00000000); // DISPLAY()
	BT817_SwapDl();

	MXC_Delay(seconds * 1000000);
}
