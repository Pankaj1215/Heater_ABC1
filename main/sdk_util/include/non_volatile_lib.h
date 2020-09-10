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

#ifndef __NON_VOLATILE_H__
#define __NON_VOLATILE_H__

#define NVS_LUCIDTRON_NAMESPACE     "lucidtron"

/*!
 * \fn int nvs_storage_init(void)
 * \brief initialize the nvs before using, this should be called at bootup
 * \return returns success 
 */
int nvs_storage_init(void);

/*!
 * \fn int erase_storage_all(void)
 * \brief this will erase all the stored data in nvs
 * \return returns success 
 */
int erase_storage_all(void);

/*!
 * \fn int erase_string_in_storage(char* key)
 * \brief erase string data type stored in nvs
 * \param key key label used to store data
 * \return returns success or fail if not found
 */
int erase_string_in_storage(char* key);

/*!
 * \fn int set_string_to_storage(char* key, char* value)
 * \brief store the data together with the label key
 * \param key label used when storing data
 * \param value the raw value to be stored
 * \return returns success or fail if not found
 */
int set_string_to_storage(char* key, char* value);

/*!
 * \fn int get_string_from_storage(char* key, char* buffer)
 * \brief retrieve the data stored in nvs
 * \param key the label used when storing data
 * \param buff location to be used in fetching data
 * \return returns success or fail if not found
 */
int get_string_from_storage(char* key, char* buffer);

/*!
 * \fn int erase_integer_in_storage(char* key)
 * \brief erase integer data type stored in nvs
 * \param key label of the storage used
 * \return returns success or fail if not found
 */
int erase_integer_in_storage(char* key);

/*!
 * \fn int set_integer_to_storage(char* key, int value)
 * \brief store the integer data using key as reference label
 * \param key label used in storing data
 * \param value raw integer value
 * \return returns success or fail if not found
 */
int set_integer_to_storage(char* key, int value);

/*!
 * \fn int get_integer_from_storage(char* key, int* value)
 * \brief get the integer value from storage
 * \param key label used when storing integer
 * \param value location where to store the value
 * \return returns success or fail if not found
 */
int get_integer_from_storage(char* key, int* value);

/*!
 * \fn int erase_data_in_storage(char* key)
 * \brief erase the stored label 
 * \return returns success or fail if not found
 */
int erase_data_in_storage(char* key);

/*!
 * \fn int set_data_to_storage(char* key, void *data, size_t len)
 * \brief using void pointer as data type to be stored
 * \param key label of the value to be stored
 * \param data* pointer of the location of data
 * \param len lenght of data to be stored
 * \return returns success or fail if not found
 */
int set_data_to_storage(char* key, void *data, size_t len);

/*!
 * \fn int get_data_from_storage(char* key, void *data)
 * \brief get raw data from storage
 * \param key label of data stored
 * \param data* storage location of data to be fetched
 * \return returns success or fail if not found
 */
int get_data_from_storage(char* key, void *data);


#endif //__NON_VOLATILE_H__
