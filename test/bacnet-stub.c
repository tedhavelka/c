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




//----------------------------------------------------------------------
// - SECTION - function prototypes
//----------------------------------------------------------------------

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

#define ONE_MILLION_MS (1000 * 1000)
#define MILLI_SECONDS_200K (200 * 1000)
#define MAX_LOOP_CYCLES_TO_EXECUTE (500)  // as of 2017-04-06 morning was (30) - TMH

    time_t time_start;
    time_t time_present;
    time_t time_elapsed;
    bool time_out_not_reached = true;

// Variables copied from Kargs demo program named bacrp:
    bool found = false;

    unsigned max_apdu = 0;

    uint16_t pdu_len = 0;

    BACNET_ADDRESS src = { 0 };  /* address where message came from */

    unsigned timeout = 100;     /* milliseconds */


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

// 2017-04-06 THU -
#if defined(BACDL_MSTP)
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "2017-04-06 NOTE:  Kargs project network type label 'BACDL_MSTP' is defined,");
#else
    snprintf(lbuf, SIZE__DIAG_MESSAGE, "2017-04-06 NOTE:  Kargs project network type label 'BACDL_MSTP' not defined!");
#endif
    show_diag(rname, lbuf, dflag_verbose);


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

    show_diag(rname, "-", dflag_step);
    show_diag(rname, "- STEP 2 -", dflag_step);
    show_diag(rname, "- Initializations custom to BACnet test program:", dflag_step);

    show_diag(rname, "calling ~/ports/linux/rs485.c routine to set serial port . . .", dflag_verbose);
    RS485_Set_Interface("/dev/ttyUSB0");



    show_diag(rname, "-", dflag_step);
    show_diag(rname, "- STEP 3 -", dflag_step);
    show_diag(rname, "- Loop and communications attempt:", dflag_step);


// Some BACnet communications loop set ups:

    time_out_not_reached = true;
    time_start = time(NULL);
    time_present = time(NULL);

    int loop_cycles_completed = 0;


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


//        if ( found )
        if (( found ) || ( (loop_cycles_completed % 20) == 0 ))
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

            if (Request_Invoke_ID == 0)
            {
                show_diag(rname, "- MARK 2 - about to call routine Send_Read_Property_Request() with parameters and values:", dflag_mark);

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

                show_diag(rname, "- calling Send_Read_Property_Request() . . .", dflag_verbose);
                Request_Invoke_ID =
                  Send_Read_Property_Request(Target_Device_Object_Instance,
                  Target_Object_Type, Target_Object_Instance,
                  Target_Object_Property, Target_Object_Index);

                show_diag(rname, "- back from Send_Read_Property_Request(),", dflag_verbose);
            }
            else if (tsm_invoke_id_free(Request_Invoke_ID))
            {
                show_diag(rname, "routine tsm_invoke_id_free(Request_Invoke_ID) returned true, breaking out of loop . . .",
                  dflag_verbose);
                show_diag(rname, "*", dflag_comms_loop);
                break;
            }
            else if (tsm_invoke_id_failed(Request_Invoke_ID))
            {
                show_diag(rname, "routine tsm_invoke_id_failed(Request_Invoke_ID) returned true, breaking out of loop . . .",
                  dflag_verbose);

                fprintf(stderr, "\rError: TSM Timeout!\r\n");
                tsm_free_invoke_id(Request_Invoke_ID);
                Error_Detected = true;

                /* try again or abort? */
                show_diag(rname, "*", dflag_comms_loop);
                break;
            }

        }



        /* returns 0 bytes on timeout */
        show_diag(rname, "calling datalink_receive() . . .", dflag_verbose);
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

        /* process */
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

        time_elapsed = ( time_present - time_start );

//        usleep(ONE_MILLION_MS);
        usleep(100000);

        show_diag(rname, "*", dflag_comms_loop);

        ++loop_cycles_completed;
    }






    show_diag(rname, "done.", dflag_announce);
    blank_line_out(rname, 1);

    return 0;

} // end routine main()





// --- EOF ---
