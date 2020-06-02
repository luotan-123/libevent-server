#pragma once

#include <stdint.h>

// 对称加密
class Xor
{
public:
	static uint8_t* Encrypt(uint8_t* content, int length);

	static uint8_t* Decrypt(uint8_t* content, int length);
};
