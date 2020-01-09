#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "gps.h"
#include "usmart.h"							   
#include "stdio.h"	 
#include "stdarg.h"	 
#include "string.h"	 
#include "math.h"
#include "motor.h"
u8 rdata[1];
u8 buff[]="";
u32 oldcount=0;				   
u32 applenth=0;	 
u8 upload=0;
extern u8 com_flag;
u8 GPS_TX_BUF[USART2_MAX_RECV_LEN]; 					        
nmea_msg gpsx; 			
typedef  void (*iapfun)(void);		
void Led_init()
{
  GPIO_InitTypeDef GPIO_Init;
	__HAL_RCC_GPIOB_CLK_ENABLE(); 
	GPIO_Init.Pin=GPIO_PIN_0|GPIO_PIN_1;//Led1
	GPIO_Init.Mode=GPIO_MODE_OUTPUT_PP;//推挽输出
	GPIO_Init.Speed=GPIO_SPEED_FREQ_HIGH;
	GPIO_Init.Pull=GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB,&GPIO_Init);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);	
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);	

}	
void Exit_Init()
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOA_CLK_ENABLE();               
	GPIO_Initure.Pin=GPIO_PIN_0;                //PA0
  GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      
  GPIO_Initure.Pull=GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	HAL_NVIC_SetPriority(EXTI0_IRQn,2,0);      
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);            
}

//外部中断处理函数
void EXTI0_IRQHandler(void)
{
	  HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	  delay_ms(100);
	  if(GPIO_PIN_SET==HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0))
		{
			HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
			HAL_NVIC_EnableIRQ(EXTI0_IRQn); 
		}
		else
		{
			HAL_NVIC_EnableIRQ(EXTI0_IRQn); 
			return;
		}
}

void u32tostr(u32 dat,char *str)
{
	char temp[20];
	unsigned char i=0,j=0;
	i=0;
	while(dat)
	{
		temp[i]=dat%10+0x30;
		i++;
		dat/=10;
	}
	j=i;
	for(i=0;i<j;i++)
	{
		str[i]=temp[j-i-1];
	}
	if(!i)
  {
		str[i++]='0';
	}
	str[i]=0;
}
void iap_load_app(u32 appxaddr)
{ 
	iapfun jump2app;
	if(((*(vu32*)appxaddr)&0x2FF00000)==0x20000000)	
	{ 
		printf("check ok");
		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
		jump2app=(iapfun)*(vu32*)(appxaddr);	
		MSR_MSP(*(vu32*)appxaddr);					
		jump2app();									
	}
}	
void SoftReset(void)
{
    __set_FAULTMASK(1); // 关闭所有中断
    NVIC_SystemReset(); // 复位
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	  if(GPIO_Pin==GPIO_PIN_0)
		{
			//iap_load_app(0x08000000);
			SoftReset();
//			HAL_NVIC_EnableIRQ(USART2_IRQn);
//			All_Run(CW,99999);
//			printf("i am app1");
//			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
		}
}
void comd_deal(char *comd,u8 len)
{
	char data[20][30];
	u8 j=0,k=0,i;
	for(i=0;i<len;i++)
	{
		if(comd[i]!='&')
		{
			data[j][k]=comd[i];
			k++;
		}
		else
		{
		  j++;
			k=0;
		}
	}
	if(strcmp(data[0],"motor")==0)
	{
		Locate_Rle(500,500,CW);
	}
	if(strcmp(data[0],"GPS")==0)
	{
		
	}
	if(strcmp(data[0],"updata")==0)
	{ 
		
	}
}
	 
int main(void)
{

	char str_la[20];
	char str_lo[20];
	char comd[100];
	u16 i,rxlen;
  u8 key=0XFF;  	
  SCB->VTOR = FLASH_BASE | 0x10000;//设置偏移量	
	HAL_Init();                     
	Stm32_Clock_Init(360,25,2,8);   
	delay_init(180);
	uart2_init(4800);
	uart_init(115200);
	Led_init();
	Exit_Init();
	Driver_Init();
	TIM8_OPM_RCR_Init(999,179);
//	while(1)
//	{
//		//接收命令
//	}
	usmart_dev.init(90);	
  printf("before");	
	delay_ms(5);
	if(SkyTra_Cfg_Rate(5)!=0)	 
	{
		do
		{
			uart2_init(4800);			
	  	SkyTra_Cfg_Prt(0);			
			uart2_init(4800);			
      key=SkyTra_Cfg_Tp(100000);	//延迟
		}while(SkyTra_Cfg_Rate(5)!=0&&key!=0);
		delay_ms(500);
	}		
    while(1)
	  {
			  if(com_flag==1)
				{
					com_flag=0;
				  for(i=0;i<USART_RX_CNT;i++)
					{
						comd[i]=USART_RX_BUF[i];
					}
					comd_deal(comd,USART_RX_CNT);
					USART_RX_CNT=0;
					printf("%s\n",comd);
				}
				if(USART2_RX_STA&0X8000)//flag
				{
					HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
					rxlen=USART2_RX_STA&0X7FFF;	
					for(i=0;i<rxlen;i++)GPS_TX_BUF[i]=USART2_RX_BUF[i];	   
					USART2_RX_STA=0;		   	    
					GPS_TX_BUF[i]=0;			    
					GPS_Analysis(&gpsx,(u8*)GPS_TX_BUF);	
          if((gpsx.latitude != 0x00000000)&&(gpsx.longitude != 0x00000000))
          {
						u32tostr(gpsx.latitude,str_la);
						u32tostr(gpsx.longitude,str_lo);
						HAL_NVIC_DisableIRQ(USART2_IRQn);
						printf("loca_gps:%s:%s:%s:%s",&gpsx.nshemi,str_la,&gpsx.ewhemi,str_lo);
					}			
				}
		}
}

