#ifndef _LOCALHOST_H
#define _LOCALHOST_H
	#include "src/netstack.h"
	//host message

	const u_int8_t Mask[4] = { 255, 255, 255, 0 };
	const u_int8_t Gate[4] = { 10,13,80,1 };

	const u_int8_t Local_Mac[6] = { 0x00, 0x09, 0x73, 0x07, 0x73, 0xf9 };
	const u_int8_t Local_IP[4] = { 10,13,80,43 };
	const u_int8_t Target_Mac[6] = { 0x00, 0x09, 0x73, 0x07, 0x74, 0x73 };
	const u_int8_t Target_IP[4] = { 10,13,80,37 };

	const int host = CLIENT;

#endif