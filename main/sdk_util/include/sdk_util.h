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

#ifndef __RTOS_UTIL_H__
#define __RTOS_UTIL_H__


/*!
 * \fn int create_thread(void (*thread)(void* param), char* label)
 * \brief create a new thread specifying the function to be called
 * \param thread function to be called of thread
 * \param label label for the thread
 * \return returns success, fail if cannot allocate
 */
int create_thread(void (*thread)(void* param), char* label);

/*!
 * \fn int create_thread_with_stackvalue(void (*thread)(void* param), char* label, int stackvalue)
 * \brief create a new thread specifying but the stack value must be indicated
 * \param thread function to be called of thread
 * \param label label for the thread
 * \param stackvalue stackvalue must be indicated
 * \return returns success, fail if cannot allocate
 */
int create_thread_with_stackvalue(void (*thread)(void* param), char* label, int stackvalue);

/*!
 * \fn int delay_sec(int sec)
 * \brief create delay in sec
 * \param sec value how long the delay in seconds
 * \return always success
 */
int delay_sec(int sec);

/*!
 * \fn int delay_milli(int milli)
 * \brief create delay in milliseconds
 * \param milli value how long the delay in seconds
 * \return always success
 */
int delay_milli(int milli);

/*!
 * \fn void reset_device(void)
 * \brief restart the device
 * \return always success
 */
void reset_device(void);

#endif //__RTOS_UTIL_H__
