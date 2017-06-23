#include <cstdint>
unsigned SDBM_hash(unsigned src);
unsigned simple_hash(unsigned src);
unsigned OCaml_hash(unsigned src);
unsigned JS_hash(unsigned src);

//template<uint32_t, uint32_t> unsigned bobhash(unsigned k);