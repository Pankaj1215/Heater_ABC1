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

#ifndef __COMMON_H__
#define __COMMON_H__

/*
 *  NOTE: this includes all the generic lib to be used
 *
 */
#include <string.h>
#include <stdlib.h>

#define ERROR                   -1
#define SUCCESS                 0

#define FALSE                   -1
#define TRUE                    0

#define DEV_STAT_ERROR          -1      //unexpected problem
#define DEV_STAT_READY          0       //waiting for read/write
#define DEV_STAT_BUSY           1       //cannot accept command
#define DEV_STAT_READING        2       //receiving
#define DEV_STAT_WRITTING       3       //sending
#define DEV_STAT_SLEEP          4       //power saving
#define DEV_STAT_STANDBY        5       //this means need config
#define DEV_STAT_CUSTOM         100     //status for device specific

/*!
 * \fn int get_file(char* haysack, char* buff)
 * \brief parse the file in the url string
 * \param haysack raw string 
 * \param buff storage for the return string
 * \return returns success or fail if not found
 */
int get_file(char* haysack, char* buff);

/*!
 * \fn int get_path(char* haysack, char* buff)
 * \brief parse the path file in the url string
 * \param haysack raw string 
 * \param buff storage for the return string
 * \return returns success or fail if not found
 */
int get_path(char* haysack, char* buff);

/*!
 * \fn int get_url(char* haysack, char* buff)
 * \brief parse the url in the url string
 * \param haysack raw string 
 * \param buff storage for the return string
 * \return returns success or fail if not found
 */
int get_url(char* haysack, char* buff);

/*!
 * \fn int get_ip(char* haysack, char* buff)
 * \brief parse the ip in the string
 * \param haysack raw string 
 * \param buff storage for the return string
 * \return returns success or fail if not found
 */
int get_ip(char* haysack, char* buff);

/*!
 * \fn int get_port(char* haysack, char* buff)
 * \brief parse the port in the url string
 * \param haysack raw string 
 * \param buff storage for the return string
 * \return returns success or fail if not found
 */
int get_port(char* haysack, char* buff);

/*!
 * \fn int get_filepath(char* haysack, char* buff)
 * \brief parse the filepath in the url string
 * \param haysack raw string 
 * \param buff storage for the return string
 * \return returns success or fail if not found
 */
int get_filepath(char* haysack, char* buff);


#endif //__COMMON_H__
