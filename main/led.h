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

#ifndef MAIN_LED_H
#define MAIN_LED_H

#define LEDC_TIMER_SRC LEDC_TIMER_0
#define LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_TIMER_FREQ 5000
#define LEDC_TIMER_RES LEDC_TIMER_10_BIT

#define LED1_GPIO_NUM 23
#define LED2_GPIO_NUM 18
#define NIGHT_LED_GPIO_NUM 5
#define LED_R_GPIO_NUM 12
#define LED_G_GPIO_NUM 13
#define LED_B_GPIO_NUM 2

#define LED1_CHANNEL_INDEX 0
#define LED2_CHANNEL_INDEX 1
#define NIGHT_LED_CHANNEL_INDEX 2
#define LED_R_CHANNEL_INDEX 3
#define LED_G_CHANNEL_INDEX 4
#define LED_B_CHANNEL_INDEX 5

#define LEDC_CHANNEL_CNT 6
#define LEDC_MAX_DUTY_VAL 1024
#define LED_MAX_BRIGHTNESS 100

#define LED_R_POL 1
#define LED_G_POL 1
#define LED_B_POL 1


/*!
 * \fn esp_err_t led_init(void)
 * \brief initialize led devices
 * \return returns success 
 */
esp_err_t led_init(void);

/*!
 * \fn esp_err_t led1_on(void)
 * \brief toggle on the led1
 * \return returns success 
 */
esp_err_t led1_on(void);

/*!
 * \fn esp_err_t led1_off(void)
 * \brief toggle off the led1
 * \return returns success 
 */
esp_err_t led1_off(void);

/*!
 * \fn esp_err_t led1_set_brightness(uint8_t br)
 * \brief set led1 brightness level
 * \param br brightness level 
 * \return returns success 
 */
esp_err_t led1_set_brightness(uint8_t br);

/*!
 * \fn esp_err_t led2_on(void)
 * \brief toggle on the led2
 * \return returns success 
 */
esp_err_t led2_on(void);

/*!
 * \fn esp_err_t led2_off(void)
 * \brief toggle off the led2
 * \return returns success 
 */
esp_err_t led2_off(void);

/*!
 * \fn esp_err_t led2_set_brightness(uint8_t br)
 * \brief set led2 brightness level
 * \param br brightness level 
 * \return returns success 
 */
esp_err_t led2_set_brightness(uint8_t br);

/*!
 * \fn esp_err_t night_led_on(void)
 * \brief toggle on night led
 * \return returns success 
 */
esp_err_t night_led_on(void);

/*!
 * \fn esp_err_t night_led_off(void)
 * \brief toggle off night led
 * \return returns success 
 */
esp_err_t night_led_off(void);

/*!
 * \fn esp_err_t night_led_set_brightness(uint8_t br)
 * \brief set night led brightness
 * \param br brightness level
 * \return returns success 
 */
esp_err_t night_led_set_brightness(uint8_t br);

/*!
 * \fn esp_err_t led_r_on(void)
 * \brief toggle led_r on
 * \return returns success 
 */
esp_err_t led_r_on(void);

/*!
 * \fn esp_err_t led_r_off(void)
 * \brief toggle led_r off
 * \return returns success 
 */
esp_err_t led_r_off(void);

/*!
 * \fn esp_err_t led_r_set_brightness(uint8_t br)
 * \brief set led_r brightness level
 * \param br brightness level
 * \return returns success 
 */
esp_err_t led_r_set_brightness(uint8_t br);

/*!
 * \fn esp_err_t led_g_on(void)
 * \brief toggle led_g on
 * \return returns success 
 */
esp_err_t led_g_on(void);

/*!
 * \fn esp_err_t led_g_off(void)
 * \brief toggle led_g off
 * \return returns success 
 */
esp_err_t led_g_off(void);

/*!
 * \fn esp_err_t led_g_set_brightness(uint8_t br)
 * \brief set led_g brightness 
 * \param br brightness level
 * \return returns success 
 */
esp_err_t led_g_set_brightness(uint8_t br);

/*!
 * \fn esp_err_t led_b_on(void)
 * \brief toggle led_b on
 * \return returns success 
 */
esp_err_t led_b_on(void);

/*!
 * \fn esp_err_t led_b_off(void)
 * \brief toggle led_b off
 * \return returns success 
 */
esp_err_t led_b_off(void);

/*!
 * \fn esp_err_t led_b_set_brightness(uint8_t br)
 * \brief set led_b brightness level
 * \param br brightness level
 * \return returns success 
 */
esp_err_t led_b_set_brightness(uint8_t br);

#endif /* MAIN_LED_H */
