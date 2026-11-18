#pragma once
#include "Arduino.h"
// Real SHA-256 via Windows CNG behind the mbedTLS API for host tests.
struct mbedtls_sha256_context {
  BCRYPT_ALG_HANDLE algorithm = nullptr;
  BCRYPT_HASH_HANDLE hash = nullptr;
};
inline void mbedtls_sha256_init(mbedtls_sha256_context *ctx) { *ctx = {}; }
inline int mbedtls_sha256_starts(mbedtls_sha256_context *ctx, int) {
  if (BCryptOpenAlgorithmProvider(&ctx->algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) != 0) return -1;
  return BCryptCreateHash(ctx->algorithm, &ctx->hash, nullptr, 0, nullptr, 0, 0);
}
inline int mbedtls_sha256_update(mbedtls_sha256_context *ctx, const uint8_t *data, size_t size) {
  return BCryptHashData(ctx->hash, const_cast<PUCHAR>(data), size, 0);
}
inline int mbedtls_sha256_finish(mbedtls_sha256_context *ctx, uint8_t *out) {
  return BCryptFinishHash(ctx->hash, out, 32, 0);
}
inline void mbedtls_sha256_free(mbedtls_sha256_context *ctx) {
  if (ctx->hash) BCryptDestroyHash(ctx->hash);
  if (ctx->algorithm) BCryptCloseAlgorithmProvider(ctx->algorithm, 0);
}
