#include <string>

#include <iostream> 
#include <cstdio>
#include <cstdlib>
#include <stdint.h>

using namespace std;

#define SHA256_ROTL(a,b) (((a>>(32-b))&(0x7fffffff>>(31-b)))|(a<<b))
#define SHA256_SR(a,b) ((a>>b)&(0x7fffffff>>(b-1)))
#define SHA256_Ch(x,y,z) ((x&y)^((~x)&z))
#define SHA256_Maj(x,y,z) ((x&y)^(x&z)^(y&z))
#define SHA256_E0(x) (SHA256_ROTL(x,30)^SHA256_ROTL(x,19)^SHA256_ROTL(x,10))
#define SHA256_E1(x) (SHA256_ROTL(x,26)^SHA256_ROTL(x,21)^SHA256_ROTL(x,7))
#define SHA256_O0(x) (SHA256_ROTL(x,25)^SHA256_ROTL(x,14)^SHA256_SR(x,3))
#define SHA256_O1(x) (SHA256_ROTL(x,15)^SHA256_ROTL(x,13)^SHA256_SR(x,10))

typedef unsigned char uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int   uint32_t;

#define SHA1_DIGEST_SIZE 20

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);


char* StrSHA256(const char* str, long long length, char* sha256);

void crypt_sha1(uint8_t* buffer, int sz, uint8_t* output, int* e_sz);


#define ZDBG 0
#define SHFR(x, n) (((x) >> (n)))
#define ROTR(x, n) (((x) >> (n)) | ((x) << ((sizeof(x) << 3) - (n))))
#define ROTL(x, n) (((x) << (n)) | ((x) >> ((sizeof(x) << 3) - (n))))

#define CHX(x, y, z) (((x) &  (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) &  (y)) ^ ( (x) & (z)) ^ ((y) & (z)))

#define BSIG0(x) (ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define BSIG1(x) (ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SSIG0(x) (ROTR(x,  7) ^ ROTR(x, 18) ^ SHFR(x,  3))
#define SSIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))

#define SHA256_BLOCK_SIZE (512/8)
#define SHA256_COVER_SIZE (SHA256_BLOCK_SIZE*2)



#ifdef  __cplusplus  
extern "C" {
#endif 

	int zsha256(const uint8_t *src, uint32_t len, uint32_t *hash);

#ifdef  __cplusplus  
}
#endif 
