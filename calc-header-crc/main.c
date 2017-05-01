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


// 2017-04-27 - Added by Ted:
#define SIZE_OF_LINE_SUPPORTED (1024)

#define BASE_16_TO_CONVERT_FROM_HEXADECIMAL (16)




//----------------------------------------------------------------------
//  - SECTION - global variables, arrays, structures
//----------------------------------------------------------------------

char* text_file_lines[NUMBER_OF_FILE_LINES_STORABLE];

unsigned char pdu_bytes[NUMBER_OF_PROTOCOL_DATA_UNIT_BYTES_STORABLE];




//----------------------------------------------------------------------
//  - SECTION - routine definitions
//----------------------------------------------------------------------

enum crc_calculation_methods
{
    TABLE_LOOK_UP,
    CALCULATE_VIA_XOR_OPERATIONS
};




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
}




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







#define FAST_BACNET_HEADER_CRC

// //**************************************************************
// static uint8_t ucBN_Calc_HeaderCRC(uint8_t ucData) {
// //**************************************************************

// static unsigned char ucBN_Calc_HeaderCRC(unsigned char ucData)
static unsigned char ucBN_Calc_HeaderCRC(unsigned char ucData, unsigned char method_of_calculation )
{

//     uint16_t uiCRC;
     short unsigned int uiCRC;
     short unsigned int index;


     printf("%s():  received byte of data holding %02X, ucBN_HeaderCRC holds %02X,\n", "ucBN_Calc_HeaderCRC", ucData, ucBN_HeaderCRC);

     uiCRC =  ucData ^ ucBN_HeaderCRC;

// #ifdef FAST_BACNET_HEADER_CRC
//     uiCRC = uiCRC  ^ pgm_read_word(&xpuiBACnet_Header_CRC_Table[uiCRC]);

    if ( method_of_calculation == TABLE_LOOK_UP )
    {
        printf("%s():  using CRC table,\n", "ucBN_Calc_HeaderCRC");
        index = uiCRC;
        uiCRC = uiCRC ^ xpuiBACnet_Header_CRC_Table[index];
    }

// #else // FAST_BACNET_HEADER_CRC
    else
    {
        printf("%s():  calculating CRC via exclusive-OR operations . . .\n", "ucBN_Calc_HeaderCRC");
        uiCRC = uiCRC ^ (uiCRC<<1) ^ (uiCRC<<2) ^ (uiCRC<<3) ^ (uiCRC<<4) ^ (uiCRC<<5) ^ (uiCRC<<6) ^ (uiCRC<<7);
    }

// #endif // FAST_BACNET_HEADER_CRC


     printf("%s():  returning updated ucBN_HeaderCRC equal to %02X . . .\n", "ucBN_Calc_HeaderCRC", (uiCRC&0xFE) ^ ((uiCRC>>8)&1));

     return (uiCRC&0xFE) ^ ((uiCRC>>8)&1);

} // end routine ucBN_Calc_HeaderCRC() 





int main (int argc, char* argv[])
{


// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];

    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("main");



    unsigned char bytes_of_header[] = {0x55, 0xFF, 0x02, 0x00, 0x09};   // should have CRC equal to 0xDA, per document "bacnet-encoding.rtf" or "Encoding.doc" page 4 of 42,

    show_diag(rname, "starting,", dflag_announce);



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



    read_text_file(rname, "read-property-request-to-device-081064.txt");

    show_lines_from_file(rname, DEFAULT_OPTION_OF_ZERO);

    break_lines_into_tokens(rname);

    free_memory_holding_text_file_data(rname);

    show_lines_from_file(rname, DEFAULT_OPTION_OF_ZERO);



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 2017-04-28 - We need a routine to calculate the checksum of n bytes
//  in this program's globally scoped array named pdu_bytes[].  Also
//  it would be handy to have a table -- an array -- of BACnet data
//  packets.  The table could hold the length of the packet, its
//  frame number, and the name of the network traffic capture file
//  from which the packet is read . . .
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    {
        for ( int i = 0; i < 45; ++i )
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "sending PDU byte %d to CRC calculating routine . . .", i);
            show_diag(rname, lbuf, dflag_verbose);

            ucBN_HeaderCRC = ucBN_Calc_HeaderCRC(pdu_bytes[i], CALCULATE_VIA_XOR_OPERATIONS);
        }
    }


    show_diag(rname, "done.", dflag_announce);

    return 0;

} // end routine main()







//----------------------------------------------------------------------
//  - SECTION - code from earlier development steps
//----------------------------------------------------------------------

int old_tests()
{

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

    printf("header CRC value after bit-wise AND is 0x%02X,\n\n", ( ~ucBN_HeaderCRC & 0xFF ));



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





// --- EOF ---
