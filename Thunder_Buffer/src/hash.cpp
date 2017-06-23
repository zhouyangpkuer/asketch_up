#include "hash.h"

unsigned SDBM_hash(unsigned src)
{
	unsigned int ret = 0;
	unsigned char * str = (unsigned char *)&src;

	for (int i = 0; i < 4; i++) {
		ret = str[i] + (ret << 6) + (ret << 16) - ret;
	}

	return ret;
}

unsigned simple_hash(unsigned src)
{
	unsigned int ret = 0;
	unsigned char * str = (unsigned char *)&src;

	for (int i = 0; i < 4; i++) {
		ret = ret * 33 + str[i];
	}

	return ret;
}

unsigned OCaml_hash(unsigned src)
{
	unsigned int ret = 0;
	unsigned char * str = (unsigned char *)&src;

	for (int i = 0; i < 4; i++) {
		ret = ret * 19 + str[i];
	}

	return ret;
}

unsigned JS_hash(unsigned src)
{
	unsigned int ret = 0;
	unsigned char * str = (unsigned char *)&src;

	for (int i = 0; i < 4; i++) {
		ret ^= ((ret << 5) + str[i] + (ret >> 2));
	}

	return ret;
}
