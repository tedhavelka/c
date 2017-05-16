#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H





#include <stdio.h>    // provides snprintf(),

#include <string.h>   // provides strncpy(),




//--------------------------------------------------------------------
//  SECTION:  values to enable and disable diagnostics:
//--------------------------------------------------------------------

// #define DIAGNOSTICS_ON (1)
#define DIAGNOSTICS_ON (DEFAULT_DIAG_MESSAGE_FORMAT)

#define DIAGNOSTICS_OFF (0)

#define TO_QT4_APP_OUTPUT DIAGNOSTICS_ON




//--------------------------------------------------------------------
//  SECTION:  general status and results returned by library routines:
//--------------------------------------------------------------------

#ifndef SUCCESS
#define SUCCESS (0)
#endif

#ifndef FAIL
#define FAIL (1)
#endif



#define LENGTH__DIRNAME (4096)



#define SIZE__ROUTINE_NAME (256)

#define SIZE__TOKEN (256)

#define SIZE__DIAG_MESSAGE (1024)



// 2017-05-16 - simple pound defines for code readability:

#define ONE_BYTE_FOR_NULL_TERMINATION (1)



//--------------------------------------------------------------------
//  - SECTION - diagnostics formats
//
//  2017-03-16 - Some explanation of Ted's local C and C++
//    diagnostics formats . . .
//
//
//--------------------------------------------------------------------

enum diagnostics_message_formats
{
    DIAGNOSTICS__SHOW_MESSAGE_ONLY = 1,
    DIAGNOSTICS__FORMAT_MESSAGE_WITH_COLON_SEPARATOR,
    DIAGNOSTICS__FORMAT_MESSAGE_WITH_DASH_SEPARATOR,
    DIAGNOSTICS__PREPEND_CHAR_POUND_SIGN,
    DIAGNOSTICS__PREPEND_CHAR_SEMICOLON
};


#define DEFAULT_DIAG_MESSAGE_FORMAT DIAGNOSTICS__FORMAT_MESSAGE_WITH_COLON_SEPARATOR



// 2017-05-16 - enumeration for routine to display byte and character arrays:

enum byte_array_display_options
{
    BYTE_ARRAY__DISPLAY_FORMAT__8_PER_LINE,
    BYTE_ARRAY__DISPLAY_FORMAT__8_PER_LINE__GROUPS_OF_FOUR,
    BYTE_ARRAY__DISPLAY_FORMAT__16_PER_LINE,
    BYTE_ARRAY__DISPLAY_FORMAT__16_PER_LINE__GROUPS_OF_FOUR,
    BYTE_ARRAY__DISPLAY_FORMAT__END_OF_FORMATS_MARKER
};





//--------------------------------------------------------------------
//  - SECTION - macros
//--------------------------------------------------------------------

// 2007-10-19  does it matter whether pound define statements like
//  contain other pound define identifiers appear before the
//  definitions of their nested pound defines?  - TMH

#define DIAG__SET_ROUTINE_NAME(x) \
    char rname[SIZE__ROUTINE_NAME]; \
    strncpy(rname, x, SIZE__ROUTINE_NAME);



//--------------------------------------------------------------------
//  SECTION:  function prototypes
//--------------------------------------------------------------------

void show_diag(const char* caller, const char *message, unsigned int format);

void blank_line_out(const char* caller, unsigned int number_of_blank_lines);

void show_byte_array(
  const char* caller,
  char* pointer_to_bytes,
  unsigned long int count_of_bytes_to_show,
  unsigned int options
);





#endif // DIAGNOSTICS_H
