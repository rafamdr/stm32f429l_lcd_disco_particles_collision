// ---------------------------------------------------------------------------------------------------------------------
// Includes
// ---------------------------------------------------------------------------------------------------------------------
#include "global_includes.h"
#include "app.h"
#include <stdlib.h>

// ---------------------------------------------------------------------------------------------------------------------
// Private variables
// ---------------------------------------------------------------------------------------------------------------------
#define Background_Task_PRIO    ( tskIDLE_PRIORITY  + 10 )
#define Background_Task_STACK   ( 512 )

#define Demo_Task_PRIO          ( tskIDLE_PRIORITY  + 9 )
#define Demo_Task_STACK         ( 3048 )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
xTaskHandle                   Task_Handle;
xTaskHandle                   Demo_Handle;
xTimerHandle                  TouchScreenTimer;

// ---------------------------------------------------------------------------------------------------------------------
// Private functions
// ---------------------------------------------------------------------------------------------------------------------
static void initialize_peripherals(void)
{
    static RCC_ClocksTypeDef RCC_Clocks;
    /*!< At this stage the microcontroller clock setting is already configured, 
    this is done through SystemInit() function which is called from startup
    files (startup_stm32f429_439xx.s) before to branch to application main. 
    To reconfigure the default setting of SystemInit() function, refer to
    system_stm32f4xx.c file
    */  
    /* SysTick end of count event each 1ms */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
    
    /* Initialize the LCD */
    LCD_Init();
    /* Initialize the LCD Layers*/
    LCD_LayerInit();
    
    /* Enable the LTDC */
    LTDC_Cmd(ENABLE);
    
    /* Set LCD Background Layer  */
    LCD_SetLayer(LCD_FOREGROUND_LAYER);
    
    /* Clear the Background Layer */ 
    LCD_Clear(LCD_COLOR_WHITE);
}
// ---------------------------------------------------------------------------------------------------------------------

static void Demo_Task(void * pvParameters)
{  
    initialize_peripherals();
    app_init();
    
    while (1)
    {
        app_update();
    }
}


// ---------------------------------------------------------------------------------------------------------------------
// Exported functions
// ---------------------------------------------------------------------------------------------------------------------
static uint8_t ucHeap5[ configTOTAL_HEAP_SIZE ];

const HeapRegion_t xHeapRegions[] =
{
    { ( uint8_t * ) ucHeap5, configTOTAL_HEAP_SIZE },
    { NULL, 0 } /* Terminates the array. */
};

StackType_t    demo_stack_memory[Demo_Task_STACK];
StaticTask_t   demo_task_buffer;

int main(void)
{
  vPortDefineHeapRegions(xHeapRegions);
    
 
   xTaskCreateStatic(Demo_Task, 
                     (char const*)"GUI_DEMO", 
                     Demo_Task_STACK, 
                     NULL, 
                     Demo_Task_PRIO, 
                     demo_stack_memory, 
                     &demo_task_buffer);


  /* Start the FreeRTOS scheduler */
  vTaskStartScheduler();
}
// ---------------------------------------------------------------------------------------------------------------------
