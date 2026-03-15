#include "emscripten.h"
#include <stdlib.h>
#include "../zlib/zlib.h"

// JS callback to copy a chunk of WASM memory
EM_JS(void, writeToJs, (z_stream *ptr, size_t size), {
  const data = new Uint8Array(Module.HEAPU8.buffer, ptr, size);

  const o = Module.map[ptr];

  if (o && typeof o.onData === 'function') {
    o.onData(data);
  } else {
    console.error("No handler found for pointer:", ptr);
  }
});

EMSCRIPTEN_KEEPALIVE
void *_malloc(size_t size) {
  return malloc(size);
}

EMSCRIPTEN_KEEPALIVE
void _free(void *p) {
  free(p);
}

EMSCRIPTEN_KEEPALIVE
z_stream *_createDeflateContext(int compression_level, int zlib_header) {
  z_stream *ptr = (z_stream*)malloc(sizeof(z_stream));
  ptr->zalloc = Z_NULL;
  ptr->zfree = Z_NULL;
  ptr->opaque = Z_NULL;
  if (deflateInit2(ptr, compression_level, Z_DEFLATED, MAX_WBITS * zlib_header, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) == Z_OK) {
    return ptr;
  } else {
    deflateEnd(ptr);
    return NULL;
  }
}

EMSCRIPTEN_KEEPALIVE
int _deflate(z_stream *ptr, unsigned char *src, unsigned char *dst, size_t avail_in, size_t avail_out, int flush) {
  ptr->avail_in = avail_in;
  ptr->next_in = src;
  size_t have;
  int ret;
  do {
      ptr->avail_out = avail_out;
      ptr->next_out = dst;
      ret = deflate(ptr, flush ? Z_FINISH : Z_NO_FLUSH);
      if (ret == Z_STREAM_ERROR) return ret;
      have = avail_out - ptr->avail_out;
      writeToJs(ptr, have);
  } while (ptr->avail_out == 0);
  return ret;
}

EMSCRIPTEN_KEEPALIVE
void _freeDeflateContext(z_stream *ptr) {
  deflateEnd(ptr);
}

EMSCRIPTEN_KEEPALIVE
z_stream *_createInflateContext(int zlib_header) {
  z_stream *ptr = malloc(sizeof(z_stream));
  ptr->zalloc = Z_NULL;
  ptr->zfree = Z_NULL;
  ptr->opaque = Z_NULL;
  ptr->avail_in = 0;
  ptr->next_in = Z_NULL;
  if (inflateInit2(ptr, MAX_WBITS * zlib_header) == Z_OK) {
    return ptr;
  } else {
    inflateEnd(ptr);
    return NULL;
  }
}

EMSCRIPTEN_KEEPALIVE
int _inflate(z_stream *ptr, unsigned char *src, unsigned char *dst, size_t avail_in, size_t avail_out) {
  ptr->avail_in = avail_in;
  ptr->next_in = src;
  int ret;
  size_t have;
  do {
    ptr->avail_out = avail_out;
    ptr->next_out = dst;
    ret = inflate(ptr, Z_NO_FLUSH);
    switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        return ret;
    }
    have = avail_out - ptr->avail_out;
    writeToJs(ptr, have);
  } while (ptr->avail_out == 0);
  return ret;
}

EMSCRIPTEN_KEEPALIVE
void _freeInflateContext(z_stream *ptr) {
  inflateEnd(ptr);
}
