//----------------------------------------------------------------------
//
//  2017-01-17
//
//
//  DESCRIPTION:
//
//    Test code to calculate BACnet header CRC, algorithms copied from
//    Dave Bruno's BACnet implementation and then ported by Tom Almy
//    to CWLP firmware cerca 2009.
//
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  TO-DO:
//
//  [ ]  2017-04-24 - To be more useful, Ted adding ability to pass this 
//    program a text filename, which it may open, read and compute when
//    possible the checksum of the hexadecimal encoded bytes in the
//    given text file . . .
//
//
//
//  COMPILE STEPS:
//
//    $ gcc -Wall main.c
//
//    $ gcc -Wall -I/usr/local/include/libtestlib-0p1 main.c
//
//    $ gcc main.c -g3 -Wall -I/usr/local/include/libtestlib-0p1 -L/usr/local/lib/libtestlib-0p1 -ltestlib-0p1 -o a.out
//
//
//
//  REFERENCES:
//
//    *  http://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c
//
//
//
//  NOTES:  In calculating the BACnet header CRC, be sure to skip the
//    preamble bytes 0x55 and 0xFF.  - TMH
//
//
//
//----------------------------------------------------------------------




#include <stdlib.h>
#include <stdio.h>


// 2017-04-24 - Ted adding local diagnostics:

#include <diagnostics.h>





//----------------------------------------------------------------------
//  - SECTION - pound defines
//----------------------------------------------------------------------

#define NUMBER_OF_FILE_LINES_STORABLE (1024)
#define LENGTH_OF_TEXT_SUPPORTED (1024)
#define NUMBER_OF_PROTOCOL_DATA_UNIT_BYTES_STORABLE (1024)

#define DEFAULT_OPTION_OF_ZERO (0)


// 2017-04-27 - added by Ted . . .
#define SIZE_OF_LINE_SUPPORTED (1024)

#define BASE_16_TO_CONVERT_FROM_HEXADECIMAL (16)

// 2017-05-01 - added by Ted . . .
#define SIZE__FILENAME (256)



//----------------------------------------------------------------------
//  - SECTION - global variables, arrays, structures
//----------------------------------------------------------------------

// 2015-07-02 - Added by Ted, testing alternate possibly more
//  centralized way to control diagnostics verbose levels on per
//  routine basis:

unsigned int global_bit_wise_diagnostics_flags_0_through_31 = 0;

// 2017-05-02 - a standard C integer is four bytes, or thirty two bits,
//  so we'll start out using a single integer to support thirty two
//  (32) bit-wise flags for routines of this program . . .

enum local_routine_identifiers_0_through_31   // <-- DFLAGS --
{
    ROUTINE__MAIN = 1,
    ROUTINE__PARSE_COMMAND_LINE_ARGUMENTS = 2,
    ROUTINE__PARSE_PDU_DATA_FROM_COMMAND_LINE = 4,
    ROUTINE__BREAK_LINES_INTO_TOKENS = 8,
    ROUTINE__FREE_MEMORY_HOLDING_TEXT_FILE_DATA = 16,
    ROUTINE__UCBN_CALC_HEADERCRC = 32,
};




// Note:  Dave Bruno's BACnet implementation makes use of a fast table
//  look up method to figure 16-bit CRCs, and also an XOR based method
//  to compute the same CRCs.  Ted testing both methods as a sanity
//  check and to see them work in equivalent fashion:
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

enum crc_calculation_methods
{
    TABLE_LOOK_UP,
    CALCULATE_VIA_XOR_OPERATIONS
};




// Note:  following global variables support this program's ability
//  to read data from a single text file.  Not scalable but good enough
//  to offer a reasonable way to get larger BACnet data packets into
//  the program rather than hard-coding or typing them in as command
//  line arguments . . .                                       - TMH
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

char global_filename__pdu_data_file[SIZE__FILENAME];

char* text_file_lines[NUMBER_OF_FILE_LINES_STORABLE];

unsigned char pdu_bytes[NUMBER_OF_PROTOCOL_DATA_UNIT_BYTES_STORABLE];




// This variable taken from Dave Bruno's code:

// volatile uint8_t               ucBN_HeaderCRC;
volatile unsigned char ucBN_HeaderCRC;



// 2017-01-16 MON - this table of CRC calculating values taken from co2_protocol
//  source file named bacnet_const.c:

// static const PROGMEM uint16_t xpuiBACnet_Header_CRC_Table[] = { 
static const short unsigned int xpuiBACnet_Header_CRC_Table[] = {
     0x0000, 0x00FE, 0x01FC, 0x0102, 0x03F8, 0x0306, 0x0204, 0x02FA, 
     0x07F0, 0x070E, 0x060C, 0x06F2, 0x0408, 0x04F6, 0x05F4, 0x050A, 
     0x0FE0, 0x0F1E, 0x0E1C, 0x0EE2, 0x0C18, 0x0CE6, 0x0DE4, 0x0D1A, 
     0x0810, 0x08EE, 0x09EC, 0x0912, 0x0BE8, 0x0B16, 0x0A14, 0x0AEA, 
     0x1FC0, 0x1F3E, 0x1E3C, 0x1EC2, 0x1C38, 0x1CC6, 0x1DC4, 0x1D3A, 
     0x1830, 0x18CE, 0x19CC, 0x1932, 0x1BC8, 0x1B36, 0x1A34, 0x1ACA, 
     0x1020, 0x10DE, 0x11DC, 0x1122, 0x13D8, 0x1326, 0x1224, 0x12DA, 
     0x17D0, 0x172E, 0x162C, 0x16D2, 0x1428, 0x14D6, 0x15D4, 0x152A, 
     0x3F80, 0x3F7E, 0x3E7C, 0x3E82, 0x3C78, 0x3C86, 0x3D84, 0x3D7A, 
     0x3870, 0x388E, 0x398C, 0x3972, 0x3B88, 0x3B76, 0x3A74, 0x3A8A, 
     0x3060, 0x309E, 0x319C, 0x3162, 0x3398, 0x3366, 0x3264, 0x329A, 
     0x3790, 0x376E, 0x366C, 0x3692, 0x3468, 0x3496, 0x3594, 0x356A, 
     0x2040, 0x20BE, 0x21BC, 0x2142, 0x23B8, 0x2346, 0x2244, 0x22BA, 
     0x27B0, 0x274E, 0x264C, 0x26B2, 0x2448, 0x24B6, 0x25B4, 0x254A, 
     0x2FA0, 0x2F5E, 0x2E5C, 0x2EA2, 0x2C58, 0x2CA6, 0x2DA4, 0x2D5A, 
     0x2850, 0x28AE, 0x29AC, 0x2952, 0x2BA8, 0x2B56, 0x2A54, 0x2AAA, 
     0x7F00, 0x7FFE, 0x7EFC, 0x7E02, 0x7CF8, 0x7C06, 0x7D04, 0x7DFA, 
     0x78F0, 0x780E, 0x790C, 0x79F2, 0x7B08, 0x7BF6, 0x7AF4, 0x7A0A, 
     0x70E0, 0x701E, 0x711C, 0x71E2, 0x7318, 0x73E6, 0x72E4, 0x721A, 
     0x7710, 0x77EE, 0x76EC, 0x7612, 0x74E8, 0x7416, 0x7514, 0x75EA, 
     0x60C0, 0x603E, 0x613C, 0x61C2, 0x6338, 0x63C6, 0x62C4, 0x623A, 
     0x6730, 0x67CE, 0x66CC, 0x6632, 0x64C8, 0x6436, 0x6534, 0x65CA, 
     0x6F20, 0x6FDE, 0x6EDC, 0x6E22, 0x6CD8, 0x6C26, 0x6D24, 0x6DDA, 
     0x68D0, 0x682E, 0x692C, 0x69D2, 0x6B28, 0x6BD6, 0x6AD4, 0x6A2A, 
     0x4080, 0x407E, 0x417C, 0x4182, 0x4378, 0x4386, 0x4284, 0x427A, 
     0x4770, 0x478E, 0x468C, 0x4672, 0x4488, 0x4476, 0x4574, 0x458A, 
     0x4F60, 0x4F9E, 0x4E9C, 0x4E62, 0x4C98, 0x4C66, 0x4D64, 0x4D9A, 
     0x4890, 0x486E, 0x496C, 0x4992, 0x4B68, 0x4B96, 0x4A94, 0x4A6A, 
     0x5F40, 0x5FBE, 0x5EBC, 0x5E42, 0x5CB8, 0x5C46, 0x5D44, 0x5DBA, 
     0x58B0, 0x584E, 0x594C, 0x59B2, 0x5B48, 0x5BB6, 0x5AB4, 0x5A4A, 
     0x50A0, 0x505E, 0x515C, 0x51A2, 0x5358, 0x53A6, 0x52A4, 0x525A, 
     0x5750, 0x57AE, 0x56AC, 0x5652, 0x54A8, 0x5456, 0x5554, 0x55AA, 
};




//----------------------------------------------------------------------
//  - section - routine prototypes
//----------------------------------------------------------------------

//
// 2017-05-02 - Ted noting that with static routine definitions,
//  a routine prototype doesn't make syntactical or rule-wise sense
//  at compile time.  The compiler interprets the prototype as a full
//  routine definition, and then complains of a twice-defined routine
//  when it encounters the definition after the prototype.  Ted yet
//  learning simple things about C and compile time issues . . . TMH
//

// static unsigned char ucBN_Calc_HeaderCRC(const unsigned int, const unsigned int);

void show_binary_number(const char* caller, int number_to_show);

int diagnostics_flag_set(const char* caller, int local_routine_identifier);
int diagnostics_flag_reset(const char* caller, int local_routine_identifier);
int diagnostics_flag_value(const char* caller, int local_routine_identifier);




//----------------------------------------------------------------------
//  - SECTION - routine definitions
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//  - SECTION - static routines
//----------------------------------------------------------------------

#define FAST_BACNET_HEADER_CRC

// //**************************************************************
// static uint8_t ucBN_Calc_HeaderCRC(uint8_t ucData) {
// //**************************************************************

// static unsigned char ucBN_Calc_HeaderCRC(unsigned char ucData)
static unsigned char ucBN_Calc_HeaderCRC(unsigned char ucData, unsigned char method_of_calculation)
{
//----------------------------------------------------------------------
//
//  2017-05-02 - This routine from Dave Bruno's BACnet part of firmware
//   from cerca 2005 work.  Just calculates a 16-bit CRC and in this
//   sense is not BACnet specific.
//
//   Ted noting as well that in context of desktop C program running
//   in a multi-user operating system environment -- in this case
//   Ubuntu Linux -- this routine may not need be static.  Ted to test
//   whether there's any detectable difference in this program's
//   output, when static qualifier is present and when absent . . .
//
//                                                             - TMH
//
//----------------------------------------------------------------------

//     uint16_t uiCRC;
     short unsigned int uiCRC;
     short unsigned int index;

// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;
//    unsigned int dflag_warning = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("ucBN_Calc_HeaderCRC");



    dflag_announce = diagnostics_flag_value(rname, ROUTINE__UCBN_CALC_HEADERCRC);
    dflag_verbose = diagnostics_flag_value(rname, ROUTINE__UCBN_CALC_HEADERCRC);

    show_diag(rname, "starting,", dflag_announce);

//    printf("%s():  received byte of data holding %02X, ucBN_HeaderCRC holds %02X,\n", "ucBN_Calc_HeaderCRC", ucData, ucBN_HeaderCRC);
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "received byte of data holding %02X, ucBN_HeaderCRC holds %02X,",
      ucData, ucBN_HeaderCRC);
    show_diag(rname, lbuf, dflag_verbose);


    uiCRC =  ucData ^ ucBN_HeaderCRC;

// #ifdef FAST_BACNET_HEADER_CRC
//     uiCRC = uiCRC  ^ pgm_read_word(&xpuiBACnet_Header_CRC_Table[uiCRC]);

    if ( method_of_calculation == TABLE_LOOK_UP )
    {
        show_diag(rname, "using CRC table,", dflag_verbose);
        index = uiCRC;
        uiCRC = uiCRC ^ xpuiBACnet_Header_CRC_Table[index];
    }

// #else // FAST_BACNET_HEADER_CRC
    else
    {
        show_diag(rname, "calculating CRC via exclusive-OR operations . . .", dflag_verbose);
        uiCRC = uiCRC ^ (uiCRC<<1) ^ (uiCRC<<2) ^ (uiCRC<<3) ^ (uiCRC<<4) ^ (uiCRC<<5) ^ (uiCRC<<6) ^ (uiCRC<<7);
    }

// #endif // FAST_BACNET_HEADER_CRC


//     printf("%s():  returning updated ucBN_HeaderCRC equal to %02X . . .\n", "ucBN_Calc_HeaderCRC", (uiCRC&0xFE) ^ ((uiCRC>>8)&1));
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "done, returning updated ucBN_HeaderCRC equal to %02X . . .\n",
      (uiCRC&0xFE) ^ ((uiCRC>>8)&1));
    show_diag(rname, lbuf, dflag_verbose);

    return (uiCRC&0xFE) ^ ((uiCRC>>8)&1);

} // end routine ucBN_Calc_HeaderCRC() 








//----------------------------------------------------------------------
//  - SECTION - code from earlier development steps
//----------------------------------------------------------------------

int old_tests()
{
//----------------------------------------------------------------------
//  DESCRIPTION:  code from earlier development steps, prior to
//   program ability to read BACnet PDU byte data from file or
//   command line arguments:
//----------------------------------------------------------------------

// should have CRC equal to 0xDA, per document "bacnet-encoding.rtf" or
// "Encoding.doc" page 4 of 42:

    unsigned char bytes_of_header[] = {0x55, 0xFF, 0x02, 0x00, 0x09};


    DIAG__SET_ROUTINE_NAME("old_tests");



    printf("- TEST 1 -\n");
    printf("initializing header CRC to 0xFF,\n");
    ucBN_HeaderCRC = 0xFF;

    printf("calling routine a few times, to calculate sample BACnet packet header CRC . . .\n");
//    sample_crc = ucBN_Calc_HeaderCRC(1);
//    sample_crc = ucBN_Calc_HeaderCRC(bytes_of_header[0]);
//    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[0]);

//    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[0], TABLE_LOOK_UP);
//    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[1], TABLE_LOOK_UP);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[2], TABLE_LOOK_UP);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[3], TABLE_LOOK_UP);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[4], TABLE_LOOK_UP);

    printf("all bytes of header sent, final CRC value is %02X\n", ucBN_HeaderCRC);

    printf("masking to least eight bits:\n");
//    printf("masking all but lowest eight bits gives %02X,\n\n", ( ~ucBN_HeaderCRC & 0xFF ));
    printf("header CRC value calculated to be 0x%02X equal to decimal %u,\n\n", ( ucBN_HeaderCRC & 0xFF ), ( ucBN_HeaderCRC & 0xFF ));

    printf("header CRC value after taking binary one's compliment and bit-wise ANDing is 0x%02X,\n\n", ( ~ucBN_HeaderCRC & 0xFF ));



    printf("- TEST 2 -\n");
    printf("initializing header CRC to 0xFF,\n");
    ucBN_HeaderCRC = 0xFF;
    printf("calling routine a few times, to calculate sample BACnet packet header CRC . . .\n");
//    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[0], CALCULATE_VIA_XOR_OPERATIONS);
//    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[1], CALCULATE_VIA_XOR_OPERATIONS);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[2], CALCULATE_VIA_XOR_OPERATIONS);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[3], CALCULATE_VIA_XOR_OPERATIONS);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[4], CALCULATE_VIA_XOR_OPERATIONS);
    printf("masking to least eight bits:\n");
    printf("header CRC value calculated to be 0x%02X equal to decimal %u,\n\n", ( ucBN_HeaderCRC & 0xFF ), ( ucBN_HeaderCRC & 0xFF ));
    printf("header CRC value after bit-wise AND is 0x%02X,\n\n", ( ~ucBN_HeaderCRC & 0xFF ));


/*
    printf("- TEST 3 -\n");
    printf("initializing header CRC to 0xFF,\n");
    ucBN_HeaderCRC = 0xFF;
    printf("calling routine a few times, to calculate sample BACnet packet header CRC . . .\n");

    bytes_of_header[0] = 0x55;   // should have CRC equal to 0xBB, per document "bacnet-encoding.rtf" or "Encoding.doc" page 4 of 42,
    bytes_of_header[1] = 0xFF;
    bytes_of_header[2] = 0x06;
    bytes_of_header[3] = 0x00;
    bytes_of_header[4] = 0x00;

    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[0], TABLE_LOOK_UP);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[1], TABLE_LOOK_UP);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[2], TABLE_LOOK_UP);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[3], TABLE_LOOK_UP);
    ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[4], TABLE_LOOK_UP);
    printf("masking to least eight bits:\n");
    printf("header CRC value calculated to be 0x%02X equal to decimal %u,\n\n", ( ucBN_HeaderCRC & 0xFF ), ( ucBN_HeaderCRC & 0xFF ));
    printf("header CRC value after bit-wise AND is 0x%02X,\n\n", ( ~ucBN_HeaderCRC & 0xFF ));
*/


    return 0;

} // end of routine old_tests()





//----------------------------------------------------------------------
//  - SECTION - local diagnostics
//----------------------------------------------------------------------

void show_binary_number(const char* caller, int number_to_show)
{

    char string__number_in_binary_formatted[SIZE__TOKEN];
    memset(string__number_in_binary_formatted, 0, SIZE__TOKEN);

// counter used to iterate over bits of number to show in binary format:
    int n;

// index used to build formatted string with additional white space between groups of bits:
    int string_index;

// copy of number to show, preserves number passed from caller:
    int number;


// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_verbose = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("show_binary_number");



    snprintf(lbuf, SIZE__DIAG_MESSAGE, "ready to show number which is %d bytes long, in binary format,",
      sizeof(number_to_show));
    show_diag(rname, lbuf, dflag_verbose);
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "caller passes us value '%d',", number_to_show);
    show_diag(rname, lbuf, dflag_verbose);

    number = number_to_show;

// Variable n here to count down from the number of bits in the number to format binarily, to one:
    n = ( sizeof(number_to_show) * 8 );

// We build the formatted number string from left to right, to ease arbitrary spacing of groups of bits:
    string_index = 0;

    while ( n )
    {
//        snprintf(lbuf, SIZE__DIAG_MESSAGE, "bit-wise ANDing %d and %d,", number, (2 ^ (n - 1)));   . . . Ahh we mistakenly using XOR operator, not 'a to power of b' - TMH
//        snprintf(lbuf, SIZE__DIAG_MESSAGE, "bit-wise ANDing %u and %u,", number, (1  << ((sizeof(number_to_show) * 8) - 1)));
//        show_diag(rname, lbuf, dflag_verbose);

        if ( number & (1  << ((sizeof(number_to_show) * 8) - 1)) )
            { string__number_in_binary_formatted[string_index] = '1'; }
        else
            { string__number_in_binary_formatted[string_index] = '0'; }

// Working from MSB to LSB one bit at time:
        number = ( number << 1 );

        --n;

        ++string_index;


//        if ( n % 8 )    . . . evaluates true for bits 1 through 31,
        if ( (n % 8) == 0 )
        {
//            snprintf(lbuf, SIZE__DIAG_MESSAGE, "checked %d bits, copy of number to show now = %d,",
//              (32 - n), number);
//            show_diag(rname, lbuf, dflag_verbose);

// String index has already been advance, so add white space then increment string index again:
//            string__number_in_binary_formatted[string_index] = ' ';
            string__number_in_binary_formatted[string_index] = 32;
            ++string_index;
        }

    }

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "number in binary:  %s", string__number_in_binary_formatted);
    show_diag(rname, lbuf, dflag_verbose);

} // end routine show_binary_number()





int diagnostics_flag_set(const char* caller, int local_routine_identifier)
{
    global_bit_wise_diagnostics_flags_0_through_31 = (global_bit_wise_diagnostics_flags_0_through_31 | local_routine_identifier);

    return 0;
}


int diagnostics_flag_reset(const char* caller, int local_routine_identifier)
{
    global_bit_wise_diagnostics_flags_0_through_31 = (global_bit_wise_diagnostics_flags_0_through_31 & (!(local_routine_identifier)));

    return 0;
}


int diagnostics_flag_value(const char* caller, int local_routine_identifier)
{


    return (global_bit_wise_diagnostics_flags_0_through_31 & local_routine_identifier);
}


void diagnostics_flag_show_value(const char* caller)
{
    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_verbose = DIAGNOSTICS_ON;

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "global diagnostics flag holds %d", global_bit_wise_diagnostics_flags_0_through_31);
    show_diag("diagnostics_flag_show_value", lbuf, dflag_verbose);
}






//----------------------------------------------------------------------
//
//  2017-04-24 - Following code taken and adapted from example at 
//
//   *  http://stackoverflow.com/questions/3501338/c-read-file-line-by-line:
//
//----------------------------------------------------------------------

// int read_text_file(void)
int read_text_file(const char* caller, const char* filename)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    unsigned int line_index = 0;

// diagnostics:

//    char lbuf[SIZE__DIAG_MESSAGE];

    unsigned int dflag_warning = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("read_text_file");



//    fp = fopen("/etc/motd", "r");
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
//        exit(EXIT_FAILURE);
        return EXIT_FAILURE;
    }



// top-of-loop preparations, loop to read file line by line:

    line_index = 0;


    while ((read = getline(&line, &len, fp)) != -1)
    {
        printf("Retrieved line of length %zu :\n", read);
        printf("%s", line);

// - STEP - store line from text file
        if ( line_index < NUMBER_OF_FILE_LINES_STORABLE )
        {
            text_file_lines[line_index] = calloc(LENGTH_OF_TEXT_SUPPORTED, sizeof(char));

            if ( text_file_lines[line_index] != NULL )
            {
                snprintf(text_file_lines[line_index], (LENGTH_OF_TEXT_SUPPORTED - 1), "%s", line);

                ++line_index;
            }
            else
            {
                show_diag(rname, "WARNING - not able to allocate memory to store line of text from file!", dflag_warning);
            }
        }


// - STEP - parse line from text file

    }


    fclose(fp);

    if (line)
    {
        free(line);
    }

//    exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;

} // end routine read_text_file()




int show_lines_from_file(const char* caller, const int option)
{

//    unsigned int line_not_zero_length = 0;
    unsigned int index_to_line = 0;

// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;
//    unsigned int dflag_warning = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("show_lines_from_file");


    show_diag(rname, "starting,", dflag_announce);
    show_diag(rname, "--  Summary of lines read from text file:  --", dflag_verbose);


//    line_not_zero_length = strlen(text_file_lines[index_to_line]);

//    while (( line_not_zero_length ) && ( index_to_line < NUMBER_OF_FILE_LINES_STORABLE ))
    while (( text_file_lines[index_to_line] != NULL ) && ( index_to_line < NUMBER_OF_FILE_LINES_STORABLE ))
    {
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "%*u:  %s\n", 5, index_to_line, text_file_lines[index_to_line]);
        printf("%s", lbuf);
        ++index_to_line;
//        line_not_zero_length = strlen(text_file_lines[index_to_line]);
    }


    show_diag(rname, "done.", dflag_announce);

    return 0;

} // end routine show_lines_from_file()




int free_memory_holding_text_file_data(const char* caller)
{

    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_announce = DIAGNOSTICS_ON;
//    unsigned int dflag_verbose = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("free_memory_holding_text_file_data");


    show_diag(rname, "starting,", dflag_announce);
    unsigned int index_to_line = 0;

    while ( text_file_lines[index_to_line] != NULL )
    {
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "freeing text line %u holding:  %s", index_to_line, text_file_lines[index_to_line]);
        printf("%s", lbuf);
        free(text_file_lines[index_to_line]);

// 2017-04-27 - Some debate about whether to set freed C lanuguage pointers
//  to null after freeing;  see "http://stackoverflow.com/questions/1879550/should-one-really-set-pointers-to-null-after-freeing-them"
        text_file_lines[index_to_line] = NULL;

// move to next line . . .
        ++index_to_line;
    }

    show_diag(rname, "done.", dflag_announce);
    return 0;

} // end routine free_memory_holding_text_file_data()




int break_lines_into_tokens(const char* caller)
{
//----------------------------------------------------------------------
//
//  PURPOSE:  to tokenize and intergerize lines from a text file, lines
//   stored in a program-scoped array.  This program parses lines
//   with expectation that the tokens represent hexadecimal encoded
//   bytes of a BACnet APDU, or Application Protocol Data Unit.  - TMH
//
//
//  HOW THIS ROUTINE WORKS:
//
//   This routine looks for non-null string pointers in program
//   calc-header-crc -- this program's -- array of strings to hold
//   text file lines.  Data structures accessed include the following
//   global arrays . . .
//
//     *  char* text_file_lines[NUMBER_OF_FILE_LINES_STORABLE]
//
//     *  unsigned char pdu_bytes[NUMBER_OF_PROTOCOL_DATA_UNIT_BYTES_STORABLE]
//
//
//   Sample code taken from forum at http://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c . . .
//
//      char str[] = "this, is the string - I want to parse";
//      char delim[] = " ,-";
//      char* token;
//
//      for (token = strtok(str, delim); token; token = strtok(NULL, delim))
//      {
//          printf("token=%s\n", token);
//      }
//
//
//
//
//----------------------------------------------------------------------

// line variables:
    unsigned int index_to_line = 0;

    char line[SIZE_OF_LINE_SUPPORTED];
    memset(line, 0, SIZE_OF_LINE_SUPPORTED);

// token variables:
    char delimiter[] = " ";

    char* token;

// parsing variables:
    long int integer_from_c_strtol = 0;

    char** endptr = NULL;   // see `man s 3 strtol`

    unsigned int index_to_byte = 0;


// diagnostics:

    char lbuf[SIZE__DIAG_MESSAGE];

    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("break_lines_into_tokens");


    show_diag(rname, "starting,", dflag_announce);

//    index_to_line = 0;
    index_to_byte = 0;


    while ( ( text_file_lines[index_to_line] != NULL ) && ( index_to_line < NUMBER_OF_FILE_LINES_STORABLE ) )
    {
        strncpy(line, text_file_lines[index_to_line], SIZE_OF_LINE_SUPPORTED);

        for (token = strtok(line, delimiter); token; token = strtok(NULL, delimiter))
        {

// process tokens from line of file here:

//            snprintf(lbuf, SIZE__DIAG_MESSAGE, "line %d:  %s", index_to_line, token);
//            show_diag(rname, lbuf, dflag_verbose);

            integer_from_c_strtol = strtol(token, endptr, BASE_16_TO_CONVERT_FROM_HEXADECIMAL);

// When C funtion "C to long" returns string with length greater than
// zero, and end pointer holds '\0' then strtol succeeded in parsing
// a number from the string it most recently got:

//            if ( ( strlen(token) > 0 ) && ( *endptr == '\0' ) )
            if ( ( strlen(token) > 0 ) && ( endptr == '\0' ) )
            {
                pdu_bytes[index_to_byte] = ( integer_from_c_strtol & 0xFF );

                snprintf(lbuf, SIZE__DIAG_MESSAGE, "storing BACnet PDU byte '%X' in array element pdu_bytes[%d],",
                  pdu_bytes[index_to_byte], index_to_byte);
                show_diag(rname, lbuf, dflag_verbose);

                ++index_to_byte;
            }
            else
            {
                // --- token was not a valid hexadecimal number ---
            }

        }

        blank_line_out(rname, 1);

        ++index_to_line;
    }





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    show_diag(rname, "done.", dflag_announce);

    return 0;

} // end routine break_lines_into_tokens()







int parse_pdu_data_from_command_line(const char* caller, int argc, char* argv[])
{
//----------------------------------------------------------------------
//
//  PURPOSE:  This routine called when the command line option -d or
//   --data is found.  This routine parses successive arguments
//   expecting them to be in hexadecimal byte format . . .     - TMH
//
//
//
//
//----------------------------------------------------------------------

// local index to step through command line arguments:
    int index_to_arguments = 2;

// local index to global PDU byte array:
    int index_to_pdu_byte_array = 0;


// local string used to hold copy of and thereby preserve argv[] arguments:
    char token[SIZE__TOKEN];

// local pointer to pointer to char used in call to C strtol() library routine:
    char** endptr = NULL;   // see `man s 3 strtol`

// local long integer to hold valid parsed PDU bytes from command line arguments:
    long int latest_pdu_byte = 0;

// count of bytes successfully parsed:
    int count_pdu_bytes_parsed = 0;


    int flag_malformed_byte_found = 0;


// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;
    unsigned int dflag_warning = DIAGNOSTICS_ON;
    DIAG__SET_ROUTINE_NAME("parse_pdu_data_from_command_line");


    show_diag(rname, "starting,", dflag_announce);

    if ( argc < 2 )
    {
        // no bytes to parse if zeroeth and first arguments are all that are present
    }
    else
    {
        for ( index_to_arguments = 2; index_to_arguments < argc; ++index_to_arguments )
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "parsing argv[%d] = %s,", index_to_arguments,
              argv[index_to_arguments]);
            show_diag(rname, lbuf, dflag_verbose);

// NOTE:  C strtol() routine affects the string token it parses, so here
//  send strtol() a copy of present argv[] element:
            strncpy(token, argv[index_to_arguments], SIZE__TOKEN);

            latest_pdu_byte = strtol(token, endptr, BASE_16_TO_CONVERT_FROM_HEXADECIMAL);

            if ( ( strlen(token) > 0 ) && ( endptr == '\0' ) )
            {
                pdu_bytes[index_to_pdu_byte_array] = latest_pdu_byte;
                ++index_to_pdu_byte_array;
            }
            else
            {
                ++flag_malformed_byte_found;
            }

        } // end FOR-loop to step through command line arguments 2 to ending

    } // end IF-ELSE block to parse when there are three or more command line arguments


    if ( flag_malformed_byte_found )
    {
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "WARNING - found %d command line arguments which don't parse as hexadecimal byte values,",
          flag_malformed_byte_found);
        show_diag(rname, lbuf, dflag_warning);
        show_diag(rname, "WARNING - stored as many valid bytes in PDU byte array as possible,", dflag_warning);
        show_diag(rname, "WARNING - yet final CRC calculation may not turn out as expected.", dflag_warning);
        show_diag(rname, "WARNING - Recommand checking byte values entered at most recent", dflag_warning);
        show_diag(rname, "WARNING - invocation of this program.", dflag_warning);
    }


    show_diag(rname, "done.", dflag_announce);
    return count_pdu_bytes_parsed;

} // end routine parse_pdu_data_from_command_line()





int parse_command_line_arguments(const char* caller, int argc, char* argv[])
{
//----------------------------------------------------------------------
//
//  STARTED:  2017-05-01
//
//  PURPOSE:  to parse first command line argument passed to this
//    program.  This program supports an option given by the user in
//    the first argument.  Options supported include,
//
//    -f --file    . . . read PDU data from file,
//
//    -d --data    . . . read PDU data from command line arguments,
//
//
//
//  RETURNS:  number of command line arguments parsed
//
//
//
//----------------------------------------------------------------------


#define INDEX_TO_ARG_HOLDING_OPTION (1)
#define INDEX_TO_ARG_HOLDING_DATA_FILENAME (2)



enum supported_options
{
    OPTION__READ_FILE,
    OPTION__DATA_ON_COMMAND_LINE
};


    int option_parsed = 0;

    int count_arguments_parsed = 0;


// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;
    unsigned int dflag_warning = DIAGNOSTICS_ON;
    DIAG__SET_ROUTINE_NAME("parse_command_line_arguments");



    show_diag(rname, "starting,", dflag_announce);

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "argc holds %d,", argc);
    show_diag(rname, lbuf, dflag_verbose);


//
// Supported command line arguments will be either one of,
//
//     <program name> [ -f | --file ] <filename>
//
//       - or -
//
//     <program name> [ -d | --data ] [byte_1 byte_2 . . . byte_n]
//

    if ( argc < 3 )
    {
        count_arguments_parsed = 0;
    }
    else
    {
//        if ( ( strncmp(argv[1], "-f", strlen("-f")) == 0 ) || ( strncmp(argv[1], "--file", strlen("--file")) == 0 ) )
        if ( ( strncmp(argv[INDEX_TO_ARG_HOLDING_OPTION], "-f", strlen("-f")) == 0 ) || ( strncmp(argv[INDEX_TO_ARG_HOLDING_OPTION], "--file", strlen("--file")) == 0 ) )
        {
            option_parsed = OPTION__READ_FILE;
        }

        if ( ( strncmp(argv[INDEX_TO_ARG_HOLDING_OPTION], "-d", strlen("-d")) == 0 ) || ( strncmp(argv[INDEX_TO_ARG_HOLDING_OPTION], "--data", strlen("--data")) == 0 ) )
        {
            option_parsed = OPTION__DATA_ON_COMMAND_LINE;
        }
    }



    switch(option_parsed)
    {
        case OPTION__READ_FILE:
        {
            strncpy(global_filename__pdu_data_file, argv[INDEX_TO_ARG_HOLDING_DATA_FILENAME], SIZE__FILENAME);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "from command line arguments parsed user data filename '%s',",
              global_filename__pdu_data_file);
            show_diag(rname, lbuf, dflag_verbose);

//            show_diag(rname, "- - -  REMAINDER OF CASE STATEMENT NOT FULLY IMPLEMENTED YET - TMH  - - -", dflag_verbose);
//            read_text_file(rname, "read-property-request-to-device-081064.txt");
            read_text_file(rname, global_filename__pdu_data_file);
        }
        break;


        case OPTION__DATA_ON_COMMAND_LINE:
        {
            parse_pdu_data_from_command_line(rname, argc, argv);
        }
        break;


        default:
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "WARNING - encountered unsupported option '%s',",
              argv[INDEX_TO_ARG_HOLDING_OPTION]);
            show_diag(rname, lbuf, dflag_warning);
        }
        break;

    }


    show_diag(rname, "done.", dflag_announce);

    return count_arguments_parsed;


#undef INDEX_TO_ARG_HOLDING_DATA_FILENAME
#undef INDEX_TO_ARG_HOLDING_OPTION

} // end routine parse_command_line_arguments()






int main (int argc, char* argv[])
{


// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];

    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("main");



    unsigned char bytes_of_header[] = {0x55, 0xFF, 0x02, 0x00, 0x09};   // should have CRC equal to 0xDA, per document "bacnet-encoding.rtf" or "Encoding.doc" page 4 of 42,

    show_diag(rname, "starting,", dflag_announce);

//    ROUTINE__UCBN_CALC_HEADERCRC
    show_diag(rname, "turning on diagnostics for D Bruno routine ucBN_Calc_HeaderCRC() . . .", dflag_verbose);
    diagnostics_flag_set(rname, ROUTINE__UCBN_CALC_HEADERCRC);
    show_binary_number(rname, global_bit_wise_diagnostics_flags_0_through_31);

    show_diag(rname, "turning off diagnostics for D Bruno routine ucBN_Calc_HeaderCRC() . . .", dflag_verbose);
    diagnostics_flag_reset(rname, ROUTINE__UCBN_CALC_HEADERCRC);


// 2017-05-01 - Ted adding command line arguments parsing . . .

    parse_command_line_arguments(rname, argc, argv);




    if ( 0 )
    {
        printf("- TEST 4 -\n");
        printf("initializing header CRC to 0xFF,\n");
        ucBN_HeaderCRC = 0xFF;

        printf("omitting preamble bytes 0x55 and 0xFF in calcuation,\n");
        bytes_of_header[0] = 0x06;   // should have CRC equal to 0xBB, per document "bacnet-encoding.rtf" or "Encoding.doc" page 4 of 42,
        bytes_of_header[1] = 0x00;
        bytes_of_header[2] = 0x00;

        printf("calling routine to calculate sample BACnet packet header CRC . . .\n");
        ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[0], TABLE_LOOK_UP);
        ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[1], TABLE_LOOK_UP);
        ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(bytes_of_header[2], TABLE_LOOK_UP);
        printf("masking to least eight bits:\n");
        printf("header CRC value calculated to be 0x%02X equal to decimal %u,\n", ( ucBN_HeaderCRC & 0xFF ), ( ucBN_HeaderCRC & 0xFF ));
        printf("header CRC value after bit-wise AND is 0x%02X,\n\n", ( ~ucBN_HeaderCRC & 0xFF ));
    }




//    read_text_file(rname, "read-property-request-to-device-081064.txt");

    if ( 0 )
    {
        show_lines_from_file(rname, DEFAULT_OPTION_OF_ZERO);

        break_lines_into_tokens(rname);

        free_memory_holding_text_file_data(rname);

        show_lines_from_file(rname, DEFAULT_OPTION_OF_ZERO);
    }



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 2017-04-28 - We need a routine to calculate the checksum of n bytes
//  in this program's globally scoped array named pdu_bytes[].  Also
//  it would be handy to have a table -- an array -- of BACnet data
//  packets.  The table could hold the length of the packet, its
//  frame number, and the name of the network traffic capture file
//  from which the packet is read . . .
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    {
        printf("- TEST 5 -\n");
        printf("initializing header CRC to 0xFF,\n");
        ucBN_HeaderCRC = 0xFF;

//        for ( int i = 0; i < 45; ++i )
        for ( int i = 0; i < 9; ++i )
        {
//            snprintf(lbuf, SIZE__DIAG_MESSAGE, "sending PDU byte %d to CRC calculating routine . . .", i);
//            show_diag(rname, lbuf, dflag_verbose);

            ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(pdu_bytes[i], CALCULATE_VIA_XOR_OPERATIONS);

            snprintf(lbuf, SIZE__DIAG_MESSAGE, "sending pdu_byte[%u] = 0x%02X to CRC calculator . . . present CRC is 0x%02X",
              i, pdu_bytes[i], (~ucBN_HeaderCRC & 0xFF));
            show_diag(rname, lbuf, dflag_verbose);
        }
    }

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "CRC after taking binary one's compliment and bit-wise ANDing is 0x%02X,\n\n",
      ( ~ucBN_HeaderCRC & 0xFF ));
    show_diag(rname, lbuf, dflag_verbose);

    show_diag(rname, "done.", dflag_announce);

    return 0;

} // end routine main()








// --- EOF ---
