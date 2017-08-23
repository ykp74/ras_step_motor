// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/mman.h>
#include <time.h>
#include <arpa/inet.h> 

#include "drone_GPIO.h"
#include "drone_SPI.h"
#include "drone_I2C.h"

// Pin modes

#define	GPIO_IN			 0
#define	GPIO_OUT	         1

#define	LOW			 0
#define	HIGH			 1
#define	True			 1
#define	False			 0

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

#define   None   0
#define   Active 1   

unsigned char SSD1306_vccstate;
unsigned char SSD1306_buffer[128*64];
unsigned char SSD1306_pages;
unsigned char SSD1306_spi,SSD1306_i2c,SSD1306_gpio_rst;
int rst, rs, width, height;
int max_speed_hz;

int RSPort  = 24;
int RstPort = 25;

int fd_i2c,fd_spi;

void initialize();
void write_cmd(unsigned char c);
void write_data(unsigned char c);
int write(unsigned char *list);
void reset();
void begin(unsigned char vccstate);

int SSD1306_init(unsigned char gpio_rst,unsigned char spi,unsigned char i2c)
{
       unsigned char vccstate;
       unsigned char address;

       int i,size;

		width =  SSD1306_WIDTH;
		height = SSD1306_HEIGHT;

		SSD1306_pages = height/8;

        size =  width * SSD1306_pages;
        for(i=0; i<size; i++)
		SSD1306_buffer[i] = 0;

// Default to platform GPIO if not provided.
// Setup reset pin.

        max_speed_hz = 8000000;

        if(gpio_rst != None)
        {
        rst = RstPort;
        GPIO_mode(rst, GPIO_OUT);
	    SSD1306_gpio_rst = gpio_rst;
        }
// Handle hardware SPI
		if(spi != None)
        {
			printf("Using hardware SPI\n");
			SSD1306_spi = spi;

        fd_spi = drone_SPISetup (0,8000000) ; 

        if(fd_spi ==-1)
        {
                printf("Can't setup the SPI device\n");
                return -1;
        }
        }
		// Handle hardware I2C
		else if (i2c != 0) {
		printf("Using hardware I2C with custom I2C provider.");

        address = SSD1306_I2C_ADDRESS;

        fd_i2c= drone_I2CSetup (address) ;  /*Use i2cdetect command to find your respective device address*/

        if(fd_i2c==-1)
        {
                printf("Can't setup the I2C device\n");
                return -1;
        }
        }

// Initialize DC pin if using SPI.

	if(SSD1306_spi != 0) {
        rs =  RSPort;
        GPIO_mode(rs, GPIO_OUT);
        }

        vccstate = SSD1306_SWITCHCAPVCC;

       begin(vccstate);

}

void write_cmd(unsigned char c)
{
        unsigned char cmdbuf[32];
        unsigned char control;
// Send command byte to display.
		if(SSD1306_spi != None)
        {
// SPI write.
        cmdbuf[0] = c;
        GPIO_output(rs, False);  // RS=0
        drone_SPIDataRW (0, cmdbuf, 1);
        }
	else// i2c
        {
// I2C write.
	control = 0x00;   // Co = 0, rs= 0
            drone_I2CWriteReg8 (fd_i2c, control, c);
        }
}

void write_data(unsigned char c)
{
        unsigned char databuf[32];
        unsigned char control;
// Send byte of data to display.
	if(SSD1306_spi != None)
        {
// SPI write.
        databuf[0] = c;
        GPIO_output(rs, True);  // RS=1
        drone_SPIDataRW (0, databuf, 1);
        }
	else //
        {
// I2C write.
	control = 0x40;   // Co = 0, rs = 0
        drone_I2CWriteReg8 (fd_i2c, control, c);
        }
}
int write(unsigned char *list)
{
        unsigned char pixline[1024];
        unsigned char *ptr;
        int len;

        len = strlen(list);

        if(len == 0) return -1;
        GPIO_output(rs, False);  // RS=0
        ptr = list;
        drone_SPIDataRW (0, ptr, 1);
        GPIO_output(rs, True) ;  // RS=1
        ptr = list + 1;
        drone_SPIDataRW (0, ptr, len-1);
        return 0;
}
void begin(unsigned char vccstate)
{
//Initialize display.
// Save vcc state.

		SSD1306_vccstate = vccstate;

		// Reset and initialize display.
		if(SSD1306_gpio_rst != None) reset();
		initialize();
		// Turn on the display.
		write_cmd(SSD1306_DISPLAYON);
}

void reset()
{
         printf("reset routine\n");
// Reset the display.
// Set reset high for a millisecond.
        GPIO_output(rst, True);
        usleep(1000);
// Set reset low for 10 milliseconds.
        GPIO_output(rst, False);
        usleep(10000);
// Set reset high again.
        GPIO_output(rst, True);
        usleep(10000);
}

void display()
{
        int i,k,m;
        int size,status;
        unsigned char c;
        unsigned char control;
// Write display buffer to physical display.
		write_cmd(SSD1306_COLUMNADDR);
		write_cmd(0);              // Column start address. (0 = reset)
		write_cmd(width-1);        // Column end address.
		write_cmd(SSD1306_PAGEADDR);
		write_cmd(1);              // Page start address. (0 = reset)
		write_cmd(SSD1306_pages-1);      // Page end address.
// Write buffer data.
		if(SSD1306_spi != None)
        {
// Set DC high for data.
        GPIO_output(rs, True) ;  // RS=1
// Write buffer.

        status = drone_SPIDataRW (0, SSD1306_buffer, 1024);
        printf("drone_SPIDataRW(%d)\n",status);

        }
	else //i2c
        {
//      total size of buffer = 1024
        size =  width * SSD1306_pages;
//      once 16 byte write
        size =  size/16;

            for(i= 0; i<size; i++)
            {
	   control = 0x40;   // Co = 0, rs = 0
           m = 16 *i;
           for(k=m; k<m+16; k++){
           c = SSD1306_buffer[k];
           drone_I2CWriteReg8 (fd_i2c, control, c);
           }
           }
        }
}

void clear()
{
       int i,size;
// Clear contents of image buffer.
        size =  width * SSD1306_pages;
        for(i=0; i<size; i++)
		SSD1306_buffer[i] = 0;

}
void set_contrast(unsigned char contrast)
{
// Sets the contrast of the display.  Contrast should be a value between
// 0 and 255.
		if(contrast < 0 || contrast > 255)
        
		printf("Contrast must be a value from 0 to 255 (inclusive).\n");
		write_cmd(SSD1306_SETCONTRAST);
		write_cmd(contrast);
}

void  SSD1306_dim(unsigned char dim)
{
       unsigned char contrast;
// Adjusts contrast to dim the display if dim is True, otherwise sets the
// contrast to normal brightness if dim is False.
		
// Assume dim display.
		contrast = 0;
		// Adjust contrast based on VCC if not dimming.
		if(!dim)
        {
			if(SSD1306_vccstate == SSD1306_EXTERNALVCC)
				contrast = 0x9F;
			else
				contrast = 0xCF;
        }
}

void initialize()
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
		if(SSD1306_vccstate == SSD1306_EXTERNALVCC)
        {
		write_cmd(0x10);
        }
		else
        {
		write_cmd(0x14);
        }
		write_cmd(SSD1306_MEMORYMODE);                    // 0x20
		write_cmd(0x00);                                // 0x0 act like ks0108
		write_cmd(SSD1306_SEGREMAP | 0x1);
		write_cmd(SSD1306_COMSCANDEC);
		write_cmd(SSD1306_SETCOMPINS) ;                   // 0xDA
		write_cmd(0x12);
		write_cmd(SSD1306_SETCONTRAST);                  // 0x81
		if(SSD1306_vccstate == SSD1306_EXTERNALVCC)
        {
		write_cmd(0x9F);
        }
		else
        {
		write_cmd(0xCF);
        }
		write_cmd(SSD1306_SETPRECHARGE);
// 0xd9
		if(SSD1306_vccstate == SSD1306_EXTERNALVCC)
        {
		write_cmd(0x22);
        }
		else
        {
		write_cmd(0xF1);
        }
		write_cmd(SSD1306_SETVCOMDETECT);                 // 0xDB
		write_cmd(0x40);
		write_cmd(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
		write_cmd(SSD1306_NORMALDISPLAY);                 // 0xA6
}


int main (int argc, char *argv[])
{
     unsigned char gpio_rst,spi,i2c;
     unsigned char interface_mode;
     int i,k,m;

    if (argc != 2)
    {
        printf("Usage : ./drone_SSD1306 [option]\n");
        printf("예    : ./drone_SSD1306  -i2c(spi)\n");    
        exit(0);
    }

    interface_mode = 0;

       if(strncmp("-i2c", argv[1], 4) == 0) {
        interface_mode = 0;
        }
        else if(strncmp("-spi", argv[1], 4) == 0) {
       interface_mode = 1;
       }
       else
       {
       printf("interface mode (%s)\n",argv[1]);
       printf("itry again(-i2c or -spi\n");
       exit(0);
       }
  // Set up gpi pointer for direct register access

	if ( GPIO_setup() == -1 )
		exit( 1 );

        if(  interface_mode == 0) {
//      i2c mode
        gpio_rst = None;
        spi = None;
        i2c = Active;
        SSD1306_init(gpio_rst,spi,i2c);
        }
        else
        {
// spi mode
         gpio_rst = Active;
         spi = Active;
         i2c = None;
         SSD1306_init(gpio_rst,spi,i2c);
        }
 
        while(10)
        {
        m = 0;
        for(i=0; i<64; i++) {
        for(k=0; k<128; k++) {
         SSD1306_buffer[m] = 0;
         m++;
         }
         }

       display();
  
      sleep(5);
         m = 0;
        for(i=0; i<64; i++) {
        for(k=0; k<128; k++) {
         SSD1306_buffer[m] = 0x0f0;
         m++;
         }
         }
     display();
  
      sleep(5);
       }
      return 0;
}



