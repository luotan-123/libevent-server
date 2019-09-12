#pragma once

//MD5 加密类
class KERNEL_CLASS MD5_CTX 
{
	//变量定义
private:
	unsigned int					state[4];
	unsigned int					count[2];
	unsigned char					buffer[64];  
	unsigned char					PADDING[64];

	//函数定义
public:
	//构造函数
	MD5_CTX();
	//析构函数
	virtual ~MD5_CTX();
	void MD5Update ( unsigned char *input, unsigned int inputLen);
	void MD5Final (unsigned char digest[16]);

	//私有函数
private:
	void MD5Init();
	void MD5Transform (unsigned int state[4], unsigned char block[64]);
	void MD5_memcpy (unsigned char* output, unsigned char* input,unsigned int len);
	void Encode (unsigned char *output, unsigned int *input,unsigned int len);
	void Decode (unsigned int *output, unsigned char *input, unsigned int len);
	void MD5_memset (unsigned char* output,int value,unsigned int len);
};