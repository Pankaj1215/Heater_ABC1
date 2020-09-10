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

#ifndef __VERSION_H__
#define __VERSION_H__

#define VERSION_MAJOR               (1)
#define VERSION_MINOR               (6)
#define VERSION_REVISION            (0)

/*!
 * \fn uint32_t get_version_major(void)
 * \brief get major version
 * \return returns major version 
 */
uint32_t get_version_major(void);

/*!
 * \fn uint32_t get_version_minor(void)
 * \brief get minor version
 * \return returns minor version
 */
uint32_t get_version_minor(void);

/*!
 * \fn uint32_t get_version_revision(void)
 * \brief get revision version
 * \return returns revision version
 */
uint32_t get_version_revision(void);

/*!
 * \fn size_t get_version(char *buf)
 * \brief get the version in string format
 * \param buff storage location for string version format
 * \return returns size of string version 
 */
size_t get_version(char *buf);


#endif //__VERSION_H__

