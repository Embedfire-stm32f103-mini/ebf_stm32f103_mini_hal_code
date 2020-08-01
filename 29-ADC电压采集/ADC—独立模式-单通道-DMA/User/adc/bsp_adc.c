#include "./adc/bsp_adc.h"

__IO uint32_t ADC_ConvertedValue;
DMA_HandleTypeDef hdma_adcx;
ADC_HandleTypeDef ADC_Handle;
ADC_ChannelConfTypeDef ADC_Config;

static void Rheostat_ADC_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RHEOSTAT_ADC_CLK_ENABLE(); 
  // 使能 GPIO 时钟
  RHEOSTAT_ADC_GPIO_CLK_ENABLE();
        
  // 配置 IO
  GPIO_InitStructure.Pin = RHEOSTAT_ADC_GPIO_PIN;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;	    
//  GPIO_InitStructure.Pull = GPIO_NOPULL ; //不上拉不下拉
  HAL_GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStructure);		
}

static void Rheostat_ADC_Mode_Config(void)
{
    // ------------------DMA Init 结构体参数 初始化--------------------------
    // 开启DMA时钟
    RHEOSTAT_ADC_DMA_CLK_ENABLE();
    // 数据传输通道
     hdma_adcx.Instance = RHEOSTAT_ADC_DMA_STREAM;
  
     hdma_adcx.Init.Direction=DMA_PERIPH_TO_MEMORY;;            //外设到存储器
     hdma_adcx.Init.PeriphInc=DMA_PINC_DISABLE;                 //外设非增量模式
     hdma_adcx.Init.MemInc=DMA_MINC_ENABLE;    //                 //存储器增量模式 
     hdma_adcx.Init.PeriphDataAlignment=DMA_PDATAALIGN_HALFWORD;//外设数据长度:16位
     hdma_adcx.Init.MemDataAlignment=DMA_MDATAALIGN_HALFWORD;   //存储器数据长度:16位
     hdma_adcx.Init.Mode= DMA_CIRCULAR;                         //外设普通模式
     hdma_adcx.Init.Priority=DMA_PRIORITY_MEDIUM;               //中等优先级

    //初始化DMA流，流相当于一个大的管道，管道里面有很多通道
    HAL_DMA_Init(&hdma_adcx); 

    __HAL_LINKDMA( &ADC_Handle,DMA_Handle,hdma_adcx);
  
   //---------------------------------------------------------------------------
    RCC_PeriphCLKInitTypeDef ADC_CLKInit;
    // 开启ADC时钟
    ADC_CLKInit.PeriphClockSelection=RCC_PERIPHCLK_ADC;			//ADC外设时钟
    ADC_CLKInit.AdcClockSelection=RCC_ADCPCLK2_DIV8;			  //分频因子6时钟为72M/8=9MHz
    HAL_RCCEx_PeriphCLKConfig(&ADC_CLKInit);					      //设置ADC时钟
   
    ADC_Handle.Instance=RHEOSTAT_ADC;
    ADC_Handle.Init.DataAlign=ADC_DATAALIGN_RIGHT;             //右对齐
    ADC_Handle.Init.ScanConvMode=DISABLE;                      //非扫描模式
    ADC_Handle.Init.ContinuousConvMode=ENABLE;                 //连续转换
    ADC_Handle.Init.NbrOfConversion=1;                         //1个转换在规则序列中 也就是只转换规则序列1 
    ADC_Handle.Init.DiscontinuousConvMode=DISABLE;             //禁止不连续采样模式
    ADC_Handle.Init.NbrOfDiscConversion=0;                     //不连续采样通道数为0
    ADC_Handle.Init.ExternalTrigConv=ADC_SOFTWARE_START;       //软件触发
    HAL_ADC_Init(&ADC_Handle);                                 //初始化 
 
 //---------------------------------------------------------------------------
    ADC_Config.Channel      = RHEOSTAT_ADC_CHANNEL;
    ADC_Config.Rank         = 1;
    // 采样时间间隔	
    ADC_Config.SamplingTime = ADC_SAMPLETIME_239CYCLES_5 ;
    // 配置 ADC 通道转换顺序为1，第一个转换，采样时间为3个时钟周期
    HAL_ADC_ConfigChannel(&ADC_Handle, &ADC_Config);
		/* 执行ADC 自校准 */
    HAL_ADCEx_Calibration_Start(&ADC_Handle);
		
    HAL_ADC_Start_DMA(&ADC_Handle, (uint32_t*)&ADC_ConvertedValue, 1);
}



void Rheostat_Init(void)
{
	Rheostat_ADC_GPIO_Config();
	Rheostat_ADC_Mode_Config();
}

