//======================================================================
//
//  PROJECT:  Test library written in C
//
//
//  FILE:  message-board.c
//
//
//  STARTED:  2017-05-26 FRI
//
//
//  PURPOSE:  To provide a means for one code module to pass data
//   at run time to another code module.  First use Ted's bacnet-stub.c
//   program talking with select routines of Steve Kargs' open source
//   BACnet stack,
//
//
//  NOTES ON IMPLEMENTATION:
//
//   As of 2017-05-30 Ted implementing two routines in this source file
//   named message-board.c.  These routines are a "set value" routine
//   and a corresponding "get value" routine.  These routines take
//   multiple parameters but the one parameter which carries new values
//   to their storing point on a message board, and the like parameter
//   which returns values to calling code from the same message board,
//   that one parameter is of type 'void star'.  The goal with this
//   parameter type choice is for a small number of routines to support
//   various data types including:  basic C data types, arrays of basic
//   C data types, structures and arrays of structures.
//
//   Ted noting here that for now he is implementing separate 'set'
//   and 'get' routines.  This seems a natural dividing point to keep
//   routines a manageable size, but there are with this design choice
//   two places to update in this source file, whenever new message
//   board data are added.
//
//
//   
//
//
//
//
//======================================================================



// - SECTION - pound includes


#include<stdbool.h>


// C test libraries of Ted's:
#include <message-board.h>

#include <diagnostics.h>





//----------------------------------------------------------------------
// - SECTION - global variables and arrays
//----------------------------------------------------------------------

/*
enum message_board_values
{
    MARKER__FIRST_SUPPORTED_VALUE = 0,
    MESSAGE_BOARD_VALUE__NUMBER_RS485_FRAME_BYTES_TO_SEND,
    MESSAGE_BOARD_VALUE__SECOND_SUPPORTED_VALUE,
    MARKER__LAST_SUPPORTED_VALUE

} MESSAGE_BOARD_VALUE;


enum message_board_value_mode_of_use
{
    MESSAGE_BOARD_VALUE__USE_ONCE,
    MESSAGE_BOARD_VALUE__USE_UNTIL_NEXT_SETTING,
    MESSAGE_BOARD_VALUE__ENABLE_USE,
    MESSAGE_BOARD_VALUE__DISABLE_USE,
};
*/


//
//======================================================================
//  **  **  **                                            **  **  **
//    **  **     MESSAGE BOARD SUPPORTED FLAGS AND DATA     **  **
//  **  **  **                                            **  **  **
//======================================================================
//

unsigned int number_of_bytes_to_send_via_RS485_Send_Frame = 0;

bool number_of_bytes_to_send_via_RS485_Send_Frame__value_sent = true;






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
//     *  string to identify calling code,
//
//     *  numeric identifier indicating which message board value to set,
//
//     *  way or mode of using value, e.g. one time versus persistent
//        until next setting,
//
//     *  new value to store on the message board implemented by this
//        source file,
//
//
//  RETURNS:
//     0              . . . on success
//     1 or greater   . . . on error found
//
//
//
//
//
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
    show_diag(rname, lbuf, dflag_verbose);

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "and with mode of use (presently under testing) equal to %u,", value_mode_of_use);
    show_diag(rname, lbuf, dflag_verbose);


    switch ( value_identifier )
    {
        case MESSAGE_BOARD_VALUE__NUMBER_RS485_FRAME_BYTES_TO_SEND:
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "updating 'number of bytes to send' value to %d,", (unsigned int)new_value);
            show_diag(rname, lbuf, dflag_verbose);

            number_of_bytes_to_send_via_RS485_Send_Frame = (unsigned int)new_value;

// 2017-06-01 - Ted implementing 'mode of use' for this message board value . . .
            number_of_bytes_to_send_via_RS485_Send_Frame__value_sent = false;
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "updating 'number of bytes to send' mode to %d,",
              (unsigned int)number_of_bytes_to_send_via_RS485_Send_Frame__value_sent);
            show_diag(rname, lbuf, dflag_verbose);

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

} // end routine message_board_set_value()





unsigned int message_board_get_value(
  const char* caller,
  unsigned int value_identifier,
  void* present_value
)
{
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
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

    DIAG__SET_ROUTINE_NAME("message_board_get_value");


    snprintf(lbuf, SIZE__DIAG_MESSAGE, "starting, called by '%s',", caller);
    show_diag(rname, lbuf, dflag_announce);

    switch ( value_identifier )
    {
        case MESSAGE_BOARD_VALUE__NUMBER_RS485_FRAME_BYTES_TO_SEND:
        {
//
// In this case calling code has sent a pointer of type pointer to
// integer, hence in assignment to parameter named present_value we
// use following C syntax . . .
//
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "message board 'number of bytes to send' holds %d,", number_of_bytes_to_send_via_RS485_Send_Frame);
            show_diag(rname, lbuf, dflag_verbose);

//            present_value = (void*)number_of_bytes_to_send_via_RS485_Send_Frame;
//            present_value = number_of_bytes_to_send_via_RS485_Send_Frame;                 // warning: assignment makes pointer from integer without a cast [-Wint-conversion]
//            present_value = (unsigned int)number_of_bytes_to_send_via_RS485_Send_Frame;   // warning: assignment makes pointer from integer without a cast [-Wint-conversion]
//            present_value = (*unsigned int)number_of_bytes_to_send_via_RS485_Send_Frame;  // error: expected expression before ‘unsigned’
            present_value = (unsigned int*)number_of_bytes_to_send_via_RS485_Send_Frame;    // . . .

//            snprintf(lbuf, SIZE__DIAG_MESSAGE, "now setting calling code's 'present value' parameter to %d,", (int)present_value);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "now setting calling code's 'present value' parameter to %d,", (unsigned int*)present_value);
            show_diag(rname, lbuf, dflag_verbose);


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


    show_diag(rname, "done, returning to caller . . .", dflag_announce);

    return flag_routine_success;

} // end routine message_board_get_value()




//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Following routine returns non-zero value to indicate to caller not
// to use value from message board:
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//

unsigned int from_message_board__number_bytes_to_send(
  const char* caller,
  unsigned int* number_bytes_to_send
)
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  PURPOSE:  to return to calling code a posted number of bytes to
//    send out onto an EIA-485 network.
//
//  TO DO:  implement 'mode of use' for this message board value - TMH
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
{


    int routine_status = 0;

    DIAG__SET_ROUTINE_NAME("from_message_board__number_bytes_to_send");

    if ( strlen(caller) == 0 ) { }   // . . . avoid compiler warning of unused parameter,


// TO DO:  implement 'mode of use' attribute, so that caller can know
//  when this value posted to message board is stale or not meant to be
//  used.  Value one (1) in following IF-test is a place holder for
//  mode of use test . . .

    if ( number_of_bytes_to_send_via_RS485_Send_Frame__value_sent == false  )
    {
        *number_bytes_to_send = number_of_bytes_to_send_via_RS485_Send_Frame;

// Mark this message board variable as sent and spent:
        number_of_bytes_to_send_via_RS485_Send_Frame__value_sent = true;
    }
    else
    {
        routine_status = 1;
    }


// On success, return zero to calling code:
    return routine_status;

} // end routine from_message_board__number_bytes_to_send()





// --- EOF ---
