#ifndef _ASKETCH_H
#define _ASKETCH_H

#include "params.h"
#include "BOBHash32.h"
#include <string.h>
#include <x86intrin.h>
#include <bmiintrin.h>
using namespace std;

class ASketch
{
private:
	int w, d;
	int filter_size;

	int *new_count;
	int *old_count;
	uint *items;

	int *counter[MAX_HASH_NUM];

	BOBHash32 *bobhash[MAX_HASH_NUM];

	int cur_pos, bucket_num;
public:
	ASketch(double memory, int _d, int _filter_size)
	{
		w = (memory * 1024 * 1024 - _filter_size * 12) * 8.0 / 32.0 / _d;
		d = _d;
		filter_size = _filter_size;
		bucket_num = filter_size / 16;

		for(int i = 0; i < d; i++)	
		{
			counter[i] = new int[w];
			memset(counter[i], 0, sizeof(int) * w);
		}

		items = new uint[filter_size];
		memset(items, 0xFF, sizeof(uint) * filter_size);


		new_count = new int[filter_size];
		old_count = new int[filter_size];
		memset(new_count, 0, sizeof(int) * filter_size);
		memset(old_count, 0, sizeof(int) * filter_size);

		for(int i = 0; i < d; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}

		cur_pos = 0;
	}
	uint * get_items()
	{
		return items;
	}
	int * get_freq()
	{
		return new_count;
	}
	void Insert(uint key)
	{	
		const __m128i item = _mm_set1_epi32((int)key);

		for(int i = 0; i < bucket_num; i++)
		{
			__m128i *keys_p = (__m128i *)(items + (i << 4));
		
			__m128i a_comp = _mm_cmpeq_epi32(item, keys_p[0]);
			__m128i b_comp = _mm_cmpeq_epi32(item, keys_p[1]);
			__m128i c_comp = _mm_cmpeq_epi32(item, keys_p[2]);
			__m128i d_comp = _mm_cmpeq_epi32(item, keys_p[3]);
		
			a_comp = _mm_packs_epi32(a_comp, b_comp);
			c_comp = _mm_packs_epi32(c_comp, d_comp);
			a_comp = _mm_packs_epi32(a_comp, c_comp);
		
			int matched = _mm_movemask_epi8(a_comp);
		
			if(matched != 0)
			{
				//return 32 if input is zero;
				int matched_index = _tzcnt_u32((uint)matched) + (i << 4);
				new_count[matched_index] ++;
				return;
			}
		}

		if(cur_pos != filter_size)
		{
			items[cur_pos] = key;
			new_count[cur_pos] = 1;
			old_count[cur_pos] = 0;
			cur_pos ++;
			return;
		}


		int estimate_value, min_index, min_value, hash_value, temp;
		
		estimate_value = (1 << 30);
		for(int i = 0; i < d; i++)
		{
			hash_value = (bobhash[i]->run((const char *)&key, 4)) % w;
			counter[i][hash_value] ++;
			temp = counter[i][hash_value];

			estimate_value = estimate_value < temp ? estimate_value : temp;
		}

		min_index = 0;
		min_value = (1 << 30);
		for(int i = 0; i < filter_size; i++)
		{
			if(items[i] != (uint)(-1) && min_value > new_count[i])
			{
				min_value = new_count[i];
				min_index = i;
			}
		}
		if(estimate_value > min_value)
		{
			temp = new_count[min_index] - old_count[min_index];
			if(temp > 0)
			{
				for(int i = 0; i < d; i++)
				{
					hash_value = (bobhash[i]->run((const char *)&items[min_index], 4)) % w;
					counter[i][hash_value] += temp;
				}
			}
			items[min_index] = key;
			new_count[min_index] = estimate_value;
			old_count[min_index] = estimate_value;
		}
	}
	int Query(uint key)
	{
		const __m128i item = _mm_set1_epi32((int)key);
		for(int i = 0; i < bucket_num; i++)
		{
			__m128i *keys_p = (__m128i *)(items + (i << 4));
		
			__m128i a_comp = _mm_cmpeq_epi32(item, keys_p[0]);
			__m128i b_comp = _mm_cmpeq_epi32(item, keys_p[1]);
			__m128i c_comp = _mm_cmpeq_epi32(item, keys_p[2]);
			__m128i d_comp = _mm_cmpeq_epi32(item, keys_p[3]);
		
			a_comp = _mm_packs_epi32(a_comp, b_comp);
			c_comp = _mm_packs_epi32(c_comp, d_comp);
			a_comp = _mm_packs_epi32(a_comp, c_comp);
		
			int matched = _mm_movemask_epi8(a_comp);
		
			if(matched != 0)
			{
				//return 32 if input is zero;
				int matched_index = _tzcnt_u32((uint)matched) + (i << 4);
				return new_count[matched_index];
			}
		}


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
	~ASketch()
	{
		for(int i = 0; i < d; i++)	
		{
			delete []counter[i];
		}
		for(int i = 0; i < d; i++)
		{
			delete bobhash[i];
		}
		delete []items;
		delete []old_count;
		delete []new_count;
	}
};

#endif//_ASKETCH_H