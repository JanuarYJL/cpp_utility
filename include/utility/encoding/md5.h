#include <cstddef>
#pragma once

/* Type define */
typedef unsigned char byte;
//typedef unsigned int/*long*/ ulong;	// ��g++64λ���뻷����longռ8���ֽڣ�������ʹ������������������
typedef unsigned int uint;

class CMD5
{
public:
	CMD5();
	CMD5(const void *input, size_t length);

public:
	void update(const void *input, size_t length);
	char *output();

private:
	void reset();
	const byte *digest();
	void update(const byte *input, size_t length);
	void final();
	void transform(const byte block[64]);
	void encode(const unsigned int *input, byte *output, size_t length);
	void decode(const byte *input, unsigned int *output, size_t length);

private:
	uint _state[4];   /* state (ABCD) */
	uint _count[2];   /* number of bits, modulo 2^64 (low-order word first) */
	byte _buffer[64]; /* input buffer */
	byte _digest[16]; /* message digest */
	bool _finished;   /* calculate finished ? */

	static const byte PADDING[64]; /* padding for calculate */
	static const char HEX[16];

	char _out[64];
};
