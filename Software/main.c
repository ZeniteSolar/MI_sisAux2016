/**
* @file main.c
* @brief MI_sisAux2016.c
*
* @author Natan Ogliari
*
* @date 7/08/2016
*/

#define F_CPU 16000000UL
#include "globalDefines.h"
#include <avr/pgmspace.h>
#include "ATmega328.h"
//#include "avr/wdt.h"
#include <string.h>
#include "can.h"

#define aciBomba1	PD3			//! Definição de pino de saida
#define aciBomba2	PD4			//! Definição de pino de saida
#define aciMPPT	    PD5			//! Definição de pino de saida
#define aux1	    PD6			//! Definição de pino de saida
#define aux2	    PD7			//! Definição de pino de saida
#define INICIODOPACOTE		'@'
#define FINALDOPACOTE		'*'
#define POT_MODE			2 //!<APAGAR*****
#define SERIAL_MODE			1 //!<APAGAR*********
#define SETWORDSIZE 		8  // Number of bits of word to set parameter, used in the comunication with PC
#define GETWORDSIZE 		4  // Number of bits of word to get parameter, used in the comunication with PC


typedef union estados_t{	//!cria uma estrutura para ser enviado no pacote.
		struct {
			unsigned char estBomba1 : 1;
			unsigned char estBomba2 : 1;
			unsigned char estMPPT   : 1;
			unsigned char estAux1   : 1;
			unsigned char estAux2   : 1;
		};
		unsigned char todas;
	} estados_t;

volatile estados_t estados;		//<!Define estrutura como dado Volatile.

// Bit field flags
struct system_flags
{
	uint8 warning		: 1;
	uint8 erro 			: 1;
	uint8 mode			: 2;//modos de operação, definem quem controla o acionamento
	uint8 on			: 1;
	uint8 dms	    	: 1;
	uint8 				: 0;//completa a variavel
};
typedef struct system_flags flags_t;
flags_t flags;
//volatile struct system_flags flags_t;

const	uint8_t can_filter[] PROGMEM;		//!< Declara a função de filtro do canBus
void stringTransmit(char* texto);
void esvaziaBuffer();

/// @Brief Função principal.
/// here...
int main(void)
{
	_delay_ms(1000);
	// VARIAVEIS LOCAIS;
	char frameData[50];
	uint8 frameIndex = 0;
	char recebido[100] = "";
	char msgToSend[8] = "";
	uint8 pos =  0;
	flags.mode = POT_MODE;
	
	DDRD = (1 << aciBomba1) || (1 << aciBomba2) || (1 << aciMPPT) || (1 << aux1) || (1 << aux2);
 	//wdt_enable(WDTO_2S); //!<habilita watchdog em 2s
	estados.todas = 0;
	// Initialize MCP2515
	can_init(BITRATE_125_KBPS);		//!<Define a velocidade de comunicação.
	
	// Load filters and masks
	can_static_filter(can_filter);
		
	can_t msg;		//!Cria o pacote a ser enviado.
	
	msg.id = 0x000;
	//msg.flags.rtr = 0;
	
	msg.length = 5;			//!Define o tamanho do pacote a ser enviado.
	
	//se estiver no modo Serial configura a usart
	if (flags.mode == SERIAL_MODE)
	{
		// CONFIGURA A USART
		usartConfig(USART_MODE_ASYNCHRONOUS,USART_BAUD_9600 ,USART_DATA_BITS_8,USART_PARITY_NONE,USART_STOP_BIT_SINGLE);
		usartEnableReceiver();
		usartEnableTransmitter();
		usartActivateReceptionCompleteInterrupt();
	}
	
	while (1)
	{
		//wdt_reset(); //!<diz que esta ok para watchdog
		
		if ((1<<aciBomba1)&&PORTD)//!<Realiza a leitura Digital na porta PD3
		{
			estados.estBomba1 = !estados.estBomba1;
		}
			
		if ((1<<aciBomba2)&&PORTD)//!<Realiza a leitura Digital na porta PD4
		{
			estados.estBomba2 = !estados.estBomba2;
		}
		
		if ((1<<aciMPPT)&&PORTD)//!<Realiza a leitura Digital na porta PD5
		{
			estados.estMPPT = !estados.estMPPT;
		}
		
		if ((1<<aux1)&&PORTD)//!<Realiza a leitura Digital na porta PD6
		{
			estados.estAux1 = !estados.estAux1;
		}
		
		if ((1<<aux2)&&PORTD)//!Realiza a leitura Digital na porta PD7
		{
			estados.estAux2 = !estados.estAux2;
		}
		//!<Empacota os dados a serem enviados.
		msg.data[0] = estados.estBomba1;
		msg.data[1] = estados.estBomba2;
		msg.data[2] =   estados.estMPPT;
		msg.data[3] =   estados.estAux1;
		msg.data[4] =   estados.estAux2; 
		 
	    can_send_message(&msg);		//!Envia um o pacote
		 
		//!Check if a new messag was received
		if (can_check_message())
		{
			can_t msg;
			
			//!Try to read the message
			if (can_get_message(&msg))
			{
				
				//!Send the new message
				can_send_message(&msg);
			}
		}
		
		
		if(flags.mode == SERIAL_MODE)
		{
			while(!usartIsReceiverBufferEmpty())
			{
				frameData[frameIndex++] = usartGetDataFromReceiverBuffer();
				if ((frameData[frameIndex-1] == FINALDOPACOTE))
				{//se esta no final da palavra
					if(frameData[0] == INICIODOPACOTE )
					{//verifica se o inicio da palavra esta correto
						strcpy(recebido,frameData);
						pos = (recebido[2]-48) + (recebido[1] - 48)*10;
						if(frameIndex == GETWORDSIZE)
						{
							memcpy( recebido,  (recebido+1), 2);
							recebido[2] = '\0';//isola o id
							switch (pos)
							{
								case 0:
								strcpy(msgToSend,"OK");
								break;/*
								case 1:
								uint16ToString4(msgToSend,status.freq);
								break;
								case 2:
								uint8ToString4(msgToSend,maxCurrent);
								break;
								case 3:
								uint8ToString4(msgToSend,maxDC);
								break;
								case 4:
								uint8ToString4(msgToSend,minDC);
								break;
								case 5:
								uint8ToString4(msgToSend,maxDV);
								break;
								case 6 :
								if(flags.on)
								strcpy(msgToSend, "0001");
								else
								strcpy(msgToSend, "0000");
								break;
								case 7:
								if(flags.dms)
								strcpy(msgToSend, "0001");
								else
								strcpy(msgToSend, "0000");
								break;
								case 8:
								uint8ToString4(msgToSend,maxTemp);
								break;
								case 9:
								uint8ToString4(msgToSend,minVotage);
								break;
								case 10:
								uint8ToString4(msgToSend,status.dc);
								break;
								case 11:
								uint8ToString4(msgToSend,status.temperature);
								break;
								case 12:
								uint8ToString4(msgToSend,status.current);
								break;
								case 13:
								uint8ToString4(msgToSend,status.voltage);
								break;*/
								default:
								strcpy(msgToSend,"ERRO");
							}
							strcat(recebido,msgToSend);
							strcpy(msgToSend,recebido);

							stringTransmit(msgToSend);
						}
						else
						{
							if(frameIndex == SETWORDSIZE)
							{
								memcpy((void *) recebido, (void *) (recebido+3), 4);//isola somente o valor, usando 4 caracteres
								recebido[4] = '\0';

								switch (pos)
								{/*
									case 1:
									seta_freq(string4ToUint16(recebido));
									break;
									case 2:
									maxCurrent = string4Touint8(recebido);
									break;
									case 3:
									maxDC = string4Touint8(recebido);
									break;
									case 4:
									minDC = string4Touint8(recebido);
									estados.estAux1;
									break;
									case 5:
									maxDV = string4Touint8(recebido);
									break;
									case 6 :
									if(recebido[3] == '1')
									flags.on = 1;
									if(recebido[3] == '0')
									flags.on = 0;
									break;
									case 7:
									if(recebido[3] == '1')
									flags.dms = 1;
									if(recebido[3] == '0')
									flags.dms = 0;
									break;
									case 8:
									maxTemp = string4Touint8(recebido);
									break;
									case 9:
									minVotage = string4Touint8(recebido);
									break;
									case 10:
									//seta_dc(string4Touint8(recebido));
									dcReq = string4Touint8(recebido);
									break;
									case 11:
									case 12:
									case 13:*/

									default:
									stringTransmit("ERRO");
								}
							}
							else
							{
								stringTransmit("wrong size");
								esvaziaBuffer();
							}
						}
					}
					else
					{//se o inicio da palavra nao esta correto
						esvaziaBuffer();
					}
					frameIndex = 0;
				}
			}
		}
		//wdt_reset(); //!diz que esta ok para watchdog	
	}
	return 0;
}

/// @Brief Seta o filtro do canBus a ser usado
/// here...
/// @param O filtro a ser usado.
const	uint8_t can_filter[] PROGMEM = {
	// Group 0
	MCP2515_FILTER(0),				// Filter 0
	MCP2515_FILTER(1),				// Filter 1
	
	// Group 1
	MCP2515_FILTER(0),				// Filter 2
	MCP2515_FILTER(0),				// Filter 3
	MCP2515_FILTER(0),				// Filter 4
	MCP2515_FILTER(0),				// Filter 5
	
	MCP2515_FILTER(0),				// Mask 0 (for group 0)
	MCP2515_FILTER(0),				// Mask 1 (for group 1)
};


//envia uma msg usando o protocolo GUI
void stringTransmit(char* texto)
{
	uint8 i = 0;
	usartTransmit(INICIODOPACOTE);
	for(i = 0; texto[i] != '\0'; i++)
	usartTransmit(texto[i]);
	usartTransmit(FINALDOPACOTE);
}

//esvazia o buffer de entrada da usart
void esvaziaBuffer()
{
	while(!usartIsReceiverBufferEmpty())
	usartGetDataFromReceiverBuffer();
}
