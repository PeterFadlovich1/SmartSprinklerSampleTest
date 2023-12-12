/******************************************************************************
 * Copyright (C) 2023 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   ADC demo application
 * @details Continuously monitors the ADC channels
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>

#include <mxc.h>
#include "adc.h"

/***** Definitions *****/

/* Change to #undef USE_INTERRUPTS for polling mode */
// #define USE_INTERRUPTS
#define MOISTURE_READ_1 MXC_ADC_CH_2
#define RAIN_READ_1 MXC_ADC_CH_0 //1
#define RAIN_READ_2 MXC_ADC_CH_1

#define SENSOR_SELECT 1 // 1 for rain    2 for cap

/***** Globals *****/
#ifdef USE_INTERRUPTS
volatile unsigned int adc_done = 0;
#endif

static int32_t adc_val;
uint8_t overflow;

/***** Functions *****/
#if 0
#ifdef USE_INTERRUPTS
void adc_complete_cb(void *req, int adcRead)
{
    overflow = (adcRead == E_OVERFLOW ? 1 : 0);
    adc_val = adcRead;
    adc_done = 1;
    return;
}
void ADC_IRQHandler(void)
{
    MXC_ADC_Handler();
}
#endif
#endif


void initADC(mxc_adc_monitor_t monitor, mxc_adc_chsel_t chan, uint16_t hithresh){
    if (MXC_ADC_Init() != E_NO_ERROR) {
        printf("Error Bad Parameter\n");

    }

    /* Set up LIMIT0 to monitor high and low trip points */
    MXC_ADC_SetMonitorChannel(monitor, chan);
    MXC_ADC_SetMonitorHighThreshold(monitor, hithresh);
    MXC_ADC_SetMonitorLowThreshold(monitor, 0);
    MXC_ADC_EnableMonitor(monitor);

    //NVIC_EnableIRQ(ADC_IRQn);
}

void GPIOINIT(){
    mxc_gpio_cfg_t gpio_moisture_enable1;
    mxc_gpio_cfg_t gpio_moisture_enable2;

    mxc_gpio_cfg_t gpio_load_enable;

    gpio_moisture_enable1.port = MXC_GPIO0;
    gpio_moisture_enable1.mask = MXC_GPIO_PIN_21;
    gpio_moisture_enable1.pad = MXC_GPIO_PAD_NONE;
    gpio_moisture_enable1.func = MXC_GPIO_FUNC_OUT;
    gpio_moisture_enable1.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&gpio_moisture_enable1);

    gpio_moisture_enable2.port = MXC_GPIO0;
    gpio_moisture_enable2.mask = MXC_GPIO_PIN_22;
    gpio_moisture_enable2.pad = MXC_GPIO_PAD_NONE;
    gpio_moisture_enable2.func = MXC_GPIO_FUNC_OUT;
    gpio_moisture_enable2.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&gpio_moisture_enable2);

    gpio_load_enable.port = MXC_GPIO2;
    gpio_load_enable.mask = MXC_GPIO_PIN_6;
    gpio_load_enable.pad = MXC_GPIO_PAD_NONE;
    gpio_load_enable.func = MXC_GPIO_FUNC_OUT;
    gpio_load_enable.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&gpio_load_enable);


}


int main(void)
{

    initADC(MXC_ADC_MONITOR_0, MOISTURE_READ_1, 0x1ff); //One ADC channel for moisture, select with GPIO to amps. load switch as well?
    initADC(MXC_ADC_MONITOR_1, RAIN_READ_1, 0x1ff);
    initADC(MXC_ADC_MONITOR_2, RAIN_READ_2, 0x1ff); // Two ADC channels for rain, 

    GPIOINIT();

    MXC_GPIO_OutSet(MXC_GPIO2,  MXC_GPIO_PIN_6);

    int adcval;
    int rainCount = 0;
    int moistureCount = 0;

    while(1){
        if(SENSOR_SELECT == 1){
            if(rainCount%2 == 0){
                adcval = MXC_ADC_StartConversion(RAIN_READ_1);//*(1.22/1024);
                printf("Rain reading 1 %f: \n", adcval*(1.22/1024));
            }
            else{
                adcval = MXC_ADC_StartConversion(RAIN_READ_2);//*(1.22/1024);
                printf("Rain reading 2 %f: \n", adcval*(1.22/1024));
            }
            rainCount++;
            MXC_Delay(1000000);
        }
        else{
            if(moistureCount%2 == 0){
                MXC_GPIO_OutSet(MXC_GPIO0,  MXC_GPIO_PIN_21);
                MXC_Delay(500000);
                adcval = MXC_ADC_StartConversion(MOISTURE_READ_1);//*(1.22/1024);
                printf("ADC moisture 1 reading %f: \n", adcval*(1.22/1024));
            
                MXC_GPIO_OutClr(MXC_GPIO0,  MXC_GPIO_PIN_21);
            }
        else{
                MXC_GPIO_OutSet(MXC_GPIO0, MXC_GPIO_PIN_22);
                MXC_Delay(500000);
                adcval = MXC_ADC_StartConversion(MOISTURE_READ_1);//*(1.22/1024);
                printf("ADC moisture 2 reading %f: \n", adcval*(1.22/1024));
            
                MXC_GPIO_OutClr(MXC_GPIO0,  MXC_GPIO_PIN_22);
            }
            moistureCount++;
            MXC_Delay(500000);
        }

    }
    return 0;




    #if 0
    printf("********** ADC Example **********\n");

    /* Initialize ADC */
    if (MXC_ADC_Init() != E_NO_ERROR) {
        printf("Error Bad Parameter\n");

        while (1) {}
    }

    /* Set up LIMIT0 to monitor high and low trip points */
    MXC_ADC_SetMonitorChannel(MXC_ADC_MONITOR_0, MXC_ADC_CH_0);
    MXC_ADC_SetMonitorHighThreshold(MXC_ADC_MONITOR_0, 0x300);
    MXC_ADC_SetMonitorLowThreshold(MXC_ADC_MONITOR_0, 0x25);
    MXC_ADC_EnableMonitor(MXC_ADC_MONITOR_0);

#ifdef USE_INTERRUPTS
    NVIC_EnableIRQ(ADC_IRQn);
#endif

    while (1) {
        /* Flash LED when starting ADC cycle */
        LED_On(0);
        MXC_TMR_Delay(MXC_TMR0, MSEC(10));
        LED_Off(0);

        /* Convert channel 0 */
#ifdef USE_INTERRUPTS
        adc_done = 0;
        MXC_ADC_StartConversionAsync(MXC_ADC_CH_0, adc_complete_cb);

        while (!adc_done) {}
#else
        adc_val = MXC_ADC_StartConversion(MXC_ADC_CH_0);
        overflow = (adc_val == E_OVERFLOW ? 1 : 0);
#endif

        /* Display results on OLED display, display asterisk if overflow */
        printf("0: 0x%04x%s\n\n", adc_val, overflow ? "*" : " ");

        /* Determine if programmable limits on AIN0 were exceeded */
        if (MXC_ADC_GetFlags() & (MXC_F_ADC_INTR_LO_LIMIT_IF | MXC_F_ADC_INTR_HI_LIMIT_IF)) {
            printf(" %s Limit on AIN0 ",
                   (MXC_ADC_GetFlags() & MXC_F_ADC_INTR_LO_LIMIT_IF) ? "Low" : "High");
            MXC_ADC_ClearFlags(MXC_F_ADC_INTR_LO_LIMIT_IF | MXC_F_ADC_INTR_HI_LIMIT_IF);
        } else {
            printf("                   ");
        }

        printf("\n");

        /* Delay for 1/4 second before next reading */
        MXC_TMR_Delay(MXC_TMR0, MSEC(250));
    }
    #endif
}
