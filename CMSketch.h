#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include "params.h"
#include "BOBHash32.h"
#include <string.h>
using namespace std;

class CMSketch
{
private:
	int w, d;

	int *new_count;
	int *old_count;
	uint *items;

	int *counter[MAX_HASH_NUM];

	BOBHash32 *bobhash[MAX_HASH_NUM];

public:
	CMSketch(double memory, int _d)
	{
		w = (memory * 1024 * 1024) * 8.0 / 32.0 / _d;
		d = _d;

		for(int i = 0; i < d; i++)	
		{
			counter[i] = new int[w];
			memset(counter[i], 0, sizeof(int) * w);
		}

		for(int i = 0; i < d; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}
	}
	void Insert(uint key)
	{			
		int hash_value;
		for(int i = 0; i < d; i++)
		{
			hash_value = (bobhash[i]->run((const char *)&key, 4)) % w;
			counter[i][hash_value] ++;
		}
	}
	int Query(uint key)
	{
		int hash_value, temp;
		int estimate_value = (1 << 30);
		for(int i = 0; i < d; i++)
		{
			hash_value = (bobhash[i]->run((const char *)&key, 4)) % w;
			temp = counter[i][hash_value];
			estimate_value = estimate_value < temp ? estimate_value : temp;
		}
		return estimate_value;
	}
	~CMSketch()
	{
		for(int i = 0; i < d; i++)	
		{
			delete []counter[i];
		}
		for(int i = 0; i < d; i++)
		{
			delete bobhash[i];
		}
	}
};
#endif//_CMSKETCH_H