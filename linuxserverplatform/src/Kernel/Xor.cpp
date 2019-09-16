#include "CommonHead.h"
#include "Xor.h"

static unsigned char sKey[] = "linuxserverplatform2019";

uint8_t* Xor::Encrypt(uint8_t* content, int length)
{
	for (size_t i = 0; i < length; i++)
	{
		content[i] ^= sKey[i % (sizeof(sKey) - 1)];
	}
	return content;
}

uint8_t* Xor::Decrypt(uint8_t* content, int length)
{
	for (size_t i = 0; i < length; i++)
	{
		content[i] ^= sKey[i % (sizeof(sKey)-1)];
	}
	return content;
}
