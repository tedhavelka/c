#ifndef MESSAGE_BOARD_H
#define MESSAGE_BOARD_H




//----------------------------------------------------------------------
// - SECTION - enumerations
//----------------------------------------------------------------------

// These enumerations in message-board.h header file, so that code
// external to this C test library may send integer options which are 
// descriptively labeled . . .

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




//----------------------------------------------------------------------
// - SECTION - function prototypes
//----------------------------------------------------------------------

unsigned int message_board_set_value(
  const char* caller,
  unsigned int value_identifier,
  unsigned int value_mode_of_use,
  void* new_value
);


unsigned int message_board_get_value(
  const char* caller,
  unsigned int value_identifier,
  void* present_value
);






#endif // MESSAGE_BOARD_H
