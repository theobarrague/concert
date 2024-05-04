#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "concert.h"

int main ( int argc, char **argv )
{
    char *devices[10];
    int num_devices = concert_list_serial_devices ( devices, 10 );

    if ( num_devices < 0 )
    {
        printf ( "Error listing serial devices.\n" );
        return 1;
    }

    if ( num_devices == 0 )
    {
        printf ( "No serial devices available.\n" );
        return 1;
    }

    printf ( "Available serial devices:\n" );

    for ( int i = 0; i < num_devices; i++ )
    {
        printf ( "%d. %s\n", i + 1, devices[i] );
    }

    int selected_device;
    char auto_mode = 'N';
    printf ( "\nEnter 'A' for auto mode or select a device (1-%d): ",
             num_devices );
    scanf ( "%c", &auto_mode );

    if ( auto_mode == 'A' )
    {
        selected_device = -1;

        for ( int i = 0; i < num_devices; i++ )
        {
            int ping_result = concert_device_ping ( i );

            if ( ping_result == 0 )
            {
                selected_device = i;
                break;
            }
        }
    }
    else
    {
        scanf ( "%d", &selected_device );
        selected_device--;
    }

    if ( selected_device < 0 || selected_device >= num_devices )
    {
        printf ( "Invalid selection.\n" );
        return 1;
    }

    unsigned int amount_eur;
    printf ( "Enter amount in euros: " );
    scanf ( "%u", &amount_eur );

    unsigned int amount_cents = amount_eur * 100;

    int result = concert_simple_request ( devices[selected_device],
                                          amount_cents, CONCERT_CURRENCY_EUR );

    if ( result != 0 )
    {
        printf ( "Error sending payment request.\n" );
        return 1;
    }

    printf ( "Payment request sent successfully.\n" );

    for ( int i = 0; i < num_devices; i++ )
    {
        free ( devices[i] );
    }

    return 0;
}

