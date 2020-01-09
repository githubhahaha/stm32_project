#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  

typedef  void (*iapfun)(void);				
#define FLASH_APP1_ADDR		0x08010000  
											
void iap_load_app(u32 appxaddr);		
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	
#endif







































