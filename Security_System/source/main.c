/*******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for MQTT Client Example for ModusToolbox.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2020-2024, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/* Header file includes */
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "mqtt_task.h"
#include "subscriber_task.h"

#include "FreeRTOS.h"
#include "task.h"

#include "queue.h"
#include "capsense_task.h"
#include "stdio.h"

#include "cy_pdl.h"
#include "mtb_thermistor_ntc_gpio.h"

#define TASK_CAPSENSE_PRIORITY (configMAX_PRIORITIES - 1)
#define TASK_LED_PRIORITY (configMAX_PRIORITIES - 2)

/* Stack sizes of user tasks in this project */
#define TASK_CAPSENSE_STACK_SIZE (256u)
#define TASK_LED_STACK_SIZE (configMINIMAL_STACK_SIZE)

/* Queue lengths of message queues used in this project */
#define SINGLE_ELEMENT_QUEUE (1u)

/*******************************************************************************
* Macros
*******************************************************************************/
/* LED blink timer clock value in Hz  */
#define LED_BLINK_TIMER_CLOCK_HZ          (10000)

/* LED blink timer period value */
#define LED_BLINK_TIMER_PERIOD            (9999)

/* Define the pins for the thermistor
 * See https://www.infineon.com/dgdl/Infineon-CY8CPROTO-062-4343W_Schematic-PCBDesignData-v01_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0f010c6d183a
 * for reference.
 */
#define THERM_VDD_PIN    P10_3
#define THERM_GND_PIN    P10_0
#define THERM_OUT_PIN    P10_2

/*******************************************************************************
* Global Variables
*******************************************************************************/
bool timer_interrupt_flag = false;
bool led_blink_active_flag = true;

cyhal_adc_t adc;

mtb_thermistor_ntc_gpio_t thermistor;

/* These constants are taken from the file 'CY8CKIT_028_epd8pins.h'
 * (https://github.com/Infineon/CY8CKIT-028-EPD/blob/master/cy8ckit_028_epd_pins.h),
 * as that board uses the same NCP18XH103F03RB thermistor. This is validated by the
 * datasheet of this thermistor, which can be found at
 * https://www.murata.com/en-eu/api/pdfdownloadapi?cate=&partno=NCP18XH103F03RB.
 */
mtb_thermistor_ntc_gpio_cfg_t thermistor_cfg = {
    .r_ref = (float)(10000),
    .b_const = (float)(3380),
    .r_infinity = (float)(0.1192855),
};

/* Variable for storing character read from terminal */
uint8_t uart_read_value;

/* Timer object used for blinking the LED */
cyhal_timer_t led_blink_timer;
/* Include serial flash library and QSPI memory configurations only for the
 * kits that require the Wi-Fi firmware to be loaded in external QSPI NOR flash.
 */
#if defined(CY_DEVICE_PSOC6A512K)
#include "cy_serial_flash_qspi.h"
#include "cycfg_qspi_memslot.h"
#endif


#define PAYLOAD_SIZE 128
#define MAX_TOPIC_LENGTH 50
#define MAX_TOPICS 10  // Maximum number of topics


void timer_init(void);
static void isr_timer(void *callback_arg, cyhal_timer_event_t event);


void lock_task(void *arg)
{

	(void)arg;

	cyhal_pwm_t pwm_obj;
	cyhal_pwm_init(&pwm_obj, P5_4, NULL);
	cyhal_pwm_start(&pwm_obj);

	while(mqttConnected == 0){}
	//vTaskDelay(20000);
	/* Initialize hardware */
	char topic[] = "lock";
	char data[128];
	int value = 0;

	float degrees = 0;
	float dutycycle = 0;
	subscriber_data_t subscriber_q_data;
	subscriber_q_data.topic = topic;
	xQueueSend(subscriber_task_q, &subscriber_q_data, portMAX_DELAY);
	vTaskDelay(1000);
	cyhal_gpio_write_internal((P5_4), 0);
	for (;;)
	{
		QueueHandle_t queue1 = get_queue_for_topic(topic);
		xQueueReceive(queue1, &data, pdMS_TO_TICKS(portMAX_DELAY));

		if(data[0] == '0'){ //unlock
			value = 0;
			degrees = 160;
		}else if(data[0] == '1'){ //lock
			value = 1;
			degrees = 70;
		}else{continue;}


		dutycycle = ((1.0/40)+(1.0/10)*(degrees/180))*100;


		cyhal_pwm_set_duty_cycle(&pwm_obj, dutycycle, 50);
		vTaskDelay(1000);
		cyhal_pwm_set_duty_cycle(&pwm_obj, 0, 50);


		//cyhal_gpio_write_internal((P5_4), 1);

		//cyhal_gpio_write_internal((P5_4), 0);
		vTaskDelay(10);
	}
}

void buttonTask(void *arg)
{
	(void)arg;


	cyhal_gpio_init(P5_5, CYHAL_GPIO_DRIVE_PULLUP,
		                    CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);

	while(mqttConnected == 0){}

	char topic[] = "button";
	char data[128] = "0";
	int value = 0;
	subscriber_data_t subscriber_q_data;
	subscriber_q_data.topic = topic;
	xQueueSend(subscriber_task_q, &subscriber_q_data, portMAX_DELAY);
	vTaskDelay(1000);
	int prevValue = 0;
	for (;;)
	{
		while(1){
			value = cyhal_gpio_read_internal(P5_5);
			if(value != prevValue){break;}
		}

		prevValue = value;
		//sprintf(data, "%d", value);
		if(value == 0){
			data[0] = '0';
		}else{
			data[0] = '1';
		}
		//data[1] = 0;
		PublishMessage( data, topic);
		vTaskDelay(10);
	}
}


void buzzer_task(void *arg)
{

	(void)arg;

	cyhal_gpio_init(P5_6, CYHAL_GPIO_DIR_OUTPUT,
			CYHAL_GPIO_DRIVE_STRONG, 0);

	while(mqttConnected == 0){}
	//vTaskDelay(20000);
	/* Initialize hardware */
	char topic[] = "buzzer";
	char data[128] = "0";
	int value = 0;

	subscriber_data_t subscriber_q_data;
	subscriber_q_data.topic = topic;
	xQueueSend(subscriber_task_q, &subscriber_q_data, portMAX_DELAY);
	vTaskDelay(1000);
	for (;;)
	{
		QueueHandle_t queue1 = get_queue_for_topic(topic);
		xQueueReceive(queue1, &data, pdMS_TO_TICKS(portMAX_DELAY));

		if(data[0] == '0'){
			value = 0;
		}else if(data[0] == '1'){
			value = 1;
		}else{continue;}

		cyhal_gpio_write_internal((P5_6), value);
		//PublishMessage( data, topic);
		vTaskDelay(10);
	}
}






void ledTask(void *arg)
{

	(void)arg;

	cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
	                        CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

	while(mqttConnected == 0){}
	/* Initialize hardware */
	char topic[] = "led";
	char data[128] = "3";
	int value = 0;

	//printf("Value = %s \n", data);

	//sscanf(data, "%d", &value);

	//int value = atoi(data);
	//printf("Value2 = %d \n", value);

	//subscribe_to_topic(topic);
	subscriber_data_t subscriber_q_data;
	subscriber_q_data.topic = topic;
	xQueueSend(subscriber_task_q, &subscriber_q_data, portMAX_DELAY);
	vTaskDelay(1000);
	for (;;)
	{
		QueueHandle_t queue1 = get_queue_for_topic(topic);
		xQueueReceive(queue1, &data, pdMS_TO_TICKS(portMAX_DELAY));

		//sscanf(data, "%d", &value);

		if(data[0] == '1'){
			value = 0;
		}else{
			value = 1;
		}

		cyhal_gpio_write_internal((P13_7), value);
		vTaskDelay(100);
	}
}


void thermistorTask(void *arg)
{
	(void)arg;
	while(mqttConnected == 0){}
	/* Initialize hardware */
	char topic[] = "thermistor";
	char data[20];
	int value = 0;
	//subscribe_to_topic(topic);
	subscriber_data_t subscriber_q_data;
	subscriber_q_data.topic = topic;
	xQueueSend(subscriber_task_q, &subscriber_q_data, portMAX_DELAY);
	for (;;)
	{
        float temperature = mtb_thermistor_ntc_gpio_get_temp(&thermistor);
		sprintf(data, "%.2f", temperature);
        PublishMessage( data, topic);
		vTaskDelay(100000);
	}
}

void pirTask(void *arg)
{
	(void)arg;

	int pirValue;

    // Initialize pin P0_0 as an input
    cyhal_gpio_init(P8_5, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);

	for (;;)
	{
		pirValue = cyhal_gpio_read(P8_5);
		if(pirValue == 1){
			printf("PIR ACTIVATED\n");
		}
	}
}




/*
typedef struct {
    char *topic;               // Topic name
    QueueHandle_t queue;       // Queue for this topic
} topic_queue_entry_t ;
*/

//topic_queue_entry_t topic_queues[MAX_TOPICS];



int main()
{
    cy_rslt_t result;

#if defined (CY_DEVICE_SECURE)
    cyhal_wdt_t wdt_obj;

    /* Clear watchdog timer so that it doesn't trigger a reset */
    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif /* #if defined (CY_DEVICE_SECURE) */
    /* Initialize the board support package. */
    
    result = cybsp_init();
    CY_ASSERT(CY_RSLT_SUCCESS == result);

    /* To avoid compiler warnings. */
    (void) result;

    /* Enable global interrupts. */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port. */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                        CY_RETARGET_IO_BAUDRATE);
    CY_ASSERT(result == CY_RSLT_SUCCESS);

    /* Intialize adc */
    result = cyhal_adc_init(&adc, THERM_OUT_PIN, NULL);
    CY_ASSERT(result == CY_RSLT_SUCCESS);


    /* Initialize thermistor */
    result = mtb_thermistor_ntc_gpio_init(&thermistor, &adc,
        THERM_GND_PIN, THERM_VDD_PIN, THERM_OUT_PIN,
        &thermistor_cfg, MTB_THERMISTOR_NTC_WIRING_VIN_NTC_R_GND);
    CY_ASSERT(result == CY_RSLT_SUCCESS);


#if defined(CY_DEVICE_PSOC6A512K)
    /* Initialize the QSPI serial NOR flash with clock frequency of 50 MHz. */
    const uint32_t bus_frequency = 50000000lu;
    cy_serial_flash_qspi_init(smifMemConfigs[0], CYBSP_QSPI_D0, CYBSP_QSPI_D1,
                                  CYBSP_QSPI_D2, CYBSP_QSPI_D3, NC, NC, NC, NC,
                                  CYBSP_QSPI_SCK, CYBSP_QSPI_SS, bus_frequency);

    /* Enable the XIP mode to get the Wi-Fi firmware from the external flash. */
    cy_serial_flash_qspi_enable_xip(true);
#endif

    /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen. */
    printf("\x1b[2J\x1b[;H");
    printf("===============================================================\n");
#if defined(COMPONENT_CM0P)
    printf("CE229889 - MQTT Client running on CM0+\n");
#endif

#if defined(COMPONENT_CM4)
    printf("CE229889 - MQTT Client running on CM4\n");
#endif

#if defined(COMPONENT_CM7)
    printf("CE229889 - MQTT Client running on CM7\n");
#endif
    printf("===============================================================\n\n");


     capsense_command_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(capsense_command_t));


	 led_command_data_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(uint32_t));
	   /* Create the user tasks. See the respective task definition for more
		* details of these tasks.
		*/
	 slider_data_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(uint32_t));


	 // Dynamic array of topic queues
	 //topic_queue_entry_t *topic_queues = NULL;




	 //led_subscribe_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(uint32_t));



	 //publisher_task_q = xQueueCreate(PUBLISHER_TASK_QUEUE_LENGTH, sizeof(publisher_data_t));



	 /* Create the MQTT Client task. */
	 xTaskCreate(mqtt_client_task, "MQTT Client task", MQTT_CLIENT_TASK_STACK_SIZE*2,
				 NULL, MQTT_CLIENT_TASK_PRIORITY, NULL);

	 printf("idk \n");

	//vTaskDelay(10000);

 	 xTaskCreate(task_capsense, "CapSense Task", TASK_CAPSENSE_STACK_SIZE, NULL, TASK_CAPSENSE_PRIORITY, NULL);


 	xTaskCreate(lock_task , // Task function
 			"Task Name2", // Task name
 			1024, // Task stack size
 			NULL, // Parameters passed to task
 			1, // Task priority
 			NULL); // Task handle

	xTaskCreate(buzzer_task , // Task function
			"Task Name2", // Task name
			1024, // Task stack size
			NULL, // Parameters passed to task
			1, // Task priority
			NULL); // Task handle

	xTaskCreate(buttonTask , // Task function
			"Task Name3", // Task name
			1024, // Task stack size
			NULL, // Parameters passed to task
			1, // Task priority
			NULL); // Task handle
	xTaskCreate(thermistorTask , // Task function
			"Task Name4", // Task name
			1024, // Task stack size
			NULL, // Parameters passed to task
			1, // Task priority
			NULL); // Task handle
	xTaskCreate(ledTask , // Task function
			"Task Name5", // Task name
			1024*2, // Task stack size
			NULL, // Parameters passed to task
			1, // Task priority
			NULL); // Task handle
	xTaskCreate(pirTask , // Task function
				"Task Name6", // Task name
				1024*2, // Task stack size
				NULL, // Parameters passed to task
				1, // Task priority
				NULL); // Task handle






    /* Start the FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Should never get here. */
    CY_ASSERT(0);
}

/*******************************************************************************
* Function Name: timer_init
********************************************************************************
* Summary:
* This function creates and configures a Timer object. The timer ticks
* continuously and produces a periodic interrupt on every terminal count
* event. The period is defined by the 'period' and 'compare_value' of the
* timer configuration structure 'led_blink_timer_cfg'. Without any changes,
* this application is designed to produce an interrupt every 1 second.
*
* Parameters:
*  none
*
* Return :
*  void
*
*******************************************************************************/
 void timer_init(void)
 {
    cy_rslt_t result;

    const cyhal_timer_cfg_t led_blink_timer_cfg =
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = LED_BLINK_TIMER_PERIOD,   /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = true,              /* Run timer indefinitely */
        .value = 0                          /* Initial value of counter */
    };

    /* Initialize the timer object. Does not use input pin ('pin' is NC) and
     * does not use a pre-configured clock source ('clk' is NULL). */
    result = cyhal_timer_init(&led_blink_timer, NC, NULL);

    /* timer init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Configure timer period and operation mode such as count direction,
       duration */
    cyhal_timer_configure(&led_blink_timer, &led_blink_timer_cfg);

    /* Set the frequency of timer's clock source */
    cyhal_timer_set_frequency(&led_blink_timer, LED_BLINK_TIMER_CLOCK_HZ);

    /* Assign the ISR to execute on timer interrupt */
    cyhal_timer_register_callback(&led_blink_timer, isr_timer, NULL);

    /* Set the event on which timer interrupt occurs and enable it */
    cyhal_timer_enable_event(&led_blink_timer, CYHAL_TIMER_IRQ_TERMINAL_COUNT,
                              7, true);

    /* Start the timer with the configured settings */
    cyhal_timer_start(&led_blink_timer);
 }


/*******************************************************************************
* Function Name: isr_timer
********************************************************************************
* Summary:
* This is the interrupt handler function for the timer interrupt.
*
* Parameters:
*    callback_arg    Arguments passed to the interrupt callback
*    event            Timer/counter interrupt triggers
*
* Return:
*  void
*******************************************************************************/
static void isr_timer(void *callback_arg, cyhal_timer_event_t event)
{
    (void) callback_arg;
    (void) event;

    /* Set the interrupt flag and process it from the main while(1) loop */
    timer_interrupt_flag = true;
}





/* [] END OF FILE */
