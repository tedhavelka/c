//======================================================================
//
//  PROJECT:  C Shared Object Type Library
//
//  FILE:  diagnostics.c
//
//  PURPOSE OF THIS FILE:  . . .
//
//======================================================================





#include "diagnostics.h"




void show_diag(const char* caller, const char *message, unsigned int format)
{
//----------------------------------------------------------------------
//  PURPOSE:  to provide a simple and general purpose diagnostics
//   displaying routine with a few simple formatting options.
//----------------------------------------------------------------------


    char lbuf[SIZE__DIAG_MESSAGE];

    DIAG__SET_ROUTINE_NAME("show_diag");



    if ( format == DIAGNOSTICS_OFF )
    {   
        return;
    }   


// STEP - merge and format calling code name, and message, into one string:

    switch ( format )
    {   
        case DIAGNOSTICS__SHOW_MESSAGE_ONLY:
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "%s", message);
            break;
        }

        case DIAGNOSTICS__FORMAT_MESSAGE_WITH_COLON_SEPARATOR:
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "%s:  %s\n", caller, message);
            break;
        }

        case DIAGNOSTICS__FORMAT_MESSAGE_WITH_DASH_SEPARATOR:
        default:
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "%s - %s\n", caller, message);
            break;
        }

    } // end switch on parameter 'format'


    {
        printf("%s", lbuf);
    }

} // end routine show_diag()





void blank_line_out(const char* caller, unsigned int number_of_blank_lines)
{

    int i;

    if ( strlen(caller) == 0 )
    {
//    * Warn that calling code does not identify itself *
    }

    if ( number_of_blank_lines >= 1 )
    {
        for ( i = 0; i < number_of_blank_lines; ++i )
        {
            printf("\n");
        }

    }

} // end routine blank_line_out()




void show_byte_array(
  const char* caller,
  char* pointer_to_bytes,
  unsigned long int count_of_bytes_to_show,
  unsigned int options
)
{
//----------------------------------------------------------------------
//
//  PURPOSE:
//    To display byte arrays in a simple, hexadecimal format, defaulting
//    to sixteen bytes per line.
//
//
//  EXPECTS:
//    *  string to identify calling code,
//    *  pointer to byte or character array,
//    *  count of bytes to show (1)
//    *  formatting option
//
//   (1) up to calling code to limit sensibly number of bytes to show.
//
//
//  RETURNS:
//    *  nothing
//
//
//  TO DO:  add support for additional formats
//
//
//----------------------------------------------------------------------


    unsigned int on_formatting_and_other_options;

    char datum_formatted[SIZE__TOKEN];
    memset(datum_formatted, 0, SIZE__TOKEN);
    char space_between_data[SIZE__TOKEN];
    memset(space_between_data, 0, SIZE__TOKEN);
    char lbuf_for_bytes[SIZE__DIAG_MESSAGE];

    int formatted_line_space_remaining = 0;


// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];

    unsigned int dflag_announce   = DIAGNOSTICS_ON;
    unsigned int dflag_verbose    = DIAGNOSTICS_ON;
    unsigned int dflag_warning    = DIAGNOSTICS_ON;

    unsigned int dflag_development = DIAGNOSTICS_ON;
    unsigned int dflag_modulus    = DIAGNOSTICS_OFF;

    unsigned int dflag_byte_array = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("show_byte_array");


    show_diag(rname, "starting,", dflag_announce);

    if ( strlen(caller) == 0 )
    {
//    * Warn that calling code does not identify itself *
    }

    if ( options == 0 )
    {
        show_diag(rname, "- DEV - received default option of zero,", dflag_development);
        show_diag(rname, "- DEV - Ted noting here that formatting and other options not yet,", dflag_development);
        show_diag(rname, "- DEV - supported in this C test library routine.", dflag_development);
    }
    else
    {
        on_formatting_and_other_options = options;
    }

    if ( count_of_bytes_to_show < 1 )
    {
        show_diag(rname, "calling code has sent byte limit of zero,", dflag_verbose);
        show_diag(rname, "therefore no bytes to show.", dflag_verbose);
        show_diag(rname, "returning to caller . . .", dflag_announce);
        return;
    }


//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  2017-05-16 - Switch statement to support different formatting and
//     other potential future options, for showing byte arrays:
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//

    {
        switch ( on_formatting_and_other_options )
        {

            case BYTE_ARRAY__DISPLAY_FORMAT__16_PER_LINE__GROUPS_OF_FOUR:
            {
                show_diag(rname, "at case to show 16 bytes per line, in groups of four,", dflag_verbose);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "calling routine '%s' asks to see %lu bytes,", caller, count_of_bytes_to_show);
                show_diag(rname, lbuf, dflag_verbose);


    int datum_width = 2;

    int bytes_per_line = 16;
    int group_every_n_bytes = 4;

    int lines_remaining = 0;
    int lines_shown = 0;
    int bytes_remaining = 0;
    int bytes_shown = 0;

//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  NOTE:  some of the variables just above this comment could be
//   set in format cases of this case statement, and the WHILE-loop
//   construct just below this comment could be moved just past this
//   CASE statement block, and thereby written only once . . .   - TMH
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//

                lines_remaining = ( count_of_bytes_to_show / bytes_per_line );

                while ( lines_remaining > -1 )
                {
                    bytes_shown = 0;
                    bytes_remaining = ( count_of_bytes_to_show - bytes_shown );
                    memset(lbuf_for_bytes, 0, SIZE__DIAG_MESSAGE);

                    while ( ( bytes_remaining > 0 ) && ( bytes_shown < bytes_per_line ) )
                    {
                        // - show sixteen or as many remaining bytes on a line of output -

                        // step - format the present byte:
//                        snprintf(datum_formatted, SIZE__TOKEN, "%02X", pointer_to_bytes[bytes_shown]);
//                        snprintf(datum_formatted, SIZE__TOKEN, "%.*X", datum_width, (char)pointer_to_bytes[bytes_shown]);  // <- getting '00   55 FFFFFFAA FFFFFFFF 00'
//                        snprintf(datum_formatted, SIZE__TOKEN, "%.*X", datum_width, (pointer_to_bytes[bytes_shown] & 0xff));
                        snprintf(datum_formatted, SIZE__TOKEN, "%.*X", datum_width, (pointer_to_bytes[(lines_shown * bytes_per_line + bytes_shown)] & 0xff));

                        snprintf(lbuf, SIZE__DIAG_MESSAGE, "bytes shown %d modulus %d gives %d,",
                          bytes_shown, group_every_n_bytes, ( bytes_shown % group_every_n_bytes ));
                        show_diag(rname, lbuf, dflag_modulus);

                        // step - format space between bytes and byte groups:
//                        if (( bytes_shown % group_every_n_bytes ) == 0 )
                        if (( bytes_shown % group_every_n_bytes ) == (group_every_n_bytes - 1) )
                        {
                            snprintf(space_between_data, SIZE__TOKEN, "%s", "   ");
                        }
                        else
                        {
                            snprintf(space_between_data, SIZE__TOKEN, "%s", " ");
                        }

                        // step - format space between bytes and byte groups:
                        formatted_line_space_remaining = (( SIZE__TOKEN - strlen(lbuf_for_bytes)) - ONE_BYTE_FOR_NULL_TERMINATION);
                        strncat(lbuf_for_bytes, datum_formatted, formatted_line_space_remaining);

                        formatted_line_space_remaining = (( SIZE__TOKEN - strlen(lbuf_for_bytes)) - ONE_BYTE_FOR_NULL_TERMINATION);
                        strncat(lbuf_for_bytes, space_between_data, formatted_line_space_remaining);

                        ++bytes_shown;
//                        bytes_remaining = ( count_of_bytes_to_show - bytes_shown );
                        bytes_remaining = ( count_of_bytes_to_show - (lines_shown * bytes_per_line + bytes_shown));
                    }

                    show_diag(rname, lbuf_for_bytes, dflag_byte_array);

                    ++lines_shown;
                    --lines_remaining;

                } // end WHILE-loop to formwat bytes of array for showing

                break;
            }


            default:
            {
                show_diag(rname, "WARNING - reached 'default' line in case statement to select among byte array formats",
                  dflag_warning);
                break;
            }

        }

    } // end local scope of 'show_byte_array' format switch statement

    show_diag(rname, "done.", dflag_announce);

} // end routine show_byte_array(), C test library





// --- EOF ---
