/* 
 * File:   main.c
 * Author: Chu Trong Khanh -DHBKHN
 *
 * Created on Ngay 02 thang 8 nam 2012, 12:47
 */

#include "p18f14k50.h"
#include "USB/usb.h"
#include "USB/usb_function_hid.h"
#include "HID Device Driver/usb_function_hid.c"
#include "USB/usb_device.h"
#include "usb_device.c"
#include "Console.h"
#include "LCD.h"

#pragma config CPUDIV = NOCLKDIV
#pragma config USBDIV = OFF
#pragma config FOSC   = HS
#pragma config PLLEN  = ON
#pragma config FCMEN  = OFF
#pragma config IESO   = OFF
#pragma config PWRTEN = OFF
#pragma config BOREN  = OFF
#pragma config BORV   = 30
#pragma config WDTEN  = OFF
#pragma config WDTPS  = 32768
#pragma config MCLRE  = ON
#pragma config HFOFST = OFF
#pragma config STVREN = ON
#pragma config LVP    = OFF
#pragma config XINST  = OFF
#pragma config BBSIZ  = OFF
#pragma config CP0    = OFF
#pragma config CP1    = OFF
#pragma config CPB    = OFF
#pragma config WRT0   = OFF
#pragma config WRT1   = OFF
#pragma config WRTB   = OFF
#pragma config WRTC   = OFF
#pragma config EBTR0  = OFF
#pragma config EBTR1  = OFF
#pragma config EBTRB  = OFF

unsigned char outbuffer[HID_OUT_SIZE];
unsigned char inbuffer[HID_IN_SIZE];
unsigned char error[] =" Error";
unsigned char success[] = " Success";
unsigned char init[] = "Init";

USB_HANDLE USBOutHandle = 0;
USB_HANDLE USBInHandle = 0;

void high_isr(void);
void low_isr(void); 

#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
    _asm GOTO low_isr _endasm
}
#pragma code

#pragma code high_vector=0x8
void interrupt_at_high_vector(void)
{
    _asm GOTO high_isr _endasm
}
#pragma code

#pragma interrupt high_isr
void high_isr()
{
    
#if defined(USB_INTERRUPT)
        USBDeviceTasks();
#endif
}

#pragma interruptlow low_isr
void low_isr()
{

}

static void InitializeSystem(void) {
    ADCON1 |= 0x0F;
//    TRISCbits.TRISC6=0;
//    TRISCbits.TRISC7 =0;
//    LATCbits.LATC6 =0;
//    LATCbits.LATC7=0;
    USBDeviceInit();
#if defined(USE_UART)
    Init_UART(BAUD_RATE);
    Init_Tran_UART();
    Init_Rec_UART();
#endif
#if defined(USE_LCD)
    Init_PORTS();
    Init_LCD();
#endif
}

BOOL red, green;

void main() {
    InitializeSystem();


    red = FALSE;
    green = FALSE;

    #if defined(USB_INTERRUPT)
        USBDeviceAttach();
    #endif

    while (1){
         #if defined(USB_POLLING)
		// Check bus status and service USB interrupts.
        USBDeviceTasks(); // Interrupt or polling method.  If using polling, must call
        				  // this function periodically.  This function will take care
        				  // of processing and responding to SETUP transactions
        				  // (such as during the enumeration process when you first
        				  // plug in).  USB hosts require that USB devices should accept
        				  // and process SETUP packets in a timely fashion.  Therefore,
        				  // when using polling, this function should be called
        				  // regularly (such as once every 1.8ms or faster** [see
        				  // inline code comments in usb_device.c for explanation when
        				  // "or faster" applies])  In most cases, the USBDeviceTasks()
        				  // function does not take very long to execute (ex: <100
        				  // instruction cycles) before it returns.
        #endif
        Tran_String_UART(init);
        lcd_gotoxy(1,2);
        lcd_putstr(init);
        if((USBGetDeviceState() \< CONFIGURED_STATE) ||(USBIsDeviceSuspended() == TRUE))
            {
            #if defined USE_UART
               Tran_String_UART(error);

            #endif
            #if defined USE_LCD
               lcd_clear();
               lcd_gotoxy(3,1);
               lcd_putstr(error);
            #endif
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else

            #if defined USE_UART
                Tran_String_UART(success);

            #endif
            #if defined USE_LCD
                lcd_clear();
                lcd_gotoxy(3,1);
                lcd_putstr(success);
            #endif
//            if(!HIDRxHandleBusy(USBOutHandle))
//            {
//                if (outbuffer[0] == 0) {
//                    if(!HIDTxHandleBusy(USBInHandle))
//                    {
//                        inbuffer[0] = green;
//                        inbuffer[1] = red;
//                        USBInHandle = HIDTxPacket(HID_EP,(BYTE*)&inbuffer[0], HID_IN_SIZE);
//                    }
//                }
//                if (outbuffer[0] == 1) {
//                    if (outbuffer[1] == 1) green = TRUE; else green = FALSE;
//                    if (outbuffer[2] == 1) red = TRUE; else red = FALSE;
//                    if (green) LATCbits.LATC6 = 1; else LATCbits.LATC6 = 0;
//                    if (red) LATCbits.LATC7 = 1; else LATCbits.LATC7 = 0;
//                }
//                if(outbuffer[0]==2){
//                    LATCbits.LATC6=0;
//                    LATCbits.L4ATC7=0;
//                }
//                USBOutHandle = HIDRxPacket(HID_EP, (BYTE*)&outbuffer, HID_OUT_SIZE);
//            } 
    }
}

BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size)
{
    switch(event)
    {
        case EVENT_CONFIGURED:
            USBEnableEndpoint(HID_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
            USBOutHandle = HIDRxPacket(HID_EP,(BYTE*)&outbuffer, HID_OUT_SIZE);
            break;
        case EVENT_EP0_REQUEST:
            USBCheckHIDRequest();
            break;
        default:
            break;
    }
    return TRUE;
}