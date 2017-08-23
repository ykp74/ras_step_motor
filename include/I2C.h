 #ifndef I2C_H
 #define I2C_H
/*
 * I2C.h:
 *	Simplified I2C access routines
 */
extern int I2CRead           (int fd) ;
extern int I2CReadReg8       (int fd, int reg) ;
extern int I2CReadReg16      (int fd, int reg) ;

extern int I2CWrite          (int fd, int data) ;
extern int I2CWriteReg8      (int fd, int reg, int data) ;
extern int I2CWriteReg16     (int fd, int reg, int data) ;

extern int I2CSetupInterface (const char *device, int devId) ;
extern int I2CSetup          (const int devId) ;
#endif

