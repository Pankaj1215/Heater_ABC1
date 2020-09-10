/*
 *  Copyright 2020. Lucidtron Philippines. All Rights Reserved. Permission
 *  to use, copy, modify, and distribute the software and its documentation
 *  for education, research and non-for-profit purposes. without fee and without
 *  signed licensing agreement, is hereby granted, provided that the above
 *  copyright notice, this paragraph and the following two paragraphs appear
 *  in all copies, modification, and distributions. Contact The offfice of
 *  Lucidtron Philippines Inc. Unit D 2nd Floor GMV-Winsouth 2 104 
 *  East Science Avenue, Laguna Technopark, Biñan City, Laguna 4024.
 *  lucidtron@lucidtron.com (049) 302 7001 for commercial and licensing 
 *  opportunities.
 *
 *  IN NO EVENT SHALL LUCIDTRON BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 *  SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, 
 *  ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF 
 *  LUCIDTRON HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  LUCIDTRON SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO. THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 *  PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY,
 *  PROVIDED HEREUNDER IS PROVIDED "AS IS". LUCIDTRON HAS NO OBLIGATION TO PROVIDE
 *  MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENS, OR MODIFICATIONS.
 *
 */

#ifndef MAIN_TEMPSENSOR_H
#define MAIN_TEMPSENSOR_H

#define NTC_VCC_GPIO_NUM 19
#define NTC_ADC_IN_CHANNEL ADC1_CHANNEL_2
#define NTC_ADC_WIDTH ADC_WIDTH_12Bit
#define NTC_ADC_ATTEN ADC_ATTEN_DB_11
#define NTC_ADC_IN_CHANNEL_GPIO_NUM 38

#define NTC_SAMPLES_CNT 32
#define NTC_ADC_MAX_VAL 4096
#define NTC_BETA 3380
#define NTC_R_BALANCE 10000

#define V_REF   1100
//#define NTC_VCC_EN
//#define NTC_VCC_ALWAYS_ON
#define ADC_RESAMPLE_CNT_MAX 3
#define TEMP_MAX_DIFF_DEGC 0.5f
#define ROLLING_AVE_SAMPLES_CNT 16


/*!
 * \fn esp_err_t tempsensor_init(void)
 * \brief initialize temperature sensor device
 * \return returns success or fail if not found
 */
esp_err_t tempsensor_init(void);

/*!
 * \fn float tempsensor_get_temperature(void)
 * \brief get temperature 
 * \return returns temperature value
 */
float tempsensor_get_temperature(void);


#endif /* MAIN_TEMPSENSOR_H */

