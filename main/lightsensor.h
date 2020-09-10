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

#ifndef MAIN_LIGHTSENSOR_H
#define MAIN_LIGHTSENSOR_H

#define LDR_ADC_IN_CHANNEL ADC1_CHANNEL_1
#define LDR_ADC_WIDTH ADC_WIDTH_12Bit
#define LDR_ADC_ATTEN ADC_ATTEN_DB_11
#define LDR_ADC_IN_CHANNEL_GPIO_NUM 37

#define LDR_SAMPLES_CNT 32
#define LDR_ADC_MAX_VAL 400
#define LDR_RESAMPLE_CNT_MAX 3
#define LDR_MAX_DIFF_ADC 100
#define LDR_ROLLING_AVE_SAMPLES_CNT 8


/*!
 * \fn esp_err_t lightsensor_init(void)
 * \brief initialize light sensor device
 * \return returns success 
 */
esp_err_t lightsensor_init(void);

/*!
 * \fn int lightsensor_get_val(void)
 * \brief get the light sensor value
 * \return returns the light sensor value
 */
int lightsensor_get_val(void);


#endif /* MAIN_LIGHTSENSOR_H */

