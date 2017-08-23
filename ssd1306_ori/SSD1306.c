#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include "I2C.h"
#include "SSD1306.h"
// Constants
#define   SSD1306_I2C_ADDRESS  0x3C	// 011110+SA0+RW - 0x3C or 0x3D
#define   SSD1306_SETCONTRAST  0x81
#define   SSD1306_DISPLAYALLON_RESUME  0xA4
#define   SSD1306_DISPLAYALLON  0xA5
#define   SSD1306_NORMALDISPLAY  0xA6
#define   SSD1306_INVERTDISPLAY  0xA7
#define   SSD1306_DISPLAYOFF  0xAE
#define   SSD1306_DISPLAYON  0xAF
#define   SSD1306_SETDISPLAYOFFSET  0xD3
#define   SSD1306_SETCOMPINS  0xDA
#define   SSD1306_SETVCOMDETECT  0xDB
#define   SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define   SSD1306_SETPRECHARGE  0xD9
#define   SSD1306_SETMULTIPLEX  0xA8
#define   SSD1306_SETLOWCOLUMN  0x00
#define   SSD1306_SETHIGHCOLUMN  0x10
#define   SSD1306_SETSTARTLINE  0x40
#define   SSD1306_MEMORYMODE  0x20
#define   SSD1306_COLUMNADDR  0x21
#define   SSD1306_PAGEADDR  0x22
#define   SSD1306_COMSCANINC  0xC0
#define   SSD1306_COMSCANDEC  0xC8
#define   SSD1306_SEGREMAP  0xA0
#define   SSD1306_CHARGEPUMP  0x8D
#define   SSD1306_EXTERNALVCC  0x1
#define   SSD1306_SWITCHCAPVCC  0x2

// Scrolling constants
#define   SSD1306_ACTIVATE_SCROLL  0x2F
#define   SSD1306_DEACTIVATE_SCROLL  0x2E
#define   SSD1306_SET_VERTICAL_SCROLL_AREA  0xA3
#define   SSD1306_RIGHT_HORIZONTAL_SCROLL  0x26
#define   SSD1306_LEFT_HORIZONTAL_SCROLL  0x27
#define   SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL  0x29
#define   SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL   0x2A

#define   SSD1306_WIDTH   128
#define   SSD1306_HEIGHT   64

unsigned char SSD1306_vccstate;
int width, height;
int fd_i2c;

void begin(unsigned char vccstate);
void write_cmd(unsigned char c);
void initialize(void);

int SSD1306_init(void)
{
    unsigned char vccstate;
    unsigned char address;

    int i,size;

    width =  SSD1306_WIDTH;
    height = SSD1306_HEIGHT;

    SSD1306_pages = height/8;

    size =  width * SSD1306_pages;
    for(i=0; i<size; i++){
        SSD1306_buffer[i] = 0;
    }
    // Handle hardware I2C
    printf("Using hardware I2C with custom I2C provider.");

    fd_i2c= I2CSetup (SSD1306_I2C_ADDRESS) ;  /*Use i2cdetect command to find your respective device address*/

    if(fd_i2c==-1){
        printf("Can't setup the I2C device\n");
        return -1;
    }
    vccstate = SSD1306_SWITCHCAPVCC;

    begin(vccstate);
    return 0;
}

void begin(unsigned char vccstate)
{
    //Initialize display.
    // Save vcc state.
    SSD1306_vccstate = vccstate;

    initialize();
    // Turn on the display.
    write_cmd(SSD1306_DISPLAYON);
}

void write_cmd(unsigned char c)
{
    unsigned char control;

	control = 0x00;   // Co = 0, rs= 0
    I2CWriteReg8(fd_i2c, control, c);
}

void initialize(void)
{
    // 128x64 pixel specific initialization.
    write_cmd(SSD1306_DISPLAYOFF);                    // 0xAE
    write_cmd(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
    write_cmd(0x80);                                  // the suggested ratio 0x80
    write_cmd(SSD1306_SETMULTIPLEX);                  // 0xA8
    write_cmd(0x3F);
    write_cmd(SSD1306_SETDISPLAYOFFSET);             // 0xD3
    write_cmd(0x0);                                   // no offset
    write_cmd(SSD1306_SETSTARTLINE | 0x0);            // line //0
    write_cmd(SSD1306_CHARGEPUMP);                   // 0x8D

	if(SSD1306_vccstate == SSD1306_EXTERNALVCC){
    	write_cmd(0x10);
    } else {
		write_cmd(0x14);
    }
	
    write_cmd(SSD1306_MEMORYMODE);                    // 0x20
    write_cmd(0x00);                                // 0x0 act like ks0108
    write_cmd(SSD1306_SEGREMAP | 0x1);
    write_cmd(SSD1306_COMSCANDEC);

    write_cmd(SSD1306_SETCOMPINS) ;                   // 0xDA
    write_cmd(0x12);
    write_cmd(SSD1306_SETCONTRAST);                  // 0x81

	if(SSD1306_vccstate == SSD1306_EXTERNALVCC){
		write_cmd(0x9F);
    } else {
		write_cmd(0xCF);
    }

	write_cmd(SSD1306_SETPRECHARGE);

	if(SSD1306_vccstate == SSD1306_EXTERNALVCC){
        write_cmd(0x22);
    } else {
		write_cmd(0xF1);
    }
    write_cmd(SSD1306_SETVCOMDETECT);                 // 0xDB
    write_cmd(0x40);
    write_cmd(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
    write_cmd(SSD1306_NORMALDISPLAY);                 // 0xA6

    write_cmd(SSD1306_DEACTIVATE_SCROLL);
    write_cmd(SSD1306_DISPLAYON);       //--turn on oled panel
}

void display(void)
{
    int i,k,m;
    int size,status;
    unsigned char c;
    unsigned char control;
    
    // Write display buffer to physical display.
	write_cmd(SSD1306_COLUMNADDR);
	write_cmd(0);               // Column start address. (0 = reset)
	write_cmd(width-1);         // Column end address.
	
    write_cmd(SSD1306_PAGEADDR);
	write_cmd(0);               // Page start address. (0 = reset)
	write_cmd(7);               // Page end address.  SSD1306_LCDHEIGHT == 64

    // Write buffer data.
    // total size of buffer = 1024
    size =  width * SSD1306_pages;

    //once 16 byte write
    size =  size/16;

    for(i= 0; i < size; i++){
	    control = 0x40;   // Co = 0, rs = 0
        m = 16 * i;
        for(k=m; k<m+16; k++) {
           c = SSD1306_buffer[k];
           I2CWriteReg8 (fd_i2c, control, c);
        }
    }
    printf("display \n");
}