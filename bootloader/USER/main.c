#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "iap.h"
 
u8 rdata[1];
u8 buff[]="";
u32 oldcount=0;				   
u32 applenth=0;	 
extern u8 file_over;
extern u8 flag_file;
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
  GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      //ÉÏÉýÑØ´¥·¢
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	  if(GPIO_Pin==GPIO_PIN_0)
		{
			printf("update");
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
		}
}

int main(void)
{
    			    
	HAL_Init();                     
	Stm32_Clock_Init(360,25,2,8);   
	delay_init(180);
	uart_init(115200);
	Led_init();
	Exit_Init();
	printf("i am boot");
    while(1)
	  {
		  if(flag_file==1)
	   	{
				  HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
					applenth=USART_RX_CNT;
					oldcount=0;
					USART_RX_CNT=0;
					delay_ms(50);
					if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)//
			    {	 
				     iap_write_appbin(FLASH_APP1_ADDR,USART_RX_BUF,applenth);//¸üÐÂFLASH´úÂë   
						 printf("write ok!");
			    }
			    delay_ms(5000);
					if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//ÅÐ¶ÏÊÇ·ñÎª0X08XXXXXX.
			    {	 
						printf("load in!");
						delay_ms(50);
				    iap_load_app(FLASH_APP1_ADDR);//
			    }
         file_over=0;					
		  }
		}
}

