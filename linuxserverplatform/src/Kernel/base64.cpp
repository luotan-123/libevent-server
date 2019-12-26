/* 
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/
#include "base64.h"
#include <iostream>
#include <cstring>

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string base64_decode(std::string const& encoded_string) {
  size_t in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}



char* StrSHA256(const char* str, long long length, char* sha256)
{

	char *pp, *ppend;
	long l, i, W[64], T1, T2, A, B, C, D, E, F, G, H, H0, H1, H2, H3, H4, H5, H6, H7;
	H0 = 0x6a09e667, H1 = 0xbb67ae85, H2 = 0x3c6ef372, H3 = 0xa54ff53a;
	H4 = 0x510e527f, H5 = 0x9b05688c, H6 = 0x1f83d9ab, H7 = 0x5be0cd19;
	long K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
	};
	l = length + ((length % 64 >= 56) ? (128 - length % 64) : (64 - length % 64));
	if (!(pp = (char*)malloc((unsigned long)l))) return 0;
	for (i = 0; i < length; pp[i + 3 - 2 * (i % 4)] = str[i], i++);
	for (pp[i + 3 - 2 * (i % 4)] = 128, i++; i < l; pp[i + 3 - 2 * (i % 4)] = 0, i++);
	*((long*)(pp + l - 4)) = length << 3;
	*((long*)(pp + l - 8)) = length >> 29;
	for (ppend = pp + l; pp < ppend; pp += 64){
		for (i = 0; i < 16; W[i] = ((long*)pp)[i], i++);
		for (i = 16; i < 64; W[i] = (SHA256_O1(W[i - 2]) + W[i - 7] + SHA256_O0(W[i - 15]) + W[i - 16]), i++);
		A = H0, B = H1, C = H2, D = H3, E = H4, F = H5, G = H6, H = H7;
		for (i = 0; i < 64; i++){
			T1 = H + SHA256_E1(E) + SHA256_Ch(E, F, G) + K[i] + W[i];
			T2 = SHA256_E0(A) + SHA256_Maj(A, B, C);
			H = G, G = F, F = E, E = D + T1, D = C, C = B, B = A, A = T1 + T2;
		}
		H0 += A, H1 += B, H2 += C, H3 += D, H4 += E, H5 += F, H6 += G, H7 += H;
	}
	free(pp - l);
	//sprintf(sha256, "%08X%08X%08X%08X%08X%08X%08X%08X", H0, H1, H2, H3, H4, H5, H6, H7);
	snprintf(sha256, 256, "%08X%08X%08X%08X%08X%08X%08X%08X", H0, H1, H2, H3, H4, H5, H6, H7);
	return sha256;

}



static uint32_t k[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#if ZDBG 
static int zdump_hex(const uint8_t *data, int size)
{
	int i;
	int l = 32;

	if (data[0] == 0x21) return 0;

	for (i = 0; i<size; i++) {
		if ((i%l) == 0) {
			printf("[%02x] ", i / l);
		}
		printf("%02x", data[i]);
		if (((i + 1) % l) == 0) {
			printf("\n");
		}
	}

	printf("\n");
	return 0;
}
#else
#define zdump_hex(a, b) 
#endif

static int ztransform(const uint8_t *msg, uint32_t *h)
{
	uint32_t w[64];
	uint32_t a0, b1, c2, d3, e4, f5, g6, h7;
	uint32_t t1, t2;

	int i = 0;
	int j = 0;

	for (i = 0; i<16; i++) {
		w[i] = msg[j] << 24 | msg[j + 1] << 16 | msg[j + 2] << 8 | msg[j + 3];
		j += 4;
	}

	for (i = 16; i<64; i++){
		w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
	}

	zdump_hex((uint8_t*)w, 64 * 4);

	a0 = h[0];
	b1 = h[1];
	c2 = h[2];
	d3 = h[3];
	e4 = h[4];
	f5 = h[5];
	g6 = h[6];
	h7 = h[7];

	for (i = 0; i<64; i++) {
		t1 = h7 + BSIG1(e4) + CHX(e4, f5, g6) + k[i] + w[i];
		t2 = BSIG0(a0) + MAJ(a0, b1, c2);

		h7 = g6;
		g6 = f5;
		f5 = e4;
		e4 = d3 + t1;
		d3 = c2;
		c2 = b1;
		b1 = a0;
		a0 = t1 + t2;
	}

	h[0] += a0;
	h[1] += b1;
	h[2] += c2;
	h[3] += d3;
	h[4] += e4;
	h[5] += f5;
	h[6] += g6;
	h[7] += h7;

	return 0;
}

int zsha256(const uint8_t *src, uint32_t len, uint32_t *hash)
{
	uint8_t *tmp = (uint8_t*)src;
	uint8_t  cover_data[SHA256_COVER_SIZE];
	uint32_t cover_size = 0;

	uint32_t i = 0;
	uint32_t n = 0;
	uint32_t m = 0;
	uint32_t h[8];

	h[0] = 0x6a09e667;
	h[1] = 0xbb67ae85;
	h[2] = 0x3c6ef372;
	h[3] = 0xa54ff53a;
	h[4] = 0x510e527f;
	h[5] = 0x9b05688c;
	h[6] = 0x1f83d9ab;
	h[7] = 0x5be0cd19;

	memset(cover_data, 0x00, sizeof(uint8_t)*SHA256_COVER_SIZE);

	n = len / SHA256_BLOCK_SIZE;
	m = len % SHA256_BLOCK_SIZE;

	if (m < 56) {
		cover_size = SHA256_BLOCK_SIZE;
	}
	else {
		cover_size = SHA256_BLOCK_SIZE * 2;
	}

	if (m != 0) {
		memcpy(cover_data, tmp + (n * SHA256_BLOCK_SIZE), m);
	}
	cover_data[m] = 0x80;
	cover_data[cover_size - 4] = ((len * 8) & 0xff000000) >> 24;
	cover_data[cover_size - 3] = ((len * 8) & 0x00ff0000) >> 16;
	cover_data[cover_size - 2] = ((len * 8) & 0x0000ff00) >> 8;
	cover_data[cover_size - 1] = ((len * 8) & 0x000000ff);

	zdump_hex(tmp, len - m);
	zdump_hex(cover_data, cover_size);

	for (i = 0; i<n; i++) {
		ztransform(tmp, h);
		tmp += SHA256_BLOCK_SIZE;
	}

	tmp = cover_data;
	n = cover_size / SHA256_BLOCK_SIZE;
	for (i = 0; i<n; i++) {
		ztransform(tmp, h);
		tmp += SHA256_BLOCK_SIZE;
	}

	if (NULL != hash) {
		memcpy(hash, h, sizeof(uint32_t) * 8);
	}
	return 0;
}


typedef struct {
	uint32_t state[5];
	uint32_t count[2];
	uint8_t  buffer[64];
} SHA1_CTX;



static void	SHA1_Transform(uint32_t	state[5], const	uint8_t	buffer[64]);

#define	rol(value, bits) (((value) << (bits)) |	((value) >>	(32	- (bits))))

/* blk0() and blk()	perform	the	initial	expand.	*/
/* I got the idea of expanding during the round	function from SSLeay */
/* FIXME: can we do	this in	an endian-proof	way? */
#ifdef WORDS_BIGENDIAN
#define	blk0(i)	block.l[i]
#else
#define	blk0(i)	(block.l[i]	= (rol(block.l[i],24)&0xFF00FF00) \
	|(rol(block.l[i],8)&0x00FF00FF))
#endif
#define	blk(i) (block.l[i&15] =	rol(block.l[(i+13)&15]^block.l[(i+8)&15] \
	^block.l[(i+2)&15]^block.l[i&15],1))

/* (R0+R1),	R2,	R3,	R4 are the different operations	used in	SHA1 */
#define	R0(v,w,x,y,z,i)	z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define	R1(v,w,x,y,z,i)	z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define	R2(v,w,x,y,z,i)	z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define	R3(v,w,x,y,z,i)	z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define	R4(v,w,x,y,z,i)	z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


/* Hash	a single 512-bit block.	This is	the	core of	the	algorithm. */
static void	SHA1_Transform(uint32_t	state[5], const	uint8_t	buffer[64])
{
	uint32_t a, b, c, d, e;
	typedef	union {
		uint8_t	c[64];
		uint32_t l[16];
	} CHAR64LONG16;
	CHAR64LONG16 block;

	memcpy(&block, buffer, 64);

	/* Copy	context->state[] to	working	vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];

	/* 4 rounds	of 20 operations each. Loop	unrolled. */
	R0(a, b, c, d, e, 0); R0(e, a, b, c, d, 1);	R0(d, e, a, b, c, 2); R0(c, d, e, a, b, 3);
	R0(b, c, d, e, a, 4); R0(a, b, c, d, e, 5);	R0(e, a, b, c, d, 6); R0(d, e, a, b, c, 7);
	R0(c, d, e, a, b, 8); R0(b, c, d, e, a, 9);	R0(a, b, c, d, e, 10); R0(e, a, b, c, d, 11);
	R0(d, e, a, b, c, 12); R0(c, d, e, a, b, 13);	R0(b, c, d, e, a, 14); R0(a, b, c, d, e, 15);
	R1(e, a, b, c, d, 16); R1(d, e, a, b, c, 17);	R1(c, d, e, a, b, 18); R1(b, c, d, e, a, 19);
	R2(a, b, c, d, e, 20); R2(e, a, b, c, d, 21);	R2(d, e, a, b, c, 22); R2(c, d, e, a, b, 23);
	R2(b, c, d, e, a, 24); R2(a, b, c, d, e, 25);	R2(e, a, b, c, d, 26); R2(d, e, a, b, c, 27);
	R2(c, d, e, a, b, 28); R2(b, c, d, e, a, 29);	R2(a, b, c, d, e, 30); R2(e, a, b, c, d, 31);
	R2(d, e, a, b, c, 32); R2(c, d, e, a, b, 33);	R2(b, c, d, e, a, 34); R2(a, b, c, d, e, 35);
	R2(e, a, b, c, d, 36); R2(d, e, a, b, c, 37);	R2(c, d, e, a, b, 38); R2(b, c, d, e, a, 39);
	R3(a, b, c, d, e, 40); R3(e, a, b, c, d, 41);	R3(d, e, a, b, c, 42); R3(c, d, e, a, b, 43);
	R3(b, c, d, e, a, 44); R3(a, b, c, d, e, 45);	R3(e, a, b, c, d, 46); R3(d, e, a, b, c, 47);
	R3(c, d, e, a, b, 48); R3(b, c, d, e, a, 49);	R3(a, b, c, d, e, 50); R3(e, a, b, c, d, 51);
	R3(d, e, a, b, c, 52); R3(c, d, e, a, b, 53);	R3(b, c, d, e, a, 54); R3(a, b, c, d, e, 55);
	R3(e, a, b, c, d, 56); R3(d, e, a, b, c, 57);	R3(c, d, e, a, b, 58); R3(b, c, d, e, a, 59);
	R4(a, b, c, d, e, 60); R4(e, a, b, c, d, 61);	R4(d, e, a, b, c, 62); R4(c, d, e, a, b, 63);
	R4(b, c, d, e, a, 64); R4(a, b, c, d, e, 65);	R4(e, a, b, c, d, 66); R4(d, e, a, b, c, 67);
	R4(c, d, e, a, b, 68); R4(b, c, d, e, a, 69);	R4(a, b, c, d, e, 70); R4(e, a, b, c, d, 71);
	R4(d, e, a, b, c, 72); R4(c, d, e, a, b, 73);	R4(b, c, d, e, a, 74); R4(a, b, c, d, e, 75);
	R4(e, a, b, c, d, 76); R4(d, e, a, b, c, 77);	R4(c, d, e, a, b, 78); R4(b, c, d, e, a, 79);

	/* Add the working vars	back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;

	/* Wipe	variables */
	a = b = c = d = e = 0;
}


/* SHA1Init	- Initialize new context */
static void sat_SHA1_Init(SHA1_CTX* context)
{
	/* SHA1	initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->count[0] = context->count[1] = 0;
}


/* Run your	data through this. */
static void sat_SHA1_Update(SHA1_CTX* context, const uint8_t* data, const size_t len)
{
	size_t i, j;

#ifdef VERBOSE
	SHAPrintContext(context, "before");
#endif

	j = (context->count[0] >> 3) & 63;
	if ((context->count[0] += len << 3)	< (len << 3)) context->count[1]++;
	context->count[1] += (len >> 29);
	if ((j + len) >	63)	{
		memcpy(&context->buffer[j], data, (i = 64 - j));
		SHA1_Transform(context->state, context->buffer);
		for (; i + 63 < len; i += 64) {
			SHA1_Transform(context->state, data + i);
		}
		j = 0;
	}
	else i = 0;
	memcpy(&context->buffer[j], &data[i], len - i);

#ifdef VERBOSE
	SHAPrintContext(context, "after	");
#endif
}


/* Add padding and return the message digest. */
static void sat_SHA1_Final(SHA1_CTX* context, uint8_t digest[SHA1_DIGEST_SIZE])
{
	uint32_t i;
	uint8_t	 finalcount[8];

	for (i = 0; i < 8; i++)	{
		finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
			>> ((3 - (i & 3)) * 8)) & 255);	 /*	Endian independent */
	}
	sat_SHA1_Update(context, (uint8_t *)"\200", 1);
	while ((context->count[0] & 504) != 448) {
		sat_SHA1_Update(context, (uint8_t *)"\0", 1);
	}
	sat_SHA1_Update(context, finalcount, 8);  /* Should	cause a	SHA1_Transform() */
	for (i = 0; i < SHA1_DIGEST_SIZE; i++) {
		digest[i] = (uint8_t)
			((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
	}

	/* Wipe	variables */
	i = 0;
	memset(context->buffer, 0, 64);
	memset(context->state, 0, 20);
	memset(context->count, 0, 8);
	memset(finalcount, 0, 8);	/* SWR */
}


void
crypt_sha1(uint8_t* buffer, int sz, uint8_t* digest, int* e_sz) {
	SHA1_CTX ctx;
	sat_SHA1_Init(&ctx);
	sat_SHA1_Update(&ctx, buffer, sz);
	sat_SHA1_Final(&ctx, digest);

	*e_sz = SHA1_DIGEST_SIZE;
}


