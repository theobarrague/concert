/**
 * \file concert.h
 * \brief Library for interacting with payment terminals over a serial
          connection.
 * \author Théo BARRAGUÉ
 * \version 0.1
 * \date 2024/05/03
 *
 * This header file defines constants and functions for generating messages
 * to communicate with payment terminals over a serial connection.
 */

#ifndef CONCERT_H
#define CONCERT_H

#include <stddef.h>

// Error codes
#define CONCERT_ERROR -1
#define CONCERT_ERROR_INVALID_CASH_REGISTER_ID -2
#define CONCERT_ERROR_INVALID_AMOUNT -3
#define CONCERT_ERROR_INVALID_INDICATOR -4
#define CONCERT_ERROR_INVALID_MODE -5
#define CONCERT_ERROR_INVALID_TYPE -6
#define CONCERT_ERROR_INVALID_CURRENCY -7
#define CONCERT_ERROR_INVALID_PRIVATE -8
#define CONCERT_ERROR_INVALID_DELAY -9
#define CONCERT_ERROR_INVALID_AUTHORIZATION -10
#define CONCERT_ERROR_INVALID_DEVICE -11

// Indicator values
#define CONCERT_INDICATOR_INCLUDE "1"
#define CONCERT_INDICATOR_DO_NOT_INCLUDE "0"

// Payment modes
#define CONCERT_MODE_BANK_CARD "1"
#define CONCERT_MODE_CHEQUE "C"

// Transaction types
#define CONCERT_TYPE_DEBIT "0"
#define CONCERT_TYPE_CREDIT "1"

// Currency codes
#define CONCERT_CURRENCY_EUR "978"
#define CONCERT_CURRENCY_USD "840"

// Private date placeholder
#define CONCERT_PRIVATE_EMPTY "          "

// Delay options
#define CONCERT_DELAY_LATER "A010"
#define CONCERT_DELAY_NOW "A011"

// Authorization options
#define CONCERT_AUTHORIZATION_AUTO "B010"

/**
 * \brief Check if a string represents a number.
 *
 * \param string The string to check.
 * \return 1 if the string represents a number, 0 otherwise.
 */
int concert_is_number ( const char *string );

/**
 * \brief Calculate the longitudinal redundancy check (LRC) for a string.
 *
 * \param string The string for which to calculate the LRC.
 * \return The calculated LRC value.
 */
char concert_lrc ( const char *string );

/**
 * \brief Generate a message to be sent to a payment terminal.
 *
 * \param buffer The buffer to store the generated message.
 * \param buffer_size The size of the message buffer.
 * \param cash_register_id The ID of the cash register.
 * \param amount The transaction amount.
 * \param indicator The indicator for including/excluding private data.
 * \param mode The payment mode (bank card or cheque).
 * \param type The transaction type (credit or cancellation).
 * \param currency The currency code.
 * \param private Additional private data.
 * \param delay The delay option for the transaction.
 * \param authorization The authorization option for the transaction.
 * \return Message size on success, or an error code if any parameter is
 *         invalid.
 *
 * The caller is responsible for managing memory associated with the buffer.
 */
int concert_message ( char *buffer, size_t buffer_size,
                      const char *cash_register_id, const char *amount,
                      const char *indicator, const char *mode,
                      const char *type, const char *currency,
                      const char *private, const char *delay,
                      const char *authorization );

/**
 * \brief List USB serial devices.
 *
 * \param devices A pointer to an array of strings containing the names of USB
 *                serial devices.
 * \param max_devices The maximum number of devices to retrieve.
 * \return The number of devices found, or -1 on failure.
 *
 * The caller is responsible for managing memory associated with the devices
   array.
 */
int concert_list_serial_devices ( char **devices, int max_devices );

/**
 * \brief Send a simple payment request to a payment terminal.
 *
 * \param device The name or identifier of the payment terminal device.
 * \param amount The transaction amount.
 * \param currency The currency code for the transaction.
 * \return 0 on success, or a negative error code on failure.
 */
int concert_simple_request ( const char *device, unsigned int amount,
                             const char *currency );

/**
 * \brief Opens a serial device.
 *
 * \param device The path to the serial device to open.
 * \return A file descriptor for the device or a negative error code if an
 *         error occurs.
 */
int concert_device_open ( const char *device );

/**
 * \brief Closes a previously opened serial device.
 *
 * \param device The file descriptor of the device to close.
 * \return 1 on success, CONCERT_ERROR_* otherwise
 */
int concert_device_close ( int device );

/**
 * \brief Pings the device to check its availability.
 *
 * \param device The file descriptor of the payment terminal device.
 * \return 1 if the ping is successful, negative error code otherwise
 */
int concert_device_ping ( int device );

/**
 * \brief Writes data to a serial device.
 *
 * \param device The file descriptor of the serial device.
 * \param buffer The buffer containing the data to write.
 * \param message_size The size of the data to write.
 * \return The number of bytes written to the device or negative error code if
 *         an error occurs.
 *
 * The caller is responsible for managing memory associated with the buffer.
 */
int concert_device_write ( int device, char *buffer, size_t message_size );

/**
 * \brief Reads data from a serial device.
 *
 * \param device The file descriptor of the serial device.
 * \param buffer The buffer to store the read data.
 * \param buffer_size The size of the buffer.
 * \return The number of bytes read from the device or CONCERT_ERROR_* if an
 *         error occurs.
 *
 * The caller is responsible for managing memory associated with the buffer.
 */
int concert_device_read ( int device, char *buffer, size_t buffer_size );

#endif
