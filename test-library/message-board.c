//======================================================================
//
//  PROJECT:  Test library writting in C
//
//
//  FILE:  message-board.c
//
//
//  STARTED:  2017-02-10 FRI
//
//
//  PURPOSE:  To provide a means for one code module to pass data
//   at run time to another code module.  First use Ted's bacnet-stub.c
//   program talking with select routines of Steve Kargs' open source
//   BACnet stack,
//
//
//  NOTES ON IMPLEMENTATION
//   . . .
//
//
//======================================================================



// - SECTION - pound includes

#include <message-board.h>

#include <diagnostics.h>





//----------------------------------------------------------------------
// - SECTION - global variables and arrays
//----------------------------------------------------------------------

enum message_board_values
{
    FIRST_SUPPORTED_VALUE_MARKER = 0,
    MESSAGE_BOARD_VALUE__NUMBER_RS485_FRAME_BYTES_TO_SEND,
    MESSAGE_BOARD_VALUE__SECOND_SUPPORTED_VALUE,
    LAST_SUPPORTED_VALUE_MARKER

} MESSAGE_BOARD_VALUE;


unsigned int number_of_bytes_to_send = 0;







//----------------------------------------------------------------------
// - SECTION - global variables and arrays
//----------------------------------------------------------------------

unsigned int message_board_set_value(
  const char* caller,
  unsigned int value_identifier,
  unsigned int value_mode_of_use,
  void* new_value)
{
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//  PURPOSE:
//
//
//  EXPECTS:
//
//
//  RETURNS:
//     0              . . . on success
//     1 or greater   . . . on error found
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    unsigned int flag_routine_success = 1;

// diagnostics:

    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;
    unsigned int dflag_warning = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("message_board_set_value");



    show_diag(rname, "starting,", dflag_announce);

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "called to update value identified by index %u,", value_identifier);
    show_diag(rname, lbuf, dflag_warning);

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "and with mode of use (not yet implemented) equal to %u,", value_mode_of_use);
    show_diag(rname, lbuf, dflag_warning);


    switch ( value_identifier )
    {
        case MESSAGE_BOARD_VALUE__NUMBER_RS485_FRAME_BYTES_TO_SEND:
        {
            number_of_bytes_to_send = (unsigned int)new_value;
// 2017-05-26 - mode of use not yet implemented - TMH
            flag_routine_success = 0;
            break;
        }

        default:     
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "- WARNING - encountered unsupported value identifier %u,", value_identifier);
            show_diag(rname, lbuf, dflag_warning);
            show_diag(rname, "- WARNING - + calling code asking this library of Ted's C test code", dflag_warning);
            show_diag(rname, "- WARNING - + to set a value which this library doesn't yet support.", dflag_warning);
            show_diag(rname, "- WARNING - + Not changing any message board values.", dflag_warning);
            break;
    }


    return flag_routine_success;

}










// --- EOF ---
