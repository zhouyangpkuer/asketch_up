#ifndef SOLUTIONS_H
#define SOLUTIONS_H

//#include "hash.h"
#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <immintrin.h>
#ifdef UNIX
#include <x86intrin.h>
#else
#include "immintrin.h"
#endif
#define MAX_COLLISION 64

struct Key
{
	unsigned sip;
	unsigned dip;
	Key(unsigned s, unsigned d) : sip(s), dip(d) {}
	Key(uint64_t k)
	{
		sip = (k >> 32);
		dip = k;
	}
	operator uint64_t() const
	{
		return ((uint64_t)sip << 32) + dip;
	}
	Key() { sip=0;dip=0;}
	bool operator==(const Key & b) const
	{
		return sip == b.sip && dip == b.dip;
	}
	operator bool()
	{
		return sip || dip;
	}
};

#define mix(a,b,c)  \
{ \
    a -= b; a -= c; a ^= (c>>13); \
    b -= c; b -= a; b ^= (a<<8); \
    c -= a; c -= b; c ^= (b>>13); \
    a -= b; a -= c; a ^= (c>>12);  \
    b -= c; b -= a; b ^= (a<<16); \
    c -= a; c -= b; c ^= (b>>5); \
    a -= b; a -= c; a ^= (c>>3);  \
    b -= c; b -= a; b ^= (a<<10); \
    c -= a; c -= b; c ^= (b>>15); \
}  /* MACRO mix */

template<uint32_t length, uint32_t initval>
uint32_t bobhash (const Key & _k)  /* an arbitrary value */
{
    register const uint8_t  * k = (const uint8_t *)&_k;

    register uint32_t a,b,c,len;

    /* Set up the internal state */
    len = length;
    a = b = 0x9e3779b9; /*the golden ratio; an arbitrary value */
    c = initval;         /* another arbitrary value */

    /*------------------------------------ handle most of the key */
    while (len >= 12) {
        a += (k[0] +((uint32_t)k[1]<<8) +((uint32_t)k[2]<<16)
            +((uint32_t)k[3]<<24));
        b += (k[4] +((uint32_t)k[5]<<8) +((uint32_t)k[6]<<16)
            +((uint32_t)k[7]<<24));
        c += (k[8] +((uint32_t)k[9]<<8) +((uint32_t)k[10]<<16)
            +((uint32_t)k[11]<<24));
        mix(a,b,c);
        k += 12; len -= 12;
    }

    /*---------------------------- handle the last 11 bytes */
    c += length;
    switch (len) {    /* all the case statements fall through*/
    case 11: c+=((uint32_t)k[10]<<24);
    case 10: c+=((uint32_t)k[9]<<16);
    case 9 : c+=((uint32_t)k[8]<<8);
        /* the first byte of c is reserved for the length */
    case 8 : b+=((uint32_t)k[7]<<24);
    case 7 : b+=((uint32_t)k[6]<<16);
    case 6 : b+=((uint32_t)k[5]<<8);
    case 5 : b+=k[4];
    case 4 : a+=((uint32_t)k[3]<<24);
    case 3 : a+=((uint32_t)k[2]<<16);
    case 2 : a+=((uint32_t)k[1]<<8);
    case 1 : a+=k[0];
        /* case 0: nothing left to add */
    default:
        break;
    }
    mix(a,b,c);

    /*-------------------------------- report the result */
    return c;
} /* END bobhash */

template<uint32_t length, uint32_t initval>
unsigned simple_hash(const Key & src)
{
	unsigned int ret = 0;
	unsigned char * str = (unsigned char *)&src;

	for (int i = 0; i < length; i++) {
		ret ^= (initval * str[i]);
	}

	return ret;
}

unsigned(*hash_funcs[4])(const Key &) = { bobhash<8, 0x12345678>,  bobhash<8, 0xDEADBEEF>,  bobhash<8, 0x73628761>,  bobhash<8, 0xACE15412>};
//unsigned(*hash_funcs[4])(const Key &) = { simple_hash<8, 0x12345678>, simple_hash<8, 0xDEADBEEF>, simple_hash<8, 0x73628761>, simple_hash<8, 0xACE15412> };


Key get_key(const char * data);
Key get_key(const char * data)
{
	return Key(*(unsigned *)data, *(unsigned *)(data + 4));
}

struct KVPair
{
	Key k;
	unsigned int v;
	KVPair() {v=0;}
};

class Solution
{
public:
	unsigned collisions;
	unsigned memory_accesses;
	virtual int init(unsigned memory_bytes) = 0;
	virtual int build(char * data, unsigned item_count, unsigned item_len) = 0;
	virtual unsigned query(Key key) = 0;
	virtual ~Solution() {};
};

class SolutionLinearHash: public Solution
{
	unsigned counters;
	KVPair * table;
public:
	int init(unsigned memory_bytes)
	{
		counters = (memory_bytes / sizeof(KVPair)) | 0x1;
		table = new KVPair[counters + MAX_COLLISION];
        if (!table) {
            std::cout << "Bad allocate" << std::endl;
        }
		return 0;
	}
	int build(char * data, unsigned item_count, unsigned item_len)
	{
		for (uint32_t i = 0; i < item_count; ++i) {
			char * item = data + i * item_len;
			Key key = get_key(item + 8);
			unsigned pos = hash_funcs[0](key) % counters;
			memory_accesses += 1;
			for (int j = 0; j < MAX_COLLISION; ++j) {
				if (table[pos + j].k == key) {
					table[pos + j].v++;
					goto L_DONE;
				} else if (!table[pos + j].k) {
					table[pos + j].k = key;
					table[pos + j].v += 1;
					goto L_DONE;
				}
			}
			return i;
		L_DONE:
			continue;
		}

		return 0;
	}

	unsigned query(Key key)
	{
		unsigned pos = hash_funcs[0](key) % counters;
		for (int j = 0; j < MAX_COLLISION; ++j) {
			if (table[pos + j].k == key) {
				return table[pos + j].v;
			} else if (!table[pos + j].k) {
				return 0;
			}
		}
		return 0;
	}

	~SolutionLinearHash()
	{
		delete[] table;
	}
};

template<int k>
class SolutionHashCompute : public Solution
{
	unsigned counters;
	unsigned val = 0;
public:
	int init(unsigned memory_bytes)
	{
		counters = (memory_bytes / sizeof(KVPair)) | 0x1;
		return 0;
	}
	int build(char * data, unsigned item_count, unsigned item_len)
	{
		//		memset(table_with_counter, 0, sizeof(KVPairWithCounter) * counters);
		for (int i = 0; i < item_count; ++i) {
			char * item = data + i * item_len;
			Key key = get_key(item + 8);
			for (int j = 0; j < k; ++j) {
				unsigned hash_val = hash_funcs[j](key);
				unsigned pos = hash_val % counters;
				val += pos;
			}
		}

		return 0;
	}

	unsigned query(Key key)
	{
		return val;
	}
};

enum EvictPolicy
{
	global_round_robin,
	evict_clock,
	approximate_LRU,
	evict_min,
};

template<int w>
alignas(64) struct Buffer
{
    alignas(32) union Key {
        uint64_t k[4 * w];
        __m256i m[w];
    }key;
    union Val {
        uint32_t v[4 * w];
        uint16_t ts[8 * w];
        uint64_t i64[2 * w];
    } val;
} __attribute__ ((aligned(64)));

template<int d, int w, bool AVX, EvictPolicy evict_policy=global_round_robin, bool verbose=true>
class SolutionFilteredLinearHash : public Solution
{
    Buffer<w> * batch;
    unsigned counters;
    KVPair * table;
	int pointer = 0;
	int pointers[d];

	int insert(const Key & key, unsigned val)
	{
		memory_accesses += 1;
		unsigned pos = hash_funcs[0](key) % counters;
		for (int j = 0; j < MAX_COLLISION; ++j) {
			if (table[pos + j].k == key) {
				table[pos + j].v += val;
				return 0;
			} else if (!table[pos + j].k) {
				table[pos + j].k = key;
				table[pos + j].v += val;
				return 0;
			}
		}
		return 1;
	}

	int insert_into_buffer(const Key & key, uint32_t ts)
	{
        unsigned group = (key.sip ^ key.dip) % d;
//        unsigned group = hash_funcs[2](key) % d;
		int i;
		if (AVX) {
			__m256i * src = (__m256i*)(batch[group].key.k);
			__m256i mask = _mm256_set1_epi64x(key);
			unsigned pos = 0;
			for (int j = 0; j < w; ++j) {
				__m256i tmp = _mm256_cmpeq_epi64(src[j], mask);
				pos |= (_mm256_movemask_pd(*(__m256d*)&tmp)) << (j * 4);
			}

			if (pos) {
				pos = _tzcnt_u32(pos);
				if (evict_policy == global_round_robin) {
					batch[group].val.v[pos] += 1;
				} else if (evict_policy == approximate_LRU) {
					batch[group].val.ts[pos] += 1;
					batch[group].val.ts[pos + w * 4] = uint16_t(ts >> 5);
				}
				return 0;
			}
		} else {
			for (i = 0; i < 4 * w; ++i) {
				if (batch[group].key.k[i] == (uint64_t)key) {
					if (evict_policy == global_round_robin) {
						batch[group].val.v[i] += 1;
					} else if (evict_policy == evict_clock) {
						batch[group].val.v[i] += 1;
                        batch[group].val.v[i] |= 0x80000000;
                    } else if (evict_policy == EvictPolicy::approximate_LRU) {
						batch[group].val.ts[i] += 1;
                        batch[group].val.ts[i + w * 4] = uint16_t(ts >> 5);
                    }
                    return 0;
                }
			}
		}

		// not found, select one to evict
        int pt = get_evict_index(group);
		int ret = evict_from_buffer(group, pt);
		batch[group].key.k[pt] = key;
		if (evict_policy == evict_clock) {
			batch[group].val.v[pt] = 1 | 0x80000000;
		} else if (evict_policy == EvictPolicy::approximate_LRU) {
			batch[group].val.ts[pt] = 1;
			batch[group].val.ts[pt + w * 4] = uint16_t(ts >> 5);
		} else {
			batch[group].val.v[pt] = 1;
		}
		return ret;
	}

    int get_evict_index(int group)
    {
        if (evict_policy == global_round_robin) {
            int old = pointer;
            pointer = (pointer + 1) % (4 * w);
            return old;
        } else if (evict_policy == evict_min) {
            unsigned t_min = 1 << 30;
            int t_k = 0;
            for (int i = 0; i < w * 4; ++i) {
                if (batch[group].val.v[i] < t_min) {
                    t_min = batch[group].val.v[i];
                    t_k = i;
                }
            }
            return t_k;
		} else if (evict_policy == evict_clock) {
            if (AVX && w == 1) {
                __m128i ts = _mm_setr_epi16(batch[group].val.ts[4 * w], batch[group].val.ts[4 * w + 1], batch[group].val.ts[4 * w + 2], batch[group].val.ts[4 * w + 3], 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
                int pt = _mm_cvtsi128_si32 (_mm_minpos_epu16(ts)) & 3;
                return pt;
            } else if (AVX && w == 2) {
                __m128i ts = _mm_set_epi64x(batch[group].val.i64[4], batch[group].val.i64[3]);
                int pt = _mm_cvtsi128_si32 (_mm_minpos_epu16(ts)) & 7;
                return pt;
            } else {
                int pt = pointers[group];
                for (; ; pt = (pt + 1) % (4 * w)) {
                    uint64_t mask = 0x80000000;
                    if (mask & batch[group].val.v[pt]) {
                        batch[group].val.v[pt] &= ~mask;
                    } else {
                        break;
                    }
                }
                pointers[group] = (pt + 1) % (4 * w);
                return pt;
            }
		} else if (evict_policy == approximate_LRU) {
			uint16_t min_ts = 65535;
			uint16_t min_pos = 0;
			for (int i = 0; i < 4 * w; ++i) {
				if (batch[group].val.ts[i + w * 4] < min_ts) {
					min_ts = batch[group].val.ts[i + w * 4];
					min_pos = i;
				}
			}

			return min_pos;
		}
    }

	int evict_from_buffer(unsigned group, unsigned pos)
	{
		if (!batch[group].key.k[pos]) {
			return 0;
		}
		uint32_t ins_val = 0;
		if (evict_policy == evict_clock) {
			ins_val = batch[group].val.v[pos] & 0x7FFFFFFF;
		} else if (evict_policy == approximate_LRU) {
			ins_val = batch[group].val.ts[pos];
		}
		else {
			ins_val = batch[group].val.v[pos];
		}
		return insert(batch[group].key.k[pos], ins_val);
	}

public:
	int init(unsigned memory_bytes)
	{
		counters = (memory_bytes / sizeof(KVPair)) | 0x1;
		table = new KVPair[counters + MAX_COLLISION];
        batch = (Buffer<w> *)aligned_alloc(64, sizeof(Buffer<w>) * d);
        memset(batch, 0, sizeof(Buffer<w>) * d);
		return 0;
	} 

	int build(char * data, unsigned item_count, unsigned item_len)
	{
		for (uint32_t i = 0; i < item_count; ++i) {
//            if (verbose) std::cout << "Insert element: " << i << std::endl;
			char * item = data + i * item_len;
			Key key = get_key(item + 8);
			if (insert_into_buffer(key, i)) {
				return i;
			}
		}

		int ret = 0;
		for (uint32_t i = 0; i < d; ++i) {
			for (uint32_t j = 0; j < 4 * w; ++j) {
				ret += evict_from_buffer(i, j);
			}
		}

		return ret;
	}

	unsigned query(Key key)
	{
		unsigned pos = hash_funcs[0](key) % counters;
		for (int j = 0; j < MAX_COLLISION; ++j) {
			if (table[pos + j].k == key) {
				return table[pos + j].v;
			} else if (!table[pos + j].k) {
				return 0;
			}
		}
		return 0;
	}

	~SolutionFilteredLinearHash()
	{
		delete[] table;
	}
};


#endif
