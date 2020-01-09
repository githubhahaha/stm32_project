#include "usart.h"
#include "timer.h"
#include "delay.h"
#pragma import(__use_no_semihosting)			 
//标准库需要的支持函数				 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;	   
//定义_sys_exit()以避免使用半主机模式	
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
	USART3->DR = (u8) ch;	  
	return ch;
}

UART_HandleTypeDef usart3_handler; 	
UART_HandleTypeDef usart2_handler; 
u8 USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20001000)));//接收缓冲,最大USART_REC_LEN个字节,起始地址为0X20001000.  

//GPS 
u16 USART2_RX_STA=0; 
//串口发送缓存区 	
__align(8) u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; 	//发送缓冲,最大USART3_MAX_SEND_LEN字节  	  
//串口接收缓存区 	
u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.


u16 USART_RX_STA=0;	   //接收状态标记	
u32 USART_RX_CNT=0;			//接收的字节数 

u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲

u8 com_flag=0;//普通指令
 
void uart_init(u32 bound)//跟NB通信
{
	usart3_handler.Instance=USART3;						//USART3
	usart3_handler.Init.BaudRate=bound;					//
	usart3_handler.Init.WordLength=UART_WORDLENGTH_8B;   
	usart3_handler.Init.StopBits=UART_STOPBITS_1;		
	usart3_handler.Init.Parity=UART_PARITY_NONE;			
	usart3_handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   
	usart3_handler.Init.Mode=UART_MODE_TX_RX;		   
	HAL_UART_Init(&usart3_handler);						
}
void uart2_init(u32 bound)//跟GPS通信
{	
	//UART 初始化设置
	usart2_handler.Instance=USART2;					    //USART2
	usart2_handler.Init.BaudRate=bound;				    //波特率
	usart2_handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	usart2_handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
	usart2_handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
	usart2_handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	usart2_handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	HAL_UART_Init(&usart2_handler);					    //HAL_UART_Init()会使能UART1
}
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
   
	if(huart->Instance==USART3)
	{
		GPIO_InitTypeDef GPIO_Initure1;
		__HAL_RCC_GPIOB_CLK_ENABLE();			
		__HAL_RCC_USART3_CLK_ENABLE();		
	
		GPIO_Initure1.Pin=GPIO_PIN_10;			//PB10
		GPIO_Initure1.Mode=GPIO_MODE_AF_PP;	
		GPIO_Initure1.Pull=GPIO_PULLUP;		
		GPIO_Initure1.Speed=GPIO_SPEED_FREQ_HIGH;		
		GPIO_Initure1.Alternate=GPIO_AF7_USART3;//????
		HAL_GPIO_Init(GPIOB,&GPIO_Initure1);	   	

		GPIO_Initure1.Pin=GPIO_PIN_11;			//PB11
		HAL_GPIO_Init(GPIOB,&GPIO_Initure1);	   	
		__HAL_UART_DISABLE_IT(huart,UART_IT_TC);
		
		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);
		HAL_NVIC_EnableIRQ(USART3_IRQn);	
		HAL_NVIC_SetPriority(USART3_IRQn,3,3);	
	}
	if(huart->Instance==USART2)
	{
		  //GPIO端口设置
		GPIO_InitTypeDef GPIO_Initure;
	
		__HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
		__HAL_RCC_USART2_CLK_ENABLE();			//使能USART2时钟
	
		GPIO_Initure.Pin=GPIO_PIN_2;			//PA2
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FAST;		//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART2;	//复用为USART2
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA2

		GPIO_Initure.Pin=GPIO_PIN_3;			//PA3
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA3

		__HAL_UART_ENABLE_IT(huart,UART_IT_RXNE);		//开启接收中断
		HAL_NVIC_EnableIRQ(USART2_IRQn);				//使能USART2中断
		HAL_NVIC_SetPriority(USART2_IRQn,2,3);			//抢占优先级2，子优先级3	
		TIM7_Int_Init(1000-1,9000-1);		//100ms中断
		USART2_RX_STA=0;		//清零
		TIM7->CR1&=~(1<<0);        //关闭定时器7
	}
}

void USART3_IRQHandler(void)   //跟NB通信
{
	u8 Res;
	if((__HAL_UART_GET_FLAG(&usart3_handler,UART_FLAG_RXNE)!=RESET))  //接收中断
	{
	  HAL_UART_Receive(&usart3_handler,&Res,1,1000); 
		if(USART_RX_CNT<USART_REC_LEN)
		{
			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
			if(Res=='!')
			{
				com_flag=1;
			}
			else
			{
				USART_RX_BUF[USART_RX_CNT]=Res;
				USART_RX_CNT++;
			}
				
		}
	}
	HAL_UART_IRQHandler(&usart3_handler);
}
void USART2_IRQHandler(void)
{
	u8 res;	      
	if(__HAL_UART_GET_FLAG(&usart2_handler,UART_FLAG_RXNE)!=RESET)//接收到数据
	{	 
		res=USART2->DR; 			 
		if((USART2_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART2_RX_STA<USART2_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM7->CNT=0;         				//计数器清空	
				if(USART2_RX_STA==0) 				//使能定时器7的中断 
				{
					TIM7->CR1|=1<<0;     			//使能定时器7
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	}  				 											 
}   
	

