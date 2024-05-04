#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "concert.h"

int concert_is_number ( const char *string )
{
    while ( *string )
    {
        if ( !isdigit ( *string ) )
        {
            return 0;
        }

        string++;
    }

    return 1;
}

char concert_lrc ( const char *string )
{
    char lrc = 0;

    while ( *string )
    {
        lrc ^= *string;
        string++;
    }

    return lrc;
}

int concert_message ( char *buffer, size_t buffer_size,
                      const char *cash_register_id, const char *amount,
                      const char *indicator, const char *mode,
                      const char *type, const char *currency,
                      const char *private, const char *delay,
                      const char *authorization )
{
    if ( cash_register_id == NULL || strlen ( cash_register_id ) != 2
            || !concert_is_number ( cash_register_id ) )
    {
        fprintf ( stderr, "Error in cash register ID validation" );
        return CONCERT_ERROR_INVALID_CASH_REGISTER_ID;
    }

    if ( amount == NULL || strlen ( amount ) != 8
            || !concert_is_number ( amount ) )
    {
        fprintf ( stderr, "Error in amount validation" );
        return CONCERT_ERROR_INVALID_AMOUNT;
    }

    if ( indicator == NULL || strlen ( indicator ) != 1 )
    {
        fprintf ( stderr, "Error in indicator validation" );
        return CONCERT_ERROR_INVALID_INDICATOR;
    }

    if ( mode == NULL || strlen ( mode ) != 1
            || ( strcmp ( mode, CONCERT_MODE_CHEQUE ) != 0
                 && strcmp ( mode, CONCERT_MODE_BANK_CARD ) != 0 ) )
    {
        fprintf ( stderr, "Error in mode validation" );
        return CONCERT_ERROR_INVALID_MODE;
    }

    if ( type == NULL || strlen ( type ) != 1
            || ( strcmp ( type, CONCERT_TYPE_DEBIT ) != 0
                 && strcmp ( type, CONCERT_TYPE_CREDIT ) != 0 ) )
    {
        fprintf ( stderr, "Error in type validation" );
        return CONCERT_ERROR_INVALID_TYPE;
    }

    if ( currency == NULL || strlen ( currency ) != 3 )
    {
        fprintf ( stderr, "Error in currency validation" );
        return CONCERT_ERROR_INVALID_CURRENCY;
    }

    if ( private == NULL || strlen ( private ) != 10 )
    {
        fprintf ( stderr, "Error in private data validation" );
        return CONCERT_ERROR_INVALID_PRIVATE;
    }

    if ( delay == NULL || strlen ( delay ) != 4
            || ( strcmp ( delay, CONCERT_DELAY_NOW ) != 0
                 && strcmp ( delay, CONCERT_DELAY_LATER ) != 0 ) )
    {
        fprintf ( stderr, "Error in delay validation" );
        return CONCERT_ERROR_INVALID_DELAY;
    }

    if ( authorization == NULL || strlen ( authorization ) != 4
            || strcmp ( mode, CONCERT_AUTHORIZATION_AUTO ) != 0 )
    {
        fprintf ( stderr, "Error in authorization validation" );
        return CONCERT_ERROR_INVALID_AUTHORIZATION;
    }

    memset ( buffer, 0, 37 );

    if ( snprintf ( buffer, buffer_size, "%s%s%s%s%s%s%s%s%s%c",
                    cash_register_id, amount, indicator, mode, type, currency,
                    private, delay, authorization, 0x03 ) < 0 )
    {
        perror ( "Encoding error occurs" );
        return -1;
    }

    if ( snprintf ( buffer, buffer_size, "%c%s%c", 0x02, buffer,
                    concert_lrc ( buffer ) ) )
    {
        perror ( "Encoding error occurs" );
        return -1;
    }

    return 37;
}

int concert_list_serial_devices ( char **devices, int max_devices )
{
    DIR *dir;
    struct dirent *entry;
    int count = 0;
    *devices = NULL;

    dir = opendir ( "/dev" );

    if ( dir == NULL )
    {
        perror ( "Error opening serial device directory" );
        return -1;
    }


    count = 0;

    while ( ( entry = readdir ( dir ) ) != NULL )
    {
        if ( entry->d_type == DT_LNK
                && strstr ( entry->d_name, "tty.usbmodem" ) != NULL
                && count < max_devices )
        {
            devices[count] = strdup ( entry->d_name );

            if ( devices[count] == NULL )
            {
                closedir ( dir );
                perror ( "Error duplicating device name" );

                return CONCERT_ERROR;
            }

            count++;
        }
    }

    closedir ( dir );
    return count;
}

int concert_simple_request ( const char *device, unsigned int amount,
                             const char *currency )
{
    if ( device == NULL )
    {
        return CONCERT_ERROR_INVALID_DEVICE;
    }

    char amount_str[9] = {0};
    snprintf ( amount_str, sizeof ( amount_str ), "%08u", amount );

    char buffer[BUFSIZ];
    int message_size = concert_message ( buffer, BUFSIZ, "01", amount_str,
                                         CONCERT_INDICATOR_DO_NOT_INCLUDE,
                                         CONCERT_MODE_BANK_CARD, CONCERT_TYPE_CREDIT,
                                         currency, CONCERT_PRIVATE_EMPTY,
                                         CONCERT_DELAY_NOW,
                                         CONCERT_AUTHORIZATION_AUTO );

    if ( message_size < 0 )
    {
        return message_size;
    }

    int fd = concert_device_open ( device );

    if ( fd < 0 )
    {
        return CONCERT_ERROR;
    }

    int bytes_written = concert_device_write ( fd, buffer, message_size );

    if ( bytes_written < 0 )
    {
        return bytes_written;
    }

    int error = concert_device_close ( fd );

    if ( error < 0 )
    {
        return error;
    }

    return 1;
}

int concert_device_open ( const char *device )
{
    int serial_port = open ( device, O_RDWR );

    if ( serial_port < 0 )
    {
        perror ( "Error opening serial port" );
        return CONCERT_ERROR;
    }

    struct termios tty;

    if ( tcgetattr ( serial_port, &tty ) != 0 )
    {
        perror ( "Error getting serial port attributes" );
        close ( serial_port );
        return CONCERT_ERROR;
    }

    cfsetospeed ( &tty, B9600 );
    cfsetispeed ( &tty, B9600 );

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag |= CREAD | CLOCAL;

    cfmakeraw ( &tty );

    if ( tcsetattr ( serial_port, TCSANOW, &tty ) != 0 )
    {
        perror ( "Error setting serial port attributes" );
        close ( serial_port );
        return CONCERT_ERROR;
    }

    fprintf ( stderr, "Serial port %s opened successfully\n", device );
    return 1;
}

int concert_device_close ( int device )
{
    if ( device < 0 )
    {
        return CONCERT_ERROR_INVALID_DEVICE;
    }

    if ( close ( device ) < 0 )
    {
        perror ( "Error closing serial port" );
        return CONCERT_ERROR;
    }

    return 1;
}

int concert_device_ping ( int device )
{
    char buffer[BUFSIZ] = {0};
    buffer[0] = 0x05;
    int bytes_written = concert_device_write ( device, buffer, BUFSIZ );

    if ( bytes_written < 0 )
    {
        return bytes_written;
    }

    int bytes_read = concert_device_read ( device, buffer, BUFSIZ );

    if ( bytes_read < 0 )
    {
        return bytes_read;
    }

    if ( buffer[0] != 0x06 )
    {
        return CONCERT_ERROR;
    }

    return 1;
}

int concert_device_write ( int device, char *buffer, size_t buffer_size )
{
    ssize_t bytes_written = write ( device, buffer, buffer_size );

    if ( bytes_written < 0 )
    {
        perror ( "Error writing to serial port" );
        return CONCERT_ERROR;
    }

    fprintf ( stderr, "Wrote %zd bytes to serial port\n", bytes_written );
    return bytes_written;
}

int concert_device_read ( int device, char *buffer, size_t buffer_size )
{
    if ( buffer == NULL )
    {
        return CONCERT_ERROR;
    }

    ssize_t bytes_read = read ( device, buffer, buffer_size );

    if ( bytes_read < 0 )
    {
        perror ( "Error reading from serial port" );
        return CONCERT_ERROR;
    }

    fprintf ( stderr, "Read %zd bytes from serial port\n", bytes_read );
    return bytes_read;
}
