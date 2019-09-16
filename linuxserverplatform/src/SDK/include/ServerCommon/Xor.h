#pragma once

#include <stdint.h>

class Xor
{
public:
	static uint8_t* Encrypt(uint8_t* content, int length);

	static uint8_t* Decrypt(uint8_t* content, int length);
};
