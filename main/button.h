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

#ifndef MAIN_BUTTON_H
#define MAIN_BUTTON_H


#define BUTTON_UP_GPIO_NUM 26
#define BUTTON_DOWN_GPIO_NUM 25
#define BUTTON_POWER_BACK_GPIO_NUM 34
#define BUTTON_TIMER_FORWARD_GPIO_NUM 35


typedef void (*button_cb_t) (int level);


/*!
 * \fn esp_err_t button_init(void)
 * \brief initialize button device
 * \return returns success 
 */
esp_err_t button_init(void);

/*!
 * \fn esp_err_t button_up_set_cb(button_cb_t cb)
 * \brief set callback when button up was pressed
 * \param callback function 
 * \return returns success 
 */
esp_err_t button_up_set_cb(button_cb_t cb);

/*!
 * \fn esp_err_t button_down_set_cb(button_cb_t cb)
 * \brief set callback when button down was pressed
 * \param callback function 
 * \return returns success 
 */
esp_err_t button_down_set_cb(button_cb_t cb);

/*!
 * \fn esp_err_t button_power_back_set_cb(button_cb_t cb)
 * \brief set callback when button power_back was pressed
 * \param callback function 
 * \return returns success 
 */
esp_err_t button_power_back_set_cb(button_cb_t cb);

/*!
 * \fn esp_err_t button_timer_forward_set_cb(button_cb_t cb)
 * \brief set callback when button timer_forward was pressed
 * \param callback function 
 * \return returns success 
 */
esp_err_t button_timer_forward_set_cb(button_cb_t cb);

/*!
 * \fn int button_up_get_level(void)
 * \brief button up was pressed
 * \return returns success 
 */
int button_up_get_level(void);

/*!
 * \fn int button_down_get_level(void)
 * \brief button down was pressed
 * \return returns success 
 */
int button_down_get_level(void);

/*!
 * \fn int button_power_back_get_level(void)
 * \brief button back was pressed
 * \return returns success 
 */
int button_power_back_get_level(void);

/*!
 * \fn int button_timer_forward_get_level(void)
 * \brief button forward was pressed
 * \return returns success 
 */
int button_timer_forward_get_level(void);


#endif /* MAIN_BUTTON_H */

