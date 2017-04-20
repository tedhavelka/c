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
//  KNOWN ISSUES:
//
//    2017-04-15 - function pointer datalink_receive() is not
//    correctly assigned a valid function, in Ted's 2017 case the
//    address of Steve Kargs' function dlmstp_receive().  This
//    mis-assignment doesn't create any error or warning at compile
//    time, yet at run time the main communications loop executes
//    several hundred times faster than normal and doesn't ever receive
//    any BACnet frames or bytes back from the local twisted pair
//    EIA-485 network.  Ted noting from found bug last week that a
//    a function may be defined with no code at all, just its name and
//    a curly brace pair, and compile and run.  Very hard to trace
//    this unintentional syntax error in gdb, when there are no lines
//    of code on which to break.
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

#include <time.h>




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

#include <handlers.h>     // provides routine handler_who_is(), NOTE - SHOULD ALSO PROVIDE handler_i_am_bind() . . .  - TMH

// 2017-04-04 TUE - added by Ted:

#include <bactext.h>     // provides routine ___

#include <client.h>      // provides routine Send_Read_Property_Request(),

#include <tsm.h>         // provides routine tsm_invoke_id_free(), tsm_invoke_id_failed() and tsm_free_invoke_id(),

// 2017-04-13 THU - added

#include <dlmstp.h>      // provides DLMSTP_PACKE.


// # A test library header file of Ted's:
#include <diagnostics.h>






//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define SIZE__MSTP_INPUT_BUFFER (1024)

#define ONE_MILLION_MS (1000 * 1000)
#define MILLI_SECONDS_200K (200 * 1000)
#define MAX_LOOP_CYCLES_TO_EXECUTE (1000)  // as of 2017-04-06 morning was (30) - TMH

// 2017-04-20 - added:
#define TIMES_TO_REPEAT_COMMS_LOOP (10)



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


// 2017-04-04 - Needed and copied from Kargs' ~0.8.3/demo/readprop/main.c source file:

static BACNET_ADDRESS Target_Address;

static uint8_t Request_Invoke_ID = 0;

static bool Error_Detected = false;

static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;


static uint32_t Target_Object_Instance = BACNET_MAX_INSTANCE;
static BACNET_OBJECT_TYPE Target_Object_Type = OBJECT_ANALOG_INPUT;
static BACNET_PROPERTY_ID Target_Object_Property = PROP_ACKED_TRANSITIONS;
static int32_t Target_Object_Index = BACNET_ARRAY_ALL;

// 2017-04-05 -

static uint8_t Rx_Buf[MAX_MPDU] = { 0 };



// 2017-04-13 -

/** @file linux/dlmstp.c  Provides Linux-specific DataLink functions for MS/TP. */

/* Number of MS/TP Packets Rx/Tx */
uint16_t MSTP_Packets = 0;

/* packet queues */
static DLMSTP_PACKET Receive_Packet;
/* mechanism to wait for a packet */
/*
static RT_COND Receive_Packet_Flag;
static RT_MUTEX Receive_Packet_Mutex;
*/
// static pthread_cond_t Receive_Packet_Flag;
// static pthread_mutex_t Receive_Packet_Mutex;




//----------------------------------------------------------------------
// - SECTION - function prototypes
//----------------------------------------------------------------------

/*
static void get_abstime(
    struct timespec *abstime,
    unsigned long milliseconds)
{
    struct timeval now, offset, result;

    gettimeofday(&now, NULL);
    offset.tv_sec = 0;
    offset.tv_usec = milliseconds * 1000;
    timeradd(&now, &offset, &result);
    abstime->tv_sec = result.tv_sec;
    abstime->tv_nsec = result.tv_usec * 1000;
}
*/



// 2017-04-13 - copied to here from ~0.8.3--verbose/ports/linux/dlmsp.c:

// 2017-04-13 - prototype alone here did not work:

extern uint16_t dlmstp_receive(
  BACNET_ADDRESS * src,  /* source address */
  uint8_t * pdu,         /* PDU data */
  uint16_t max_pdu,      /* amount of space available in the PDU  */
  unsigned timeout       /* milliseconds to wait for a packet */
);




/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

uint16_t dlmstp_receive(
  BACNET_ADDRESS * src,  // / * source address * /
  uint8_t * pdu,         // / * PDU data * /
  uint16_t max_pdu,      // / * amount of space available in the PDU  * /
  unsigned timeout       // / * milliseconds to wait for a packet * /
)
{
    uint16_t pdu_len = 0;
    struct timespec abstime;

    (void) max_pdu;

// diagnostics:
    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_verbose = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("dlmstp_receive()");


    show_diag(rname, "starting,", dflag_verbose);

//    / * see if there is a packet available, and a place
//       to put the reply (if necessary) and process it * /
    pthread_mutex_lock(&Receive_Packet_Mutex);
    get_abstime(&abstime, timeout);
    pthread_cond_timedwait(&Receive_Packet_Flag, &Receive_Packet_Mutex,
        &abstime);

    show_diag(rname, "checking whether Receive_Packet.ready is true,", dflag_verbose);
    if (Receive_Packet.ready)
    {
        show_diag(rname, "checking whether Receive_Packet.pud_len is not zero,", dflag_verbose);
        if (Receive_Packet.pdu_len)
        {
            MSTP_Packets++;
            if (src)
            {
                show_diag(rname, "'src' passed by reference and not null, copying to 'src' receive packet address . . .", dflag_verbose);
                memmove(src, &Receive_Packet.address, sizeof(Receive_Packet.address));
            }

            if (pdu)
            {
                show_diag(rname, "'pdu' passed by reference and not null, copying to 'pdu' receive packet PDU bytes . . .", dflag_verbose);
                memmove(pdu, &Receive_Packet.pdu, sizeof(Receive_Packet.pdu));
            }

            pdu_len = Receive_Packet.pdu_len;
        }

        Receive_Packet.ready = false;
    }

    pthread_mutex_unlock(&Receive_Packet_Mutex);

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "returning pdu_len = %d to caller . . .", pdu_len);
    show_diag(rname, lbuf, dflag_verbose);
    return pdu_len;
}

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/




void MyAbortHandler(    // <-- function prototype, function defined in Kargs' ~0.8.3/demo/readprop/main.c - TMH
  BACNET_ADDRESS * src,
  uint8_t invoke_id,
  uint8_t abort_reason,
  bool server
);


// 2017-04-03 - NEED to copy static routine MyErrorHandler() from readprop/main.c - TMH
//
// 2017-04-04 - Copied, but new compile-time errors now appear.
//  Compiler complains that Target_Address and Request_Invoke_ID are
//  not declared.  Also some warnings about implicit function
//  declarations . . .
//
//

static void MyErrorHandler(
  BACNET_ADDRESS * src,
  uint8_t invoke_id,
  BACNET_ERROR_CLASS error_class,
  BACNET_ERROR_CODE error_code)
{
    if (address_match(&Target_Address, src) && (invoke_id == Request_Invoke_ID))
    {
        printf("BACnet Error: %s: %s\r\n",
            bactext_error_class_name((int) error_class),
            bactext_error_code_name((int) error_code));
        Error_Detected = true;
    }
}



void MyRejectHandler(
  BACNET_ADDRESS * src,
  uint8_t invoke_id,
  uint8_t reject_reason
);







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
)
{
    int len = 0;
    BACNET_READ_PROPERTY_DATA data;


//    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_announce   = DIAGNOSTICS_ON;
    unsigned int dflag_verbose    = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("bacnet-stub copy of Kargs My_Read_Property_Ack_Handler()");


    show_diag(rname, "starting,", dflag_announce);

    if (address_match(&Target_Address, src) && (service_data->invoke_id == Request_Invoke_ID))
    {
        show_diag(rname, "calling rp_ack_decode_service_request() . . .", dflag_verbose);
        len = rp_ack_decode_service_request(service_request, service_len, &data);

        if (len > 0)
        {
            show_diag(rname, "got back some bytes from latest routine call,", dflag_verbose);
            show_diag(rname, "calling rp_ack_print_data() . . .", dflag_verbose);
            rp_ack_print_data(&data);
        }
    }

    show_diag(rname, "returning to calling code . . .", dflag_announce);

}




static void Init_Service_Handlers(void)
{
//    char lbuf[SIZE__DIAG_MESSAGE];
    unsigned int dflag_announce   = DIAGNOSTICS_ON;
    unsigned int dflag_verbose    = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("Init_Service_Handlers()");


    show_diag(rname, "starting,", dflag_announce);

    show_diag(rname, "Note:  this routine copied from Kargs' demo program named bacrp,", dflag_announce);

    show_diag(rname, "- INIT SERVICE HANDLERS 1 - calling Device_Init() . . .", dflag_announce);
    Device_Init(NULL);

// *  we need to handle who-is to support dynamic device binding to us * /
    show_diag(rname, "- INIT SERVICE HANDLERS 2 - calling apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is) . . .",
      dflag_verbose);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);


// *  handle i-am to support binding to other devices * /
    show_diag(rname, "- INIT SERVICE HANDLERS 3 - calling apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind) . . .",
      dflag_verbose);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);


// *  set the handler for all the services we don't implement
//       It is required to send the proper reject message... * /
    show_diag(rname, "- INIT SERVICE HANDLERS 4 - calling apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service) . . .",
      dflag_verbose);
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);


// *  we must implement read property - it's required! * /
    show_diag(rname, "- INIT SERVICE HANDLERS 5 - calling apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY, handler_read_property) . . .",
      dflag_verbose);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);


// * handle the data coming back from confirmed requests * /
    show_diag(rname, "- INIT SERVICE HANDLERS 6 - calling apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY, My_Read_Property_Ack_Handler) . . .",
      dflag_verbose);
    apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        My_Read_Property_Ack_Handler);


// * handle any errors coming back * /
    show_diag(rname, "- INIT SERVICE HANDLERS 7 - calling apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler) . . .", dflag_verbose);
    apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
    show_diag(rname, "- INIT SERVICE HANDLERS 8 - calling apdu_set_abort_handler(MyAbortHandler) . . .", dflag_verbose);
    apdu_set_abort_handler(MyAbortHandler);
    show_diag(rname, "- INIT SERVICE HANDLERS 9 - calling apdu_set_reject_handler(MyRejectHandler) . . .", dflag_verbose);
    apdu_set_reject_handler(MyRejectHandler);


    show_diag(rname, "returning to calling code . . .", dflag_announce);

} // end routine Init_Service_Handlers()

/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/




int comms_test_2(int argc, char** argv)
{
//----------------------------------------------------------------------
// 2017-04-19 NOTES:  this routine has been here about one week, and
//  is now working as expected.  Ted leaving this routine in place and
//  continuing development in a copy of this routine.  Next dev steps
//  include amending code to send "Read Property" requests multiple
//  times.  Must figure out any BACnet state machine steps which our
//  test program code must attend to, between requests . . .  - TMH
//
//----------------------------------------------------------------------


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Variables copied from Steve Kargs' demo 'bacrp':

    BACNET_ADDRESS src = {
        0
    };  /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 100;     /* milliseconds */
    unsigned max_apdu = 0;
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    bool found = false;

// END OF:  Kargs' variabels
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


//    bool time_out_not_reached = true;

    long int loop_cycles_completed = 0;

//    int return_value = 0;
//    int return_value__id_free = 0;
//    int return_value__invoke_id_failed = 0;

    int seconds_passed = 0;

// diagnostics:

    char lbuf[SIZE__DIAG_MESSAGE];

    unsigned int dflag_announce   = DIAGNOSTICS_ON;
    unsigned int dflag_verbose    = DIAGNOSTICS_ON;
    unsigned int dflag_comms_loop = DIAGNOSTICS_ON;

//    unsigned int dflag_mark = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("bacnet-stub comms_test_2()");


    show_diag(rname, "starting,", dflag_announce);

    Target_Device_Object_Instance = 133005;
    Target_Object_Type = 8;
    Target_Object_Instance = 133005;
    Target_Object_Property = 76;


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// BACnet comm's initialization code copied from Steve Kargs' demo named bacrp:

    /* setup my info */
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    address_init();
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    /* configure the timeout values */
    last_seconds = time(NULL);
    timeout_seconds = (apdu_timeout() / 1000) * apdu_retries();
    /* try to bind with the device */
    found =
        address_bind_request(Target_Device_Object_Instance, &max_apdu,
        &Target_Address);
    if (!found) {
        Send_WhoIs(Target_Device_Object_Instance,
            Target_Device_Object_Instance);
    }

// END OF:  Karg's bacrp code snippet
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *







//    show_diag(rname, "entering communiations test 2 loop:", dflag_verbose);

    if ( 1 )
    {
        show_diag(rname, "about to enter comm's loop,", dflag_verbose);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "Target_Device_Object_Instance holds '%d',", Target_Device_Object_Instance);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "Target_Object_Type holds '%d',", Target_Object_Type);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "Target_Object_Instance holds '%d',", Target_Object_Instance);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "Target_Object_Property holds '%d',", Target_Object_Property);
        show_diag(rname, lbuf, dflag_verbose);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "variable Request_Invoke_ID holds '%d',", Request_Invoke_ID);
        show_diag(rname, lbuf, dflag_verbose);

        show_diag(rname, "entering communications loop:", dflag_verbose);
    }





//    while (( time_out_not_reached ) && ( loop_cycles_completed < MAX_LOOP_CYCLES_TO_EXECUTE ))
    for (;;)
    {

//        if ( ( loop_cycles_completed % 1000000 ) == 0 )
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "-  COMMUNICATIONS LOOP - executing now %ld times,", (loop_cycles_completed + 1));
            show_diag(rname, lbuf, dflag_comms_loop);
        }




// Kargs' bacrp code:

        /* increment timer - exit if timed out */
        current_seconds = time(NULL);

        /* at least one second has passed */
        if (current_seconds != last_seconds)
        {
            ++seconds_passed;
            blank_line_out(rname, 1);
            show_diag(rname, "*** * * * * * * * * * * * * * ***", dflag_comms_loop);
//            show_diag(rname, "***   ONE SECOND HAS PASSED   ***", dflag_comms_loop);
snprintf(lbuf, SIZE__DIAG_MESSAGE, "***   SECONDS PASSED:  %d   ***", seconds_passed);
            show_diag(rname, lbuf, dflag_comms_loop);
            show_diag(rname, "*** * * * * * * * * * * * * * ***", dflag_comms_loop);
            blank_line_out(rname, 1);
            tsm_timer_milliseconds((uint16_t) ((current_seconds - last_seconds) * 1000));
        }

        if (Error_Detected)
        {
            show_diag(rname, "error detected and noted in variable named 'Error_Detected'!", dflag_verbose);
            show_diag(rname, "breaking out of communications loop . . .", dflag_verbose);
            break;
        }

        /* wait until the device is bound, or timeout and quit */
        if (!found)
        {
            show_diag(rname, "calling address_bind_request() . . .", dflag_comms_loop);
            found = address_bind_request(Target_Device_Object_Instance, &max_apdu, &Target_Address);
        }

        if (found)
        {
//            show_diag(rname, "calling tsm_invoke_id_free() . . .", dflag_comms_loop);
//            return_value__id_free = tsm_invoke_id_free(Request_Invoke_ID);
//
//            show_diag(rname, "calling tsm_invoke_id_failed() . . .", dflag_comms_loop);
//            return_value__invoke_id_failed = tsm_invoke_id_failed(Request_Invoke_ID);

            if (Request_Invoke_ID == 0)
            {
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "variable found holds %d, and variable Request_Invoke_ID holds %d,",
                  found, Request_Invoke_ID);
                show_diag(rname, lbuf, dflag_verbose);

                show_diag(rname, "calling Send_Read_Property_Request() . . .", dflag_comms_loop);
                Request_Invoke_ID =
                    Send_Read_Property_Request(Target_Device_Object_Instance,
                    Target_Object_Type, Target_Object_Instance,
                    Target_Object_Property, Target_Object_Index);

                snprintf(lbuf, SIZE__DIAG_MESSAGE, "assigned return value of Send_Read_Property_Request(), Request_Invoke_ID now equals %d,", 
                  Request_Invoke_ID);
                show_diag(rname, lbuf, dflag_verbose);
                show_diag(rname, "we only make a \"read property request\" when Request_Invoke_ID equals zero,", dflag_verbose);
            }

            else if (tsm_invoke_id_free(Request_Invoke_ID))
            {
                show_diag(rname, "call to tsm_invoke_id_free(Request_Invoke_ID) returns true,", dflag_verbose);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "and Request_Invoke_ID presently = %d,", Request_Invoke_ID);
                show_diag(rname, lbuf, dflag_verbose);
                show_diag(rname, "breaking out of communications loop . . .", dflag_verbose);
                break;
            }

            else if (tsm_invoke_id_failed(Request_Invoke_ID))
            {
                fprintf(stderr, "\rError: TSM Timeout!\r\n");

                show_diag(rname, "call to tsm_invoke_id_failed(Request_Invoke_ID) returns true,", dflag_verbose);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "and Request_Invoke_ID presently = %d,", Request_Invoke_ID);
                show_diag(rname, lbuf, dflag_verbose);
                show_diag(rname, "setting Error_Detected to 'true',", dflag_verbose);
                show_diag(rname, "breaking out of communications loop . . .", dflag_verbose);

                tsm_free_invoke_id(Request_Invoke_ID);
                Error_Detected = true;
                /* try again or abort? */
                break;
            }
        }
        else
        {
            /* increment timer - exit if timed out */
            elapsed_seconds += (current_seconds - last_seconds);
            if (elapsed_seconds > timeout_seconds)
            {
                printf("\rError: APDU Timeout!\r\n");
                Error_Detected = true;
                break;
            }
        }


        /* returns 0 bytes on timeout */
//        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

        if ( 0 )
        {
            show_diag(rname, "calling datalink_receive(), really dlmstp_receive() . . .", dflag_comms_loop);
            pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
        }
        else
        {
            show_diag(rname, "calling dlmstp_receive() directly . . .", dflag_comms_loop);
            pdu_len = dlmstp_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
        }


        /* process */
        show_diag(rname, "checking whether pdu_len not zero,", dflag_comms_loop);
        if (pdu_len)
        {
            show_diag(rname, "pdu_len not zero, calling Kargs' routine npdu_handler() . . .", dflag_comms_loop);
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }


        /* keep track of time for next check */
        last_seconds = current_seconds;



// Ted's code . . .

        ++loop_cycles_completed;


    } // end WHILE-loop to realize BACnet communications


    show_diag(rname, "leaving main communcations loop,", dflag_verbose);
    show_diag(rname, "returning to caller . . .", dflag_announce);

    return 0;

} // end routine comms_test_2()





int comms_test_3(int argc, char** argv)
{
//----------------------------------------------------------------------
// 2017-04-19:  started . . . this routine entails work on getting
//   multiple BACnet requests sent with the correct timing to obtain
//   readings, send bad packets, and see what happens, while following
//   the timing constraints of BACnet protocol.  - TMH
//
//
//
//----------------------------------------------------------------------


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Variables copied from Steve Kargs' demo 'bacrp':

    BACNET_ADDRESS src = { 0 };  /* address where message came from */

    uint16_t pdu_len = 0;
    unsigned timeout = 100;     /* milliseconds */
    unsigned max_apdu = 0;
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    bool found = false;

// END OF:  Kargs' variabels
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


//    bool time_out_not_reached = true;

    long int loop_cycles_completed = 0;

//    int return_value = 0;
//    int return_value__id_free = 0;
//    int return_value__invoke_id_failed = 0;

    int seconds_passed = 0;

// diagnostics:

    char lbuf[SIZE__DIAG_MESSAGE];

// Ted looking to use this diagnostics flag in conjunction with routine
// + which changes its value at run time, and add some flexibility to
// + the verbose level of diagnostics which Ted's "dflag" variables
// + provide in a very simple way:
    int routine_scoped_silence_flag = 1;

    unsigned int dflag_announce   = DIAGNOSTICS_ON;
    unsigned int dflag_verbose    = DIAGNOSTICS_ON;
    unsigned int dflag_comms_loop = DIAGNOSTICS_ON;
//    unsigned int dflag_announce   = ( DIAGNOSTICS_ON & routine_scoped_silence_flag );
//    unsigned int dflag_verbose    = ( DIAGNOSTICS_ON & routine_scoped_silence_flag );
//    unsigned int dflag_comms_loop = ( DIAGNOSTICS_ON & routine_scoped_silence_flag );

//    unsigned int dflag_mark = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("bacnet-stub comms_test_3()");


    show_diag(rname, "starting,", dflag_announce);

    Target_Device_Object_Instance = 133005;
    Target_Object_Type = 8;
    Target_Object_Instance = 133005;
    Target_Object_Property = 76;


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// BACnet comm's initialization code copied from Steve Kargs' demo named bacrp:

    /* setup my info */
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    address_init();
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    /* configure the timeout values */
    last_seconds = time(NULL);
    timeout_seconds = (apdu_timeout() / 1000) * apdu_retries();
    /* try to bind with the device */
    found =
        address_bind_request(Target_Device_Object_Instance, &max_apdu,
        &Target_Address);
    if (!found) {
        Send_WhoIs(Target_Device_Object_Instance,
            Target_Device_Object_Instance);
    }

// END OF:  Karg's bacrp code snippet
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *







//    show_diag(rname, "entering communiations test 2 loop:", dflag_verbose);

    if ( 1 )
    {
        show_diag(rname, "about to enter comm's loop,", dflag_verbose);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "Target_Device_Object_Instance holds '%d',", Target_Device_Object_Instance);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "Target_Object_Type holds '%d',", Target_Object_Type);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "Target_Object_Instance holds '%d',", Target_Object_Instance);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "Target_Object_Property holds '%d',", Target_Object_Property);
        show_diag(rname, lbuf, dflag_verbose);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "variable Request_Invoke_ID holds '%d',", Request_Invoke_ID);
        show_diag(rname, lbuf, dflag_verbose);

        show_diag(rname, "entering communications loop:", dflag_verbose);
        blank_line_out(rname, 3);
    }



    int i = 0;

//    while (( time_out_not_reached ) && ( loop_cycles_completed < MAX_LOOP_CYCLES_TO_EXECUTE ))
    while ( i < TIMES_TO_REPEAT_COMMS_LOOP )
    {
        ++i;
        blank_line_out(rname, 2);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "--- sending \"Read Request\" attempt %d:   ---", i);
        show_diag(rname, lbuf, dflag_verbose);

        loop_cycles_completed = 0;

//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Ted resetting Kargs' variable which appears to be a flag relating to 
// BACnet device address being discovered, but Ted not yet sure.
//
// Similarly resetting variable Request_Invoke_ID, in effort to
// repeat BACnet requests without going through initialization steps
// again:
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
        found = 0;

        Request_Invoke_ID = 0;


        for (;;)
        {

//            if ( ( loop_cycles_completed % 1000000 ) == 0 )
            {
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "-  COMMUNICATIONS LOOP - executing now %ld times,", (loop_cycles_completed + 1));
                show_diag(rname, lbuf, dflag_comms_loop);
            }




// Kargs' bacrp code:

        /* increment timer - exit if timed out */
            current_seconds = time(NULL);

        /* at least one second has passed */
            if (current_seconds != last_seconds)
            {
                ++seconds_passed;
                blank_line_out(rname, 1);
                show_diag(rname, "*** * * * * * * * * * * * * * ***", dflag_comms_loop);
//                show_diag(rname, "***   ONE SECOND HAS PASSED   ***", dflag_comms_loop);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "***   SECONDS PASSED:  %d   ***", seconds_passed);
                show_diag(rname, lbuf, dflag_comms_loop);
                show_diag(rname, "*** * * * * * * * * * * * * * ***", dflag_comms_loop);
                blank_line_out(rname, 1);
                tsm_timer_milliseconds((uint16_t) ((current_seconds - last_seconds) * 1000));
            }

            if (Error_Detected)
            {
                show_diag(rname, "error detected and noted in variable named 'Error_Detected'!", dflag_verbose);
                show_diag(rname, "breaking out of communications loop . . .", dflag_verbose);
                blank_line_out(rname, 1);
                break;
            }

        /* wait until the device is bound, or timeout and quit */
            if (!found)
            {
                show_diag(rname, "calling address_bind_request() . . .", dflag_comms_loop);
                found = address_bind_request(Target_Device_Object_Instance, &max_apdu, &Target_Address);
            }

            if (found)
            {
//                show_diag(rname, "calling tsm_invoke_id_free() . . .", dflag_comms_loop);
//                return_value__id_free = tsm_invoke_id_free(Request_Invoke_ID);
//
//                show_diag(rname, "calling tsm_invoke_id_failed() . . .", dflag_comms_loop);
//                return_value__invoke_id_failed = tsm_invoke_id_failed(Request_Invoke_ID);

                show_diag(rname, "variable 'found' now set to 'true', the return value from calling address_bind_request(),", dflag_verbose);


                if (Request_Invoke_ID == 0)
                {
                    snprintf(lbuf, SIZE__DIAG_MESSAGE, "variable found holds %d, and variable Request_Invoke_ID holds %d,",
                      found, Request_Invoke_ID);
                    show_diag(rname, lbuf, dflag_verbose);

                    show_diag(rname, "calling Send_Read_Property_Request() . . .", dflag_comms_loop);
                    Request_Invoke_ID =
                        Send_Read_Property_Request(Target_Device_Object_Instance,
                        Target_Object_Type, Target_Object_Instance,
                        Target_Object_Property, Target_Object_Index);

                    snprintf(lbuf, SIZE__DIAG_MESSAGE, "assigned return value of Send_Read_Property_Request(), Request_Invoke_ID now equals %d,", 
                      Request_Invoke_ID);
                    show_diag(rname, lbuf, dflag_verbose);
                    show_diag(rname, "we only make a \"read property request\" when Request_Invoke_ID equals zero,", dflag_verbose);
                }

                else if (tsm_invoke_id_free(Request_Invoke_ID))
                {
                    show_diag(rname, "2017-04-20 - variable 'found' set to 'true' but Request_Invoke_ID not zero,", dflag_verbose);
                    show_diag(rname, " so now calling Transaction State Machine \"invoke id free\" routine,", dflag_verbose);

                    show_diag(rname, "call to tsm_invoke_id_free(Request_Invoke_ID) returns true,", dflag_verbose);
                    snprintf(lbuf, SIZE__DIAG_MESSAGE, "and Request_Invoke_ID presently = %d,", Request_Invoke_ID);
                    show_diag(rname, lbuf, dflag_verbose);
                    show_diag(rname, "breaking out of communications loop . . .", dflag_verbose);
                    blank_line_out(rname, 1);
                    break;
                }

                else if (tsm_invoke_id_failed(Request_Invoke_ID))
                {
                    fprintf(stderr, "\rError: TSM Timeout!\r\n");

                    show_diag(rname, "call to tsm_invoke_id_failed(Request_Invoke_ID) returns true,", dflag_verbose);
                    snprintf(lbuf, SIZE__DIAG_MESSAGE, "and Request_Invoke_ID presently = %d,", Request_Invoke_ID);
                    show_diag(rname, lbuf, dflag_verbose);
                    show_diag(rname, "setting Error_Detected to 'true',", dflag_verbose);
                    show_diag(rname, "breaking out of communications loop . . .", dflag_verbose);

                    tsm_free_invoke_id(Request_Invoke_ID);
                    Error_Detected = true;

                    blank_line_out(rname, 1);

                    /* try again or abort? */
                    break;
                }
            }
            else
            {
                /* increment timer - exit if timed out */
                elapsed_seconds += (current_seconds - last_seconds);
                if (elapsed_seconds > timeout_seconds)
                {
                    printf("\rError: APDU Timeout!\r\n");
                    Error_Detected = true;
                    break;
                }
            }


        /* returns 0 bytes on timeout */
//        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

            if ( 0 )
            {
                show_diag(rname, "calling datalink_receive(), really dlmstp_receive() . . .", dflag_comms_loop);
                pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
            }
            else
            {
                show_diag(rname, "calling dlmstp_receive() directly . . .", dflag_comms_loop);
                pdu_len = dlmstp_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
            }


        /* process */
            show_diag(rname, "checking whether pdu_len not zero,", dflag_comms_loop);
            if (pdu_len)
            {
                show_diag(rname, "pdu_len not zero, calling Kargs' routine npdu_handler() . . .", dflag_comms_loop);
                npdu_handler(&src, &Rx_Buf[0], pdu_len);
            }


        /* keep track of time for next check */
            last_seconds = current_seconds;



// Ted's code . . .

            ++loop_cycles_completed;


        } // end WHILE-loop to realize BACnet communications

    } // end 2017-04-19 WHILE-loop to iterate over communcations loop n times


    show_diag(rname, "leaving main communcations loop test 3,", dflag_verbose);
    show_diag(rname, "returning to caller . . .", dflag_announce);

    return 0;

} // end routine comms_test_3()





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - SECTION - main line code
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main(int argc, char** argv)
{

    int array_size;

// 2017-04-12 - moved up near top of file, before function definitions:
// #define ONE_MILLION_MS (1000 * 1000)
// #define MILLI_SECONDS_200K (200 * 1000)
// #define MAX_LOOP_CYCLES_TO_EXECUTE (1000)  // as of 2017-04-06 morning was (30) - TMH

    time_t time_start;
    time_t time_present;
    time_t time_elapsed;
    bool time_out_not_reached = true;

    int loop_cycles_completed = 0;


// Variables copied from Kargs demo program named bacrp:
    bool found = false;

    unsigned max_apdu = 0;

    uint16_t pdu_len = 0;

    BACNET_ADDRESS src = { 0 };  /* address where message came from */

    unsigned timeout = 400;     /* milliseconds - 2017-04-10 MON, was one hundred (100) ms - TMH */


// diagnostics:

    char lbuf[SIZE__DIAG_MESSAGE];

    unsigned int dflag_announce   = DIAGNOSTICS_ON;
    unsigned int dflag_verbose    = DIAGNOSTICS_ON;
    unsigned int dflag_step       = DIAGNOSTICS_ON;
    unsigned int dflag_init_steps = DIAGNOSTICS_ON;
    unsigned int dflag_comms_loop = DIAGNOSTICS_ON;
    unsigned int dflag_target_address_summary = DIAGNOSTICS_ON;

    unsigned int dflag_mark = DIAGNOSTICS_ON;

    DIAG__SET_ROUTINE_NAME("bacnet-stub main()");



    show_diag(rname, "starting,", dflag_announce);
    memset(lbuf, 1, SIZE__DIAG_MESSAGE);
    lbuf[(SIZE__DIAG_MESSAGE - 1)] = 0;

    array_size = strlen(lbuf);
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "local buffer for messages found to be %d bytes long,", array_size);
    show_diag(rname, lbuf, dflag_verbose);


// 2017-04-14 - A few variables to test, while not in use, to avoid gcc warnings:

    if ( Receive_Packet.ready ) { }
//    if ( Receive_Packet_Flag.waiting) { }
//    if ( Receive_Packet_Mutex ) { }
//    if ( get_abstime ) { }

// . . . these are part of Kargs' library routine dlmstp_receive(),
// which Ted tested as a copied routine in this test program,
// but without success to read back and see the CWLP's
// response to a BACnet "read property" request - TMH


// 2017-04-06 THU -
#if defined(BACDL_MSTP)
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "2017-04-06 NOTE:  Kargs project network type label 'BACDL_MSTP' is defined,");
#else
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "2017-04-06 NOTE:  Kargs project network type label 'BACDL_MSTP' not defined!");
#endif
    show_diag(rname, lbuf, dflag_verbose);



//    show_diag(rname, "calling communications test routine 2 . . .", dflag_verbose);
//    comms_test_2(argc, argv);
    show_diag(rname, "calling communications test routine 3 . . .", dflag_verbose);
    comms_test_3(argc, argv);



if ( 0 )
{

    show_diag(rname, "defining a pointer variable to a structure of type mstp_port_struct_t . . .", dflag_verbose);

    volatile static struct mstp_port_struct_t* mstp_port;

    if ( mstp_port == NULL ) { }



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - STEP - reusing several lines of MSTP port initializing code from     <-- NOTE ON 2017-04-06:  THIS STEP LIKELY CAN BE REMOVED - TMH
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

/*
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "", );
    show_diag(rname, lbuf, dflag_verbose);
*/
    show_diag(rname, "-", dflag_verbose);
    show_diag(rname, "- STEP 1 - beginning initializations found in Kargs demos `bacrp` and `bacwi` . . .", dflag_verbose);
    show_diag(rname, "-", dflag_verbose);

    show_diag(rname, "- INITIALIZING STEP 1p1 -", dflag_init_steps);
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "calling routine to set device object instance number with number %u . . .", BACNET_MAX_INSTANCE);
    show_diag(rname, lbuf, dflag_verbose);
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);

    show_diag(rname, "- INITIALIZING STEP 1p2 -", dflag_init_steps);
    show_diag(rname, "calling address_init() with no arguments (takes none) . . .", dflag_verbose);
    address_init();

    show_diag(rname, "- INITIALIZING STEP 1p3 -", dflag_init_steps);
    show_diag(rname, "calling Init_Service_Handlers() with no arguments (takes none) . . .", dflag_verbose);
    Init_Service_Handlers();

    show_diag(rname, "- INITIALIZING STEP 1p4 -", dflag_init_steps);
    show_diag(rname, "calling dlenv_init() with no arguments (takes none) . . .", dflag_verbose);
    dlenv_init();

    show_diag(rname, "- INITIALIZING STEP 1p5 -", dflag_init_steps);
    show_diag(rname, "calling atexit() with argument 'datalink_cleanup', most likely defined as \"#define datalink_cleanup dlmstp_cleanup\" . . .", dflag_verbose);
    atexit(datalink_cleanup);



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - STEP -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if ( 0 )
    {
        show_diag(rname, "-", dflag_step);
        show_diag(rname, "- STEP 2 -", dflag_step);
        show_diag(rname, "- Initializations custom to BACnet test program:", dflag_step);

        show_diag(rname, "calling ~/ports/linux/rs485.c routine to set serial port . . .", dflag_verbose);
        RS485_Set_Interface("/dev/ttyUSB1");
    }
    else
    {
        show_diag(rname, "Note:  not explicitly setting serial / RS485 device port,", dflag_step);
    }


    show_diag(rname, "-", dflag_step);
    show_diag(rname, "- STEP 3 -", dflag_step);
    show_diag(rname, "- Loop and communications attempt:", dflag_step);


// Some BACnet communications loop set ups:

    time_out_not_reached = true;
    time_start = time(NULL);
    time_present = time(NULL);

    loop_cycles_completed = 0;


// 2017-04-05 -
    Target_Device_Object_Instance = 133005;

    snprintf(lbuf, SIZE__DIAG_MESSAGE, "- LOOP SET UP - requesting binding of BACnet address for/of target device object instance %u . . .", 
      Target_Device_Object_Instance);
    show_diag(rname, lbuf, dflag_verbose);
    found = address_bind_request(Target_Device_Object_Instance, &max_apdu, &Target_Address);



// 2017-04-06 - taken from Kargs' demo program named bacrp:
    if (!found)
    {
        show_diag(rname, "didn't find the requested device, calling routine Send_WhoIs() . . .",
          dflag_verbose);
        
        Send_WhoIs(Target_Device_Object_Instance, Target_Device_Object_Instance);
    }

    if (1)
    {
        show_diag(rname, "Summary of Target_Address contents:", dflag_target_address_summary);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "  mac_len:  %u", Target_Address.mac_len);
        show_diag(rname, lbuf, dflag_target_address_summary);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "      mac:  %03u.%03u.%03u.%03u.%03u.%03u.%03u",
          Target_Address.mac[0], Target_Address.mac[1], Target_Address.mac[2], Target_Address.mac[3],
          Target_Address.mac[4], Target_Address.mac[5], Target_Address.mac[6]);
        show_diag(rname, lbuf, dflag_target_address_summary);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "      net:  %u", Target_Address.net);
        show_diag(rname, lbuf, dflag_target_address_summary);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "      len:  %u", Target_Address.len);
        show_diag(rname, lbuf, dflag_target_address_summary);

        snprintf(lbuf, SIZE__DIAG_MESSAGE, "      adr:  %u %u %u %u  %u %u %u",
          Target_Address.adr[0], Target_Address.adr[1],
          Target_Address.adr[2], Target_Address.adr[3],
          Target_Address.adr[4], Target_Address.adr[5],
          Target_Address.adr[6] );
        show_diag(rname, lbuf, dflag_target_address_summary);
    }

    usleep(ONE_MILLION_MS);



    Target_Device_Object_Instance = 133005;
    Target_Object_Type = 8;
    Target_Object_Instance = 133005;
    Target_Object_Property = 76;

    blank_line_out(rname, 3);
    show_diag(rname, "entering communiations loop:", dflag_verbose);


    while (( time_out_not_reached ) && ( loop_cycles_completed < MAX_LOOP_CYCLES_TO_EXECUTE ))
    {

//        snprintf(lbuf, SIZE__DIAG_MESSAGE, "%s", "-  COMMUNICATIONS LOOP - NOT YET FULLY IMPLEMENTED!");
//        show_diag(rname, lbuf, dflag_comms_loop);

        blank_line_out(rname, 1);

        show_diag(rname, "*", dflag_comms_loop);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "-  COMMUNICATIONS LOOP - executing now %d times,", (loop_cycles_completed + 1));
        show_diag(rname, lbuf, dflag_comms_loop);

        if ( !found )
        {
//            snprintf(lbuf, SIZE__DIAG_MESSAGE, "- MARK 1 - requesting address binding, with parameters and values:");
//            show_diag(rname, lbuf, dflag_mark);
            show_diag(rname, "- MARK 1 - about to call address_bind_request() with parameters and values:", dflag_mark);
            show_diag(rname, "-", dflag_mark);

            snprintf(lbuf, SIZE__DIAG_MESSAGE, "static global variable Target_Device_Object_Instance = %u,", Target_Device_Object_Instance);
            show_diag(rname, lbuf, dflag_verbose);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "local variable max_apdu = %u,", max_apdu);
            show_diag(rname, lbuf, dflag_verbose);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "static global struct BACnet_Device_Address Target_Address with data members:");
            show_diag(rname, lbuf, dflag_verbose);

            snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Address.mac_len = %u", Target_Address.mac_len);
            show_diag(rname, lbuf, dflag_verbose);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Address.mac[%u]", MAX_MAC_LEN);
            show_diag(rname, lbuf, dflag_verbose);

            snprintf(lbuf, SIZE__DIAG_MESSAGE, "    Target_Address.mac = %03d %03d %03d %03d  %03d %03d %03d",
              Target_Address.mac[0],
              Target_Address.mac[1],
              Target_Address.mac[2],
              Target_Address.mac[3],
              Target_Address.mac[4],
              Target_Address.mac[5],
              Target_Address.mac[6]);
            show_diag(rname, lbuf, dflag_verbose);

            snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Address.net = %u", Target_Address.net);
            show_diag(rname, lbuf, dflag_verbose);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Address.len = %u", Target_Address.len);
            show_diag(rname, lbuf, dflag_verbose);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Address.adr[%u]", MAX_MAC_LEN);
            show_diag(rname, lbuf, dflag_verbose);

            snprintf(lbuf, SIZE__DIAG_MESSAGE, "    Target_Address.adr = %03d %03d %03d %03d  %03d %03d %03d",
              Target_Address.adr[0],
              Target_Address.adr[1],
              Target_Address.adr[2],
              Target_Address.adr[3],
              Target_Address.adr[4],
              Target_Address.adr[5],
              Target_Address.adr[6]);
            show_diag(rname, lbuf, dflag_verbose);

            show_diag(rname, "calling address_bind_request() . . .", dflag_verbose);

            found = address_bind_request(Target_Device_Object_Instance, &max_apdu, &Target_Address);

        }


        if ( found )
//        if (( found ) || ( (loop_cycles_completed % 20) == 0 ))
        {
//            snprintf(lbuf, SIZE__DIAG_MESSAGE, "-  COMMUNICATIONS LOOP - looks like object instance %u address already bound,",
//              Target_Device_Object_Instance);
//            show_diag(rname, lbuf, dflag_comms_loop);
            show_diag(rname, "Kargs BACnet routine address_bind_request() returns 'true'!", dflag_verbose);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  * * *
//  Following code copied from Kargs open source BACnet demo program
//  `bacwi`.  Ted placing here as development step toward a
//  configurable BACnet master device communicator to put on
//  local MS/TP network:
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            if ( Request_Invoke_ID == 0 )
            {
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "comm's loop variable Request_Invoke_ID = %d,", Request_Invoke_ID);
                show_diag(rname, lbuf, dflag_verbose);

                show_diag(rname, "- MARK 5 - about to call routine Send_Read_Property_Request() with parameters and values:", dflag_mark);

                snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Device_Object_Instance = %u", Target_Device_Object_Instance);
                show_diag(rname, lbuf, dflag_verbose);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Object_Type = %u", Target_Object_Type);
                show_diag(rname, lbuf, dflag_verbose);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Object_Instance = %u", Target_Object_Instance);
                show_diag(rname, lbuf, dflag_verbose);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Object_Property = %u", Target_Object_Property);
                show_diag(rname, lbuf, dflag_verbose);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "  Target_Object_Index = %u", Target_Object_Index);
                show_diag(rname, lbuf, dflag_verbose);

                show_diag(rname, "so calling Send_Read_Property_Request() . . .", dflag_verbose);
                Request_Invoke_ID =
                  Send_Read_Property_Request(Target_Device_Object_Instance,
                  Target_Object_Type, Target_Object_Instance,
                  Target_Object_Property, Target_Object_Index);

//                show_diag(rname, "- back from Send_Read_Property_Request(),", dflag_verbose);
                snprintf(lbuf, SIZE__DIAG_MESSAGE, "back from routine Send_Read_Property_Request() which returns value %d,",
                  Request_Invoke_ID);
                show_diag(rname, lbuf, dflag_verbose);

            }

            else if ( tsm_invoke_id_free(Request_Invoke_ID) )
            {
                show_diag(rname, "- MARK 6 - ", dflag_mark);

                show_diag(rname, "routine tsm_invoke_id_free(Request_Invoke_ID) returned true, breaking out of loop . . .",
                  dflag_verbose);
                show_diag(rname, "*", dflag_comms_loop);
                break;
            }

            else if (tsm_invoke_id_failed(Request_Invoke_ID))
            {
                show_diag(rname, "- MARK 7 - ", dflag_mark);

                show_diag(rname, "routine tsm_invoke_id_failed(Request_Invoke_ID) returned true, breaking out of loop . . .",
                  dflag_verbose);

                fprintf(stderr, "\rError: TSM Timeout!\r\n");
                tsm_free_invoke_id(Request_Invoke_ID);
                Error_Detected = true;

                /* try again or abort? */
                show_diag(rname, "*", dflag_comms_loop);
                break;
            }

            int result = tsm_invoke_id_free(Request_Invoke_ID);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "call to routine tsm_invoke_id_free() returns %d,", result);
            show_diag(rname, lbuf, dflag_verbose);

            result = tsm_invoke_id_failed(Request_Invoke_ID);
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "call to routine tsm_invoke_id_failed() returns %d,", result);
            show_diag(rname, lbuf, dflag_verbose);

        }



        /* returns 0 bytes on timeout */
//        show_diag(rname, "calling datalink_receive() . . .", dflag_verbose);
        show_diag(rname, "* * *", dflag_verbose);
        show_diag(rname, "calling datalink_receive() with:", dflag_verbose);
        show_diag(rname, "  address of 'src',", dflag_verbose);
        show_diag(rname, "  address of 'Rx_Buf[]',", dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "  MAX_MPDU holding %d,", MAX_MPDU);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "  timeout holding %d,", timeout);
        show_diag(rname, lbuf, dflag_verbose);
        show_diag(rname, "calling . . . . .", dflag_verbose);
        show_diag(rname, "* * *", dflag_verbose);

        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

        /* process */
        show_diag(rname, "checking whether Protocol Data Unit length greater than zero . . .", dflag_verbose);
        if (pdu_len)
        {
            show_diag(rname, "calling npdu_handler() . . .", dflag_verbose);
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }
        else
        {
            snprintf(lbuf, SIZE__DIAG_MESSAGE, "not calling routine npdu_handler() as pdu_len holds %d,", pdu_len);
            show_diag(rname, lbuf, dflag_verbose);
            show_diag(rname, "meaning zero bytes received by routine datalink_receive(),", dflag_verbose);
        }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  * * *
//  END CODE from Kargs demo `bacrp`
//  * * *
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -




        time_present = time(NULL);

//        usleep(ONE_MILLION_MS);
        usleep(100000);

        show_diag(rname, "*", dflag_comms_loop);

        ++loop_cycles_completed;

    } // end WHILE-loop to realize BACnet communications


    {
        time_elapsed = ( time_present - time_start );

        show_diag(rname, "communciations loop timing summary:", dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "  start time:  %lu", time_start);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "    end time:  %lu", time_present);
        show_diag(rname, lbuf, dflag_verbose);
        snprintf(lbuf, SIZE__DIAG_MESSAGE, "elapsed time:  %lu", time_elapsed);
        show_diag(rname, lbuf, dflag_verbose);
    }

} // end IF-block to enable/disable communications test 1 - TMH



    show_diag(rname, "done.", dflag_announce);
    blank_line_out(rname, 1);

    return 0;

} // end routine main()





// --- EOF ---