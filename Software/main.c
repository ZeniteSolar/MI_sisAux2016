/*
 * MI_sisAux2016.c
 *
 * Created: 08/7/2016 13:25:59
 *  Author: Natan Ogliari
 *	Equipe Zênite Solar 
 */
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "can.h"
//#include "avr/wdt.h"//watchdog



#define aciBomba1	PD3
#define aciBomba2	PD4
#define aciMPPT	    PD5
#define aux1	    PD6
#define aux2	    PD7

// -----------------------------------------------------------------------------
/** Set filters and masks.
 *
 * The filters are divided in two groups:
 *
 * Group 0: Filter 0 and 1 with corresponding mask 0.
 * Group 1: Filter 2, 3, 4 and 5 with corresponding mask 1.
 *
 * If a group mask is set to 0, the group will receive all messages.
 *
 * If you want to receive ONLY 11 bit identifiers, set your filters
 * and masks as follows:
 *
 *	uint8_t can_filter[] PROGMEM = {
 *		// Group 0
 *		MCP2515_FILTER(0),				// Filter 0
 *		MCP2515_FILTER(0),				// Filter 1
 *		
 *		// Group 1
 *		MCP2515_FILTER(0),				// Filter 2
 *		MCP2515_FILTER(0),				// Filter 3
 *		MCP2515_FILTER(0),				// Filter 4
 *		MCP2515_FILTER(0),				// Filter 5
 *		
 *		MCP2515_FILTER(0),				// Mask 0 (for group 0)
 *		MCP2515_FILTER(0),				// Mask 1 (for group 1)
 *	};
 *
 *
 * If you want to receive ONLY 29 bit identifiers, set your filters
 * and masks as follows:
 *
 * \code
 *	uint8_t can_filter[] PROGMEM = {
 *		// Group 0
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 0
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 1
 *		
 *		// Group 1
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 2
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 3
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 4
 *		MCP2515_FILTER_EXTENDED(0),		// Filter 5
 *		
 *		MCP2515_FILTER_EXTENDED(0),		// Mask 0 (for group 0)
 *		MCP2515_FILTER_EXTENDED(0),		// Mask 1 (for group 1)
 *	};
 * \endcode
 *
 * If you want to receive both 11 and 29 bit identifiers, set your filters
 * and masks as follows:
 */
const uint8_t can_filter[] PROGMEM = 
{
	// Group 0
	//MCP2515_FILTER(0),				// Filter 0
	//MCP2515_FILTER(0),				// Filter 1
	
	// Group 1
	MCP2515_FILTER(1),				// Mask 0 (for group 0
};
// You can receive 11 bit identifiers with either group 0 or 1.


// -----------------------------------------------------------------------------
// Main loop for receiving and sending messages.

int main(void)
{
	
	//wdt_enable(WDTO_2S); //habilita watchdog em 2s
	
	typedef union estados_t{
		struct {
			unsigned char estBomba1 : 1;
			unsigned char estBomba2 : 1;
			unsigned char estMPPT   : 1;
			unsigned char estAux1   : 1;
			unsigned char estAux2   : 1;
		};
		unsigned char todas;
	} estados_t;

	volatile estados_t estados;
	
	// Initialize MCP2515
	can_init(BITRATE_125_KBPS);//define velocidade da comunicação
	
	// Load filters and masks
	can_static_filter(can_filter);
	
	
	
	
	// Create a test messsage
	can_t msg;
	
	msg.id = 0x123;
	msg.flags.rtr = 0;
	//msg.flags.extended = 0;
	
	msg.length = 5;
	msg.data[0] = estados.estBomba1;
	msg.data[1] = estados.estBomba2;
	msg.data[2] = estados.estMPPT;
	msg.data[3] = estados.estAux1;
	msg.data[3] = estados.estAux2;
	
	// Send the message
	can_send_message(&msg);
	
	while (1)
	{
		//wdt_reset(); //diz que esta ok para watchdog
		
		// Check if a new messag was received
		if (can_check_message())
		{
			can_t msg;
			
			// Try to read the message
			if (can_get_message(&msg))
			{
				// If we received a message resend it with a different id
				
				
				// Send the new message
				can_send_message(&msg);
			}
		}
	//wdt_reset(); //diz que esta ok para watchdog	
	}
	return 0;
}
