#include <stdio.h>
#include <memory.h>

// 注意在vc上面，memcpy和memmove没有任何区别
// 只有在linux g++上才有区别
void mymemcpy(void* dst, void* src, int len)
{
	char* pdst = (char*)dst;
	const char* psrc = (const char*)src;
	for (int i = 0; i < len; i++)
	{
		pdst[i] = psrc[i];
	}
}

int main()
{
	char buf1[64] = "0123456789";
	char buf2[64] = "0123456789";
	char buf3[64] = "0123456789";

	//memcpy(buf1 + 3, buf1, 6);
	//memmove(buf2 + 3, buf2, 6);
	//mymemcpy(buf3 + 3, buf3, 6);

	memcpy(buf1 + 3, buf1 + 1, 7);
	memmove(buf2 + 3, buf2 + 1, 7);
	mymemcpy(buf3 + 3, buf3 + 1, 7);

	printf("buf1(memcpy)=%s\nbuf2(mmmove)=%s\nbuf3(mymcpy)=%s\n", buf1, buf2, buf3);

	return 0;
}
