
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "aes.hpp"

static const uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
static const uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
static const uint8_t testData[32]{ 0xCC };

__declspec(noinline) void CW_AesDecrypt(uint8_t* key, uint8_t* initial_iv, uint8_t* data, uint32_t size)
{
	AES_ctx ctx;
	AES_init_ctx_iv(&ctx, key, initial_iv);
	AES_CBC_decrypt_buffer(&ctx, data, size);
	//Aes(&ctx);
}

__declspec(noinline) void CW_TestSTUB()
{
	CW_AesDecrypt((uint8_t*)&key, (uint8_t*)&iv, (uint8_t*)&testData, 32);
}

int main(int, char**)
{
	CW_TestSTUB();
}