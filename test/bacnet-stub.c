//======================================================================
//
//  FILE:  bacnet-stub.c
//
//
//  DESCRIPTION:
//
//
//  COMPILE STEPS:
//
//    $ gcc bacnet-stub.c -g3 -Wall -I/usr/local/include/libtestlib-0p1 -I/home/ted/projects/bacnet/steve-karg-stack/tags/bacnet-stack-0.8.3/include -I/home/ted/projects/bacnet/steve-karg-stack/tags/bacnet-stack-0.8.3/ports/linux -L/usr/local/lib/libtestlib-0p1 -ltestlib-0p1 -lkargs-bacnet-mstp-0p8p3 -lm -lpthread -o b.out
//
//    $ gcc bacnet-stub.c -g3 -Wall -I/usr/local/include/libtestlib-0p1 -I/home/ted/projects/bacnet/steve-karg-stack/tags/bacnet-stack-0.8.3/include -I/home/ted/projects/bacnet/steve-karg-stack/tags/bacnet-stack-0.8.3/ports/linux -I/home/ted/projects/bacnet/steve-karg-stack/tags/bacnet-stack-0.8.3/demo/object -L/usr/local/lib/libtestlib-0p1 -ltestlib-0p1 -lkargs-bacnet-mstp-0p8p3 -lm -lpthread -o b.out
//
//
//
//  EXECUTE a.out STEPS:
//
//    $ export LD_LIBRARY_PATH=/usr/local/lib/libtestlib-0p1:$LD_LIBRARY_PATH
//
//    $ ./b.out
//
//
//
//  REFERENCES:
//
//    *  https://www.cs.swarthmore.edu/~newhall/unixhelp/howto_C_libraries.html
//
//
//
//
//======================================================================





//----------------------------------------------------------------------
// - SECTION - pound includes, headers . . .
//----------------------------------------------------------------------


// # Standard C library headers:
#include <stdlib.h>

// #include <pthread.h> . . . not needed directly by this BACnet test
//   program, yet needed to compile shared object library file.  Compile
//   that project with -pthread option to gcc - TMH 2017-03-07




// # A header file of Steve Kargs' open source BACnet stack project:
#include <mstp.h>         // provides definition of structure mstp_port_struct_t,
                          // 2017-02-27 - should also provide as it has function protoype of routine uint16_t MSTP_Put_Receive() - TMH

// #include <dlmstp.h>       // provides MAX_MPDU,

// 2017-02-27 MON - in ../bacnet-stack-0.8.3/ports/linux/timer.h:
#include <timer.h>        // provides TIMER_SILENCE pound define, and uint32_t timer_milliseconds() routine,

#include <rs485.h>        // provides routine RS485_Set_Interface(),

#include <datalink.h>     // provides pound define for 'datalink_cleanup',

#include <dlenv.h>        // provides routine dlenv_init(), (Data Link Environment variables parsing and handling),

// 2017-04-03 MON - added by Ted:
#include <address.h>      // provides routine address_init(),

#include <device.h>       // provides routine Device_Init(),

#include <handlers.h>     // provides routine handler_who_is(),



// # A test library header file of Ted's:
#include <diagnostics.h>






//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define SIZE__MSTP_INPUT_BUFFER (1024)




//----------------------------------------------------------------------
// - SECTION - global variables
//----------------------------------------------------------------------

// Following design pattern of Steve Karg's demonstration open source
// project, source file ~/projects/bacnet/steve-karg-stack/tags/bacnet-stack-0.8.3/demo/mstpcap/main.c
// in which a global instance of mstp_port_struct_t is declared at
// top of main.c, and then a local pointer to this structure is
// declared within routine main() . . .

static volatile struct mstp_port_struct_t MSTP_Port;
/* buffers needed by mstp port struct */
static uint8_t RxBuffer[MAX_MPDU];
static uint8_t TxBuffer[MAX_MPDU];
/* method to tell main loop to exit from CTRL-C or other signals */
static volatile bool Exit_Requested;





//----------------------------------------------------------------------
// - SECTION - function prototypes
//----------------------------------------------------------------------



// 2017-03-01 - NEED TO:  try commenting this out:

// extern void RS485_Send_Frame(
//     volatile struct mstp_port_struct_t *mstp_port,      /* port specific data */
//     uint8_t * buffer,   /* frame to send (up to 501 bytes of data) */
//     uint16_t nbytes
// );



/* functions used by the MS/TP state machine to put or get data */
/* FIXME: developer must implement these in their DLMSTP module */

// extern uint16_t MSTP_Put_Receive(volatile struct mstp_port_struct_t *mstp_port);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 2017-03-01 WED - Ted commenting out the following function
//  prototypes as these functions are statically defined and therefore
//  cannot be exposed and made available to our stub program, this
//  program, which was trying to reach these functions in a shared
//  object library:
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// // extern static uint32_t Timer_Silence(void *pArg);   // <-- gcc error "multiple storage classes in declaration specifiers"
// extern uint32_t Timer_Silence(void *pArg);
//
// extern void Timer_Silence_Reset(void);




//----------------------------------------------------------------------
// - SECTION - function definitions
//----------------------------------------------------------------------


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//

// 2017-03-01 - NEED TO:  try commenting this out:


// uint16_t MSTP_Get_Reply(
//     volatile struct mstp_port_struct_t * mstp_port,
//     unsigned timeout)
// {       /* milliseconds to wait for a packet */
//     (void) mstp_port;
//     (void) timeout;
//     return 0;
// }




// 2017-03-01 - Added by Ted as these are statically defined and can't
//  be accessed via shared object library:

static uint32_t Timer_Silence(
    void *pArg)
{
    return timer_milliseconds(TIMER_SILENCE);
}



static void Timer_Silence_Reset(
    void *pArg)
{
    timer_reset(TIMER_SILENCE);
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 2017-03-31 - added by Ted, defined multiple times across most of
//  Kargs' BACnet demo programs.  Following routine copied from
//  .../bacnet-stack-0.8.3--verbose/demo/readprop/main.c . . .
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//
// 2017-04-03 MON - Ted commenting this out, about to include readprop
//  demo in shared object . . . ahh we can't comment out this routine,
//  as it's statically declared and not accessible outside its source
//  file.  Ok, we'll return to debugging the compile-time issues we
//  see with this static routine copied here to Ted's nacient BACnet
//  test program:
//


/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/


// Steve Kargs' comment block, followed by Ted's attempt to prototype this routine:

/** Handler for a ReadProperty ACK.
 * @ingroup DSRP
 * Doesn't actually do anything, except, for debugging, to
 * print out the ACK data of a matching request.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */

void My_Read_Property_Ack_Handler(
  uint8_t * service_request,
  uint16_t service_len,
  BACNET_ADDRESS * src,
  BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data
);



static void Init_Service_Handlers(void)
{
    Device_Init(NULL);

// *  we need to handle who-is to support dynamic device binding to us * /
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);


// *  handle i-am to support binding to other devices * /
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);


// *  set the handler for all the services we don't implement
//       It is required to send the proper reject message... * /
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);


// *  we must implement read property - it's required! * /
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);


// * handle the data coming back from confirmed requests * /
    apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        My_Read_Property_Ack_Handler);


// * handle any errors coming back * /
    apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);

} // end routine Init_Service_Handlers()

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - SECTION - main line code
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main(int argc, char** argv)
{

    int array_size;


// diagnostics:

    char lbuf[SIZE__DIAG_MESSAGE];

    unsigned int dflag_announce = DIAGNOSTICS_ON;
    unsigned int dflag_verbose = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("main");



    show_diag(rname, "starting,", dflag_announce);
    memset(lbuf, 1, SIZE__DIAG_MESSAGE);
    lbuf[(SIZE__DIAG_MESSAGE - 1)] = 0;

    array_size = strlen(lbuf);
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "local buffer for messages found to be %d bytes long,", array_size);
    show_diag(rname, lbuf, dflag_verbose);


    show_diag(rname, "defining a pointer variable to a structure of type mstp_port_struct_t . . .", dflag_verbose);

    volatile static struct mstp_port_struct_t* mstp_port;

    if ( mstp_port == NULL ) { }



// QUESTION 2017-03-31:  why is following code commented out?  - TMH

/*
    show_diag(rname, "allocating some memory to key members of mstp_port instance:", dflag_verbose);
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "via malloc() giving %d bytes to mstp_port->InputBuffer . . .", SIZE__MSTP_INPUT_BUFFER);
    show_diag(rname, lbuf, dflag_verbose);

    mstp_port->InputBuffer = malloc(SIZE__MSTP_INPUT_BUFFER);


    show_diag(rname, "back from call to malloc(), checking whether mstp_port->InputBuffer is not null,", dflag_verbose);

    if ( mstp_port->InputBuffer != NULL )
    {
        show_diag(rname, "mstp_port->InputBuffer not null so calling free() on this allocated memory . . .", dflag_verbose);
        free(mstp_port->InputBuffer);
    }
*/



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - STEP - reusing several lines of MSTP port initializing code from 
//          Steve Kargs' mstpcap demo main.c source file:
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// 2017-03-31 - Ted not sure we'll need the following IF-scoped code:

    if ( 1 )
    {
        MSTP_Port.InputBuffer = &RxBuffer[0];
        MSTP_Port.InputBufferSize = sizeof(RxBuffer);
        MSTP_Port.OutputBuffer = &TxBuffer[0];
        MSTP_Port.OutputBufferSize = sizeof(TxBuffer);
        MSTP_Port.This_Station = 127;
        MSTP_Port.Nmax_info_frames = 1;
        MSTP_Port.Nmax_master = 127;
        MSTP_Port.SilenceTimer = Timer_Silence;
        MSTP_Port.SilenceTimerReset = Timer_Silence_Reset;
        /* mimic our pointer in the state machine */
        mstp_port = &MSTP_Port;
    }



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - STEP - a couple of early tests, likely not necessary now seeing
//          how demos bacwi and bacrp work:
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//    show_diag(rname, "calling MSTP_Init() to see whether this call compiles without error . . .", dflag_verbose);
//    MSTP_Init(mstp_port);

//    show_diag(rname, "calling MSTP_Slave_Node_FSM() to see whether this call compiles without error . . .", dflag_verbose);
//    MSTP_Slave_Node_FSM(mstp_port);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - STEP -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);

    address_init();

    Init_Service_Handlers();

    dlenv_init();

    atexit(datalink_cleanup);



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - STEP -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    show_diag(rname, "calling ~/ports/linux/rs485.c routine to set serial port . . .", dflag_verbose);
    RS485_Set_Interface("/dev/ttyUSB0");


    show_diag(rname, "done.", dflag_announce);

    return 0;

} // end routine main()





// --- EOF ---
