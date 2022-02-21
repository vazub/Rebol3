/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2022 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Module:  n-crypt.c
**  Summary: native functions for data sets
**  Section: natives
**  Author:  Oldes, Cyphre
**  Notes:
**
***********************************************************************/

#include "sys-crypt.h"
#include "sys-rc4.h"
#ifdef INCLUDE_AES_DEPRECATED
#include "sys-aes.h"
#endif
#include "mbedtls/cipher_wrap.h"

#ifdef INCLUDE_CHACHA20POLY1305_DEPRECATED
#include "sys-chacha20.h"
#include "sys-poly1305.h"
#endif


static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;


/***********************************************************************
**
*/	void Init_Crypt(void)
/*
***********************************************************************/
{
	REBVAL *blk;
	REBVAL  tmp;
	REBCNT  sym;

	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);
	const char *pers = "rebol";

	mbedtls_ctr_drbg_seed(
		&ctr_drbg, mbedtls_entropy_func, &entropy,
		(const unsigned char *)pers, strlen(pers));
#ifdef INCLUDE_AES_DEPRECATED
	Register_Handle(SYM_AES,  sizeof(AES_CTX), NULL);
#endif
#ifdef INCLUDE_RC4
	Register_Handle(SYM_RC4,  sizeof(RC4_CTX), NULL);
#endif
	Register_Handle(SYM_DHM,  sizeof(DHM_CTX), (REB_HANDLE_FREE_FUNC)mbedtls_dhm_free);
#ifdef INCLUDE_RSA
	Register_Handle(SYM_RSA,  sizeof(RSA_CTX), (REB_HANDLE_FREE_FUNC)mbedtls_rsa_free);
#endif
	Register_Handle(SYM_ECDH, sizeof(ECDH_CTX), (REB_HANDLE_FREE_FUNC)mbedtls_ecdh_free);
	Register_Handle(SYM_CRYPT, sizeof(CRYPT_CTX), (REB_HANDLE_FREE_FUNC)crypt_context_free);
#ifdef INCLUDE_CHACHA20POLY1305_DEPRECATED
	Register_Handle(SYM_CHACHA20, sizeof(poly1305_context), NULL);
	Register_Handle(SYM_POLY1305, sizeof(poly1305_context), NULL);
	Register_Handle(SYM_CHACHA20POLY1305, sizeof(chacha20poly1305_ctx), NULL);
#endif

	#define add_ec_word(sym) Init_Word(&tmp, sym); VAL_SET_LINE(&tmp); Append_Val(VAL_SERIES(blk), &tmp);
	blk = Get_System(SYS_CATALOG, CAT_CIPHERS);
	if (blk && IS_BLOCK(blk)) {
		add_ec_word(SYM_AES_128_ECB)
		add_ec_word(SYM_AES_192_ECB)
		add_ec_word(SYM_AES_256_ECB)
		#ifdef MBEDTLS_CIPHER_MODE_CBC
			add_ec_word(SYM_AES_128_CBC)
			add_ec_word(SYM_AES_192_CBC)
			add_ec_word(SYM_AES_256_CBC)
		#endif
		#ifdef MBEDTLS_GCM_C
			add_ec_word(SYM_AES_128_GCM)
			add_ec_word(SYM_AES_192_GCM)
			add_ec_word(SYM_AES_256_GCM)
		#endif
		#ifdef MBEDTLS_CAMELLIA_C
			add_ec_word(SYM_CAMELLIA_128_ECB)
			add_ec_word(SYM_CAMELLIA_192_ECB)
			add_ec_word(SYM_CAMELLIA_256_ECB)
		#ifdef MBEDTLS_CIPHER_MODE_CBC
			add_ec_word(SYM_CAMELLIA_128_CBC)
			add_ec_word(SYM_CAMELLIA_192_CBC)
			add_ec_word(SYM_CAMELLIA_256_CBC)
		#endif
		#ifdef MBEDTLS_GCM_C
			add_ec_word(SYM_CAMELLIA_128_GCM)
			add_ec_word(SYM_CAMELLIA_192_GCM)
			add_ec_word(SYM_CAMELLIA_256_GCM)
		#endif
		#endif
		#ifdef MBEDTLS_ARIA_C
		add_ec_word(SYM_ARIA_128_ECB)
		add_ec_word(SYM_ARIA_192_ECB)
		add_ec_word(SYM_ARIA_256_ECB)
		#ifdef MBEDTLS_CIPHER_MODE_CBC
		add_ec_word(SYM_ARIA_128_CBC)
		add_ec_word(SYM_ARIA_192_CBC)
		add_ec_word(SYM_ARIA_256_CBC)
		#endif
		#ifdef MBEDTLS_GCM_C
		add_ec_word(SYM_ARIA_128_GCM)
		add_ec_word(SYM_ARIA_192_GCM)
		add_ec_word(SYM_ARIA_256_GCM)
		#endif
		#endif
		#ifdef MBEDTLS_CHACHA20_C
			add_ec_word(SYM_CHACHA20)
		#endif
		#ifdef MBEDTLS_CHACHAPOLY_C
			add_ec_word(SYM_CHACHA20_POLY1305)
		#endif
	}

	blk = Get_System(SYS_CATALOG, CAT_ELLIPTIC_CURVES);
	if (blk && IS_BLOCK(blk)) {
		
		#ifdef MBEDTLS_ECP_DP_SECP192R1_ENABLED
		add_ec_word(SYM_SECP192R1)
		#endif
		#ifdef MBEDTLS_ECP_DP_SECP224R1_ENABLED
		add_ec_word(SYM_SECP224R1)
		#endif
		#ifdef MBEDTLS_ECP_DP_SECP256R1_ENABLED
		add_ec_word(SYM_SECP256R1)
		#endif
		#ifdef MBEDTLS_ECP_DP_SECP384R1_ENABLED
		add_ec_word(SYM_SECP384R1)
		#endif
		#ifdef MBEDTLS_ECP_DP_SECP521R1_ENABLED
		add_ec_word(SYM_SECP521R1)
		#endif
		#ifdef MBEDTLS_ECP_DP_SECP192K1_ENABLED
		add_ec_word(SYM_SECP192K1)
		#endif
		#ifdef MBEDTLS_ECP_DP_SECP224K1_ENABLED
		add_ec_word(SYM_SECP224K1)
		#endif
		#ifdef MBEDTLS_ECP_DP_SECP256K1_ENABLED
		add_ec_word(SYM_SECP256K1)
		#endif
		#ifdef MBEDTLS_ECP_DP_BP256R1_ENABLED
		add_ec_word(SYM_BP256R1)
		#endif
		#ifdef MBEDTLS_ECP_DP_BP384R1_ENABLED
		add_ec_word(SYM_BP384R1)
		#endif
		#ifdef MBEDTLS_ECP_DP_BP512R1_ENABLED
		add_ec_word(SYM_BP512R1)
		#endif
		#ifdef MBEDTLS_ECP_DP_CURVE25519_ENABLED
		add_ec_word(SYM_CURVE25519)
		#endif
		#ifdef MBEDTLS_ECP_DP_CURVE448_ENABLED
		add_ec_word(SYM_CURVE448)
		#endif
	}
	
}

/***********************************************************************
**
*/	REBNATIVE(rc4)
/*
//	rc4: native [
//		"Encrypt/decrypt data (modifies) using RC4 algorithm."
//
//		/key "Provided only for the first time to get stream HANDLE!"
//			crypt-key [binary!]  "Crypt key."
//		/stream
//			ctx  [handle!] "Stream cipher context."
//			data [binary!] "Data to encrypt/decrypt."
//	]
***********************************************************************/
{
#ifndef INCLUDE_RC4
	Trap0(RE_FEATURE_NA);
#else
    REBOOL  ref_key       = D_REF(1);
    REBVAL *val_crypt_key = D_ARG(2); 
    REBOOL  ref_stream    = D_REF(3);
    REBVAL *val_ctx       = D_ARG(4);
    REBVAL *val_data      = D_ARG(5);

    REBVAL *ret = D_RET;

    if(ref_stream) {
    	if (NOT_VALID_CONTEXT_HANDLE(val_ctx, SYM_RC4)) Trap0(RE_INVALID_HANDLE);

    	REBYTE *data = VAL_BIN_AT(val_data);
    	RC4_crypt((RC4_CTX*)VAL_HANDLE_CONTEXT_DATA(val_ctx), data, data, VAL_LEN(val_data));
    	DS_RET_VALUE(val_data);

    } else if (ref_key) {
    	//key defined - setup new context
		MAKE_HANDLE(ret, SYM_RC4);
		RC4_setup(
			(RC4_CTX*)VAL_HANDLE_CONTEXT_DATA(ret),
            VAL_BIN_AT(val_crypt_key),
            VAL_LEN(val_crypt_key)
        );
    }
    return R_RET;
#endif //INCLUDE_RC4
}


/***********************************************************************
**
*/	REBNATIVE(rsa_init)
/*
//  rsa-init: native [
//		"Creates a context which is than used to encrypt or decrypt data using RSA"
//		n  [binary!]  "Modulus"
//		e  [binary!]  "Public exponent"
//		/private "Init also private values"
//			d [binary!] "Private exponent"
//			p [binary!] "Prime number 1"
//			q [binary!] "Prime number 2"
//  ]
***********************************************************************/
{
#ifndef INCLUDE_RSA
	Trap0(RE_FEATURE_NA);
#else
	REBVAL *n       = D_ARG(1);
	REBVAL *e       = D_ARG(2);
	REBOOL  ref_private =        D_REF(3);
	REBVAL *d       = D_ARG(4);
	REBVAL *p       = D_ARG(5);
	REBVAL *q       = D_ARG(6);

	int err = 0;
	REBVAL *ret = D_RET;
	RSA_CTX *rsa_ctx;

	MAKE_HANDLE(ret, SYM_RSA);
	rsa_ctx = (RSA_CTX*)VAL_HANDLE_CONTEXT_DATA(ret);

	mbedtls_rsa_init(rsa_ctx);

	if (ref_private) {
		err = mbedtls_rsa_import_raw(
			rsa_ctx,
			VAL_BIN_AT(n), VAL_LEN(n),
			VAL_BIN_AT(p), VAL_LEN(p),
			VAL_BIN_AT(q), VAL_LEN(q),
			VAL_BIN_AT(d), VAL_LEN(d),
			VAL_BIN_AT(e), VAL_LEN(e)
		);
		if (err != 0 
			|| mbedtls_rsa_complete(rsa_ctx) != 0
			|| mbedtls_rsa_check_privkey(rsa_ctx) != 0 
		) return R_NONE;
	} else {
		err = mbedtls_rsa_import_raw(
			rsa_ctx,
			VAL_BIN_AT(n), VAL_LEN(n),
			NULL, 0,
			NULL, 0,
			NULL, 0,
			VAL_BIN_AT(e), VAL_LEN(e)
		);
		if (err != 0 
			|| mbedtls_rsa_complete(rsa_ctx) != 0
			|| mbedtls_rsa_check_pubkey(rsa_ctx) != 0 
		) return R_NONE;
	}
	return R_RET;
#endif //INCLUDE_RSA
}
#ifdef unused 
static int myrand(void *rng_state, unsigned char *output, size_t len)
{
#if !defined(__OpenBSD__) && !defined(__NetBSD__)
	size_t i;

	if (rng_state != NULL)
		rng_state = NULL;

	for (i = 0; i < len; ++i)
		output[i] = rand();
#else
	if (rng_state != NULL)
		rng_state = NULL;

	arc4random_buf(output, len);
#endif /* !OpenBSD && !NetBSD */

	return(0);
}
#endif

/***********************************************************************
**
*/	REBNATIVE(rsa)
/*
//  rsa: native [
//		"Encrypt/decrypt/sign/verify data using RSA cryptosystem. Only one refinement must be used!"
//		rsa-key [handle!] "RSA context created using `rsa-init` function"
//		data    [binary!] "Data to work with."
//		/encrypt  "Use public key to encrypt data"
//		/decrypt  "Use private key to decrypt data"
//		/sign     "Use private key to sign data. Result is PKCS1 v1.5 binary"
//		/verify   "Use public key to verify signed data (returns TRUE or FALSE)"
//		 signature [binary!] "Result of the /sign call"
//		/hash     "Signature's message digest algorithm"
//		 algorithm [word! none!]
//  ]
***********************************************************************/
{
#ifndef INCLUDE_RSA
	Trap0(RE_FEATURE_NA);
#else
	REBVAL *key         = D_ARG(1);
	REBVAL *val_data    = D_ARG(2);
	REBOOL  refEncrypt  = D_REF(3);
	REBOOL  refDecrypt  = D_REF(4);
	REBOOL  refSign     = D_REF(5);
	REBOOL  refVerify   = D_REF(6);
	REBVAL *val_sign    = D_ARG(7);
	REBOOL  refHash     = D_REF(8);
	REBVAL *val_hash    = D_ARG(9);

	RSA_CTX *rsa;
	REBSER  *data;
	REBYTE  *inBinary;
	REBYTE  *outBinary;
	REBYTE   hash[64];
	REBCNT   hashSym;
	REBCNT   inBytes;
	REBCNT   outBytes;
	REBINT   err = 0;
	mbedtls_md_type_t md_alg;

	// make sure that only one refinement is used!
	if(
		(refEncrypt && (refDecrypt || refSign    || refVerify || refHash)) ||
		(refDecrypt && (refEncrypt || refSign    || refVerify || refHash)) ||
		(refSign    && (refDecrypt || refEncrypt || refVerify)) ||
		(refVerify  && (refDecrypt || refSign    || refEncrypt))
	) {
		Trap0(RE_BAD_REFINES);
	}

	if (NOT_VALID_CONTEXT_HANDLE(key, SYM_RSA)) Trap0(RE_INVALID_HANDLE);

	rsa = (RSA_CTX*)VAL_HANDLE_CONTEXT_DATA(key);

	if(
		(mbedtls_rsa_check_pubkey(rsa) != 0) ||
		((refDecrypt || refSign) && (mbedtls_rsa_check_privkey(rsa) != 0))
	) {
		return R_NONE;
	}

	data     = VAL_SERIES(val_data);
	inBinary = VAL_BIN_AT(val_data);
	inBytes  = VAL_LEN(val_data);

	if (refVerify || refSign) {
		if (refHash && IS_NONE(val_hash)) {
			// use none if really requested
			md_alg = MBEDTLS_MD_NONE;
		}
		else {
			// if /hash was not used, make default to SHA256
			hashSym = IS_NONE(val_hash) ? SYM_SHA256 : VAL_WORD_CANON(val_hash);
			// count message digest off the input data
			if (Message_Digest(hash, inBinary, inBytes, hashSym, &inBytes)) {
				// map Rebol word to mbedtls_md_type_t (expets that have same order!)
				// no need to test a range as only known will pass above run
				md_alg = hashSym - SYM_MD5 + 1;
				inBinary = hash;
			}
			else {
				return R_NONE;
			}
		}
		if (refVerify) {
			err = mbedtls_rsa_rsassa_pkcs1_v15_verify(rsa, md_alg, inBytes, inBinary, VAL_BIN_AT(val_sign));
			return (err == 0) ? R_TRUE : R_FALSE;
		}
	}

	//allocate new binary!
	outBytes = mbedtls_rsa_get_len(rsa);
	data = Make_Binary(outBytes-1);
	outBinary = BIN_DATA(data);

	if (refSign) {
		err = mbedtls_rsa_rsassa_pkcs1_v15_sign(rsa, mbedtls_ctr_drbg_random, &ctr_drbg, md_alg, inBytes, inBinary, outBinary);
	}
	else if (refEncrypt) {
		err = mbedtls_rsa_rsaes_pkcs1_v15_encrypt(rsa, mbedtls_ctr_drbg_random, &ctr_drbg, inBytes, inBinary, outBinary);
	}
	else {
		size_t olen = 0;
		err = mbedtls_rsa_rsaes_pkcs1_v15_decrypt(rsa, mbedtls_ctr_drbg_random, &ctr_drbg, &olen, inBinary, outBinary, outBytes);
		outBytes = olen;
	}
	if (err) goto error;

	SET_BINARY(D_RET, data);
	VAL_TAIL(D_RET) = outBytes;

	return R_RET;
error:
	//printf("RSA key init failed with error:: -0x%0x\n", (unsigned int)-err);
	Free_Series(data);
	return R_NONE;
#endif //INCLUDE_RSA
}


/***********************************************************************
**
*/	REBNATIVE(dh_init)
/*
//  dh-init: native [
//		"Generates a new Diffie-Hellman private/public key pair"
//		g [binary!] "Generator"
//		p [binary!] "Field prime"
//  ]
***********************************************************************/
{
	REBVAL *g        = D_ARG(1);
	REBVAL *p        = D_ARG(2);
	REBOOL  ref_into = D_REF(3);
	REBVAL *val_dh   = D_ARG(4);
	
	REBINT  err = 0;
	DHM_CTX *dhm;

	MAKE_HANDLE(D_RET, SYM_DHM);
	dhm = (DHM_CTX*)VAL_HANDLE_CONTEXT_DATA(D_RET);
	mbedtls_dhm_init(dhm);

	err = mbedtls_mpi_read_binary(&dhm->MBEDTLS_PRIVATE(P), VAL_BIN_AT(p), VAL_SERIES(p)->tail - VAL_INDEX(p) );
	if (err) goto error;
	err = mbedtls_mpi_read_binary( &dhm->MBEDTLS_PRIVATE(G), VAL_BIN_AT(g), VAL_SERIES(g)->tail - VAL_INDEX(g));
	if (err) goto error;

	size_t n = mbedtls_dhm_get_len(dhm);
	if (n < 64 || n > 512) goto error;

	/* Generate private key X as large as possible ( <= P - 2 ) */
	err = dhm_random_below(
		&dhm->MBEDTLS_PRIVATE(X),
		&dhm->MBEDTLS_PRIVATE(P),
		mbedtls_ctr_drbg_random, &ctr_drbg);
	if (err) goto error;

	/*
	 * Calculate public key (self) GX = G^X mod P
	 */
	err = mbedtls_mpi_exp_mod(
		&dhm->MBEDTLS_PRIVATE(GX),
		&dhm->MBEDTLS_PRIVATE(G),
		&dhm->MBEDTLS_PRIVATE(X),
		&dhm->MBEDTLS_PRIVATE(P),
		&dhm->MBEDTLS_PRIVATE(RP));
	if (err) goto error;

	err = dhm_check_range(
		&dhm->MBEDTLS_PRIVATE(GX),
		&dhm->MBEDTLS_PRIVATE(P));
	if (err) goto error;

	return R_RET;

error:
	//printf("DHM key init failed with error: -0x%0x\n", (unsigned int)-err);
	Free_Hob(VAL_HANDLE_CTX(D_RET));
	return R_NONE;
}

/***********************************************************************
**
*/	REBNATIVE(dh)
/*
//  dh: native [
//		"Diffie-Hellman key exchange"
//		dh-key [handle!] "DH key created using `dh-init` function"
//		/public  "Returns public key as a binary"
//		/secret  "Computes secret result using peer's public key"
//			public-key [binary!] "Peer's public key"
//  ]
***********************************************************************/
{
	REBVAL *key        = D_ARG(1);
	REBOOL  refPublic  = D_REF(2);
	REBOOL  refSecret  = D_REF(3);
	REBVAL *gy         = D_ARG(4);

	REBSER  *out = NULL;
	REBINT   err;
	DHM_CTX *dhm;
	size_t   gx_len, gy_len, olen = 0;

	if (refPublic && refSecret) {
		// only one can be used
		Trap0(RE_BAD_REFINES);
	}

	if (NOT_VALID_CONTEXT_HANDLE(key, SYM_DHM))
		return R_NONE;	//or? Trap0(RE_INVALID_HANDLE);

	dhm = (DHM_CTX *)VAL_HANDLE_CONTEXT_DATA(key);

	if (refPublic) {
		gx_len = mbedtls_mpi_size(&dhm->MBEDTLS_PRIVATE(GX));
		if (gx_len <= 0) goto error;
		out = Make_Binary(gx_len-1);
		mbedtls_mpi_write_binary(&dhm->MBEDTLS_PRIVATE(GX), BIN_DATA(out), gx_len);
		BIN_LEN(out) = gx_len;
	}
	if (refSecret) {
		// import oposite's side public data...
		gy_len = VAL_SERIES(gy)->tail - VAL_INDEX(gy);
		err = mbedtls_mpi_read_binary(&dhm->MBEDTLS_PRIVATE(GY), VAL_BIN_AT(gy), gy_len);
		if (err) goto error;
		out = Make_Binary(gy_len-1);
		// and derive the shared secret of these 2 public parts
		err = mbedtls_dhm_calc_secret(dhm, BIN_DATA(out), gy_len, &olen, mbedtls_ctr_drbg_random, &ctr_drbg);
		if (err) goto error;
		BIN_LEN(out) = olen;
	}
	SET_BINARY(D_RET, out);
	return R_RET;
error:
	if (out != NULL) FREE_SERIES(out);
	return R_NONE;
}

#define ECP_GROUP_ID_VAL(v) (mbedtls_ecp_group_id)(VAL_WORD_CANON(v) - SYM_SECP192R1 + 1);

/***********************************************************************
**
*/	REBNATIVE(ecdh)
/*
//  ecdh: native [
//		"Elliptic-curve Diffie-Hellman key exchange"
//		key [handle! none!] "Keypair to work with, may be NONE for /init refinement"
//		/init   "Initialize ECC keypair."
//			type [word!] "One of supported curves: system/catalog/elliptic-curves"
//		/curve  "Returns handles curve type"
//		/public "Returns public key as a binary"
//		/secret  "Computes secret result using peer's public key"
//			public-key [binary!] "Peer's public key"
//  ]
***********************************************************************/
{
	REBVAL *val_handle  = D_ARG(1);
	REBOOL  ref_init    = D_REF(2);
	REBVAL *val_curve   = D_ARG(3);
	REBOOL  ref_type    = D_REF(4);
	REBOOL  ref_public  = D_REF(5);
	REBOOL  ref_secret  = D_REF(6);
	REBVAL *val_public  = D_ARG(7);


	ECDH_CTX *ctx;
	REBINT err = 0;
	REBSER *bin = NULL;
	mbedtls_ecp_group_id gid;
	mbedtls_ecdh_context_mbed *mbed;
	size_t olen = 0;

	// make sure that only valid combination of refinements is used!
	if (
		(ref_type   && (ref_public || ref_secret )) ||
		(ref_public && (ref_type   || ref_secret )) ||
		(ref_secret && (ref_public || ref_type   ))
	) {
		Trap0(RE_BAD_REFINES);
	}

	if (ref_init) {
		MAKE_HANDLE(val_handle, SYM_ECDH);
	}
	if (NOT_VALID_CONTEXT_HANDLE(val_handle, SYM_ECDH))
		return R_NONE;	//or? Trap0(RE_INVALID_HANDLE);

	ctx = (ECDH_CTX *)VAL_HANDLE_CONTEXT_DATA(val_handle);

	if (ref_init) {
		mbedtls_ecdh_init(ctx);
		gid = ECP_GROUP_ID_VAL(val_curve);
		err = mbedtls_ecdh_setup(ctx, gid);
		if (err) goto error;
		mbed = &ctx->MBEDTLS_PRIVATE(ctx).MBEDTLS_PRIVATE(mbed_ecdh);
		err = mbedtls_ecdh_gen_public(
			&mbed->MBEDTLS_PRIVATE(grp), &mbed->MBEDTLS_PRIVATE(d), &mbed->MBEDTLS_PRIVATE(Q),
			mbedtls_ctr_drbg_random, &ctr_drbg);
		if (err) goto error;
		if (!(ref_type || ref_public || ref_secret)) return R_ARG1;
	}

	if (ref_type) {
		gid = ctx->MBEDTLS_PRIVATE(grp_id);
		if (gid < 1) return R_NONE;
		Init_Word(val_curve, gid + SYM_SECP192R1 - 1);
		return R_ARG3;
	}
	if (ref_public) {
		bin = Make_Binary(256);
		mbed = &ctx->MBEDTLS_PRIVATE(ctx).MBEDTLS_PRIVATE(mbed_ecdh);
		mbedtls_ecp_point Q = mbed->MBEDTLS_PRIVATE(Q);
		// generate the public key, if it does not exists yet
		if (Q.MBEDTLS_PRIVATE(X).MBEDTLS_PRIVATE(p) == NULL) {
			err = mbedtls_ecdh_gen_public(
				&mbed->MBEDTLS_PRIVATE(grp), &mbed->MBEDTLS_PRIVATE(d), &mbed->MBEDTLS_PRIVATE(Q),
				mbedtls_ctr_drbg_random, &ctr_drbg);
			if (err) goto error;
		}
		// and return it.
		err = mbedtls_ecp_point_write_binary(
			&mbed->MBEDTLS_PRIVATE(grp),
			&mbed->MBEDTLS_PRIVATE(Q),
			MBEDTLS_ECP_PF_UNCOMPRESSED,
			&olen, BIN_DATA(bin), 256
		);
		if (err) goto error;
		SET_BINARY(D_RET, bin);
		BIN_LEN(bin) = olen;
	}
	if (ref_secret) {
		mbed = &ctx->MBEDTLS_PRIVATE(ctx).MBEDTLS_PRIVATE(mbed_ecdh);
		err = mbedtls_ecp_point_read_binary(
			&mbed->MBEDTLS_PRIVATE(grp),
			&mbed->MBEDTLS_PRIVATE(Qp),
			VAL_BIN_AT(val_public),
			VAL_LEN(val_public)
		);
		if (err) goto error;
		bin = Make_Binary(256);
		err = mbedtls_ecdh_calc_secret(ctx, &olen, BIN_DATA(bin), SERIES_REST(bin), mbedtls_ctr_drbg_random, &ctr_drbg);
		if (err) goto error;
		
		SET_BINARY(D_RET, bin);
		BIN_LEN(bin) = olen;
	}
	return R_RET;

error:
	//printf("ECDH failed with error: -0x%0x\n", (unsigned int)-err);
	if (ref_init) Free_Hob(VAL_HANDLE_CTX(val_handle));
	if (bin != NULL) Free_Series(bin);
	return R_NONE;
}


/***********************************************************************
**
*/	REBNATIVE(generate)
/*
//  generate: native [
//		"Generate specified cryptographic key"
//		type [word!] "Key type: system/catalog/elliptic-curves"
//  ]
***********************************************************************/
{
	REBVAL *val_type = D_ARG(1);
	REBINT  err;
	REBSER *bin = NULL;
	
	mbedtls_ecdsa_context ctx;
	mbedtls_ecp_group_id gid;

	gid = ECP_GROUP_ID_VAL(val_type);
	if (gid < 1 || gid >= MBEDTLS_ECP_DP_MAX)
		Trap_Arg(val_type);

	mbedtls_ecdsa_init(&ctx);

	//err = mbedtls_ecdsa_genkey(&ctx, gid, mbedtls_ctr_drbg_random, &ctr_drbg);
	//if (err) goto exit;
	err = mbedtls_ecp_group_load(&ctx.MBEDTLS_PRIVATE(grp), MBEDTLS_ECP_DP_SECP192R1);
	//err = mbedtls_ecp_gen_keypair(&ctx.MBEDTLS_PRIVATE(grp), &ctx->d, &ctx->Q, f_rng, p_rng);
	size_t len;
	bin = Make_Binary(300);
	err = mbedtls_ecp_point_write_binary(&ctx.MBEDTLS_PRIVATE(grp), &ctx.MBEDTLS_PRIVATE(Q),
		MBEDTLS_ECP_PF_UNCOMPRESSED, &len, BIN_DATA(bin), SERIES_REST(bin));
	if (err) {
		FREE_SERIES(bin);
	}
	else {
		SET_BINARY(D_RET, bin);
		BIN_LEN(bin) = len;
	}
exit:
	mbedtls_ecdsa_free(&ctx);
	return (err) ? R_NONE : R_RET;
}

/***********************************************************************
**
*/	REBNATIVE(ecdsa)
/*
//  ecdsa: native [
//		"Elliptic Curve Digital Signature Algorithm"
//		key [handle! binary!] "Keypair to work with, created using ECDH function, or raw binary key (needs /curve)"
//		hash [binary!] "Data to sign or verify"
//		/sign   "Use private key to sign data, returns ASN1 encoded result"
//		/verify "Use public key to verify signed data, returns true or false"
//			signature [binary!] "ASN1 encoded"
//		/curve "Used if key is just a binary"
//			type [word!] "One of supported curves: system/catalog/elliptic-curves"
//  ]
***********************************************************************/
{
	REBVAL *val_key     = D_ARG(1);
	REBVAL *val_hash    = D_ARG(2);
	REBOOL  ref_sign    = D_REF(3);
	REBOOL  ref_verify  = D_REF(4);
	REBVAL *val_sign    = D_ARG(5);
	REBOOL  ref_curve   = D_REF(6);
	REBVAL *val_curve   = D_ARG(7);

	REBSER *bin = NULL;
	size_t  len = 0;
	REBINT  err = 0;
	mbedtls_ecp_group_id gid;
	ECDH_CTX *ctx_ecdh = NULL;
	mbedtls_ecdh_context_mbed *mbed;
	mbedtls_mpi r, s;

	if (IS_BINARY(val_key)) {
		if (!ref_curve) Trap0(RE_MISSING_ARG);
		gid = ECP_GROUP_ID_VAL(val_curve);
		if (gid < 1 || gid >= MBEDTLS_ECP_DP_MAX)
			Trap_Arg(val_curve);

		ctx_ecdh = (ECDH_CTX *)malloc(sizeof(ECDH_CTX));
		mbedtls_ecdh_init(ctx_ecdh);
		err = mbedtls_ecdh_setup(ctx_ecdh, gid);
		if (err) goto failed_init;
		mbed = &ctx_ecdh->MBEDTLS_PRIVATE(ctx).private_mbed_ecdh;
		err = mbedtls_ecp_point_read_binary(
			&mbed->MBEDTLS_PRIVATE(grp),
			&mbed->MBEDTLS_PRIVATE(Q),
			VAL_BIN_AT(val_key),
			VAL_LEN(val_key)
		);
		if (err) {
		failed_init:
			mbedtls_ecdh_free(ctx_ecdh);
			free(ctx_ecdh);
			return R_NONE;
		}
	}
	else {
		if (NOT_VALID_CONTEXT_HANDLE(val_key, SYM_ECDH)) {
			Trap0(RE_INVALID_HANDLE);
		}
		ctx_ecdh = (ECDH_CTX *)VAL_HANDLE_CONTEXT_DATA(val_key);
		gid = ctx_ecdh->MBEDTLS_PRIVATE(grp_id);
	}

	if (ref_sign) {
		mbed = &ctx_ecdh->MBEDTLS_PRIVATE(ctx).private_mbed_ecdh;
		mbedtls_mpi_init(&r);
		mbedtls_mpi_init(&s);
		err = mbedtls_ecp_gen_keypair(
			&mbed->MBEDTLS_PRIVATE(grp),
			&mbed->MBEDTLS_PRIVATE(d),
			&mbed->MBEDTLS_PRIVATE(Q),
			mbedtls_ctr_drbg_random, &ctr_drbg);
		if (err) goto done;

		err = mbedtls_ecdsa_sign(
			&mbed->MBEDTLS_PRIVATE(grp),
			&r, &s,
			&mbed->MBEDTLS_PRIVATE(d),
			VAL_BIN_AT(val_hash), VAL_LEN(val_hash),
			mbedtls_ctr_drbg_random, &ctr_drbg);
		if (err) goto done;

		bin = Make_Binary(300);
		err = ecdsa_signature_to_asn1(&r, &s, BIN_DATA(bin), SERIES_REST(bin), &len);
		if (err) {
			FREE_SERIES(bin);
			goto done;
		}
		SET_BINARY(D_RET, bin);
		BIN_LEN(bin) = len;
	}
	if (ref_verify) {
		mbed = &ctx_ecdh->MBEDTLS_PRIVATE(ctx).private_mbed_ecdh;
		unsigned char *p = VAL_BIN_AT(val_sign);
		const unsigned char *end = VAL_BIN_TAIL(val_sign);
		mbedtls_mpi_init(&r);
		mbedtls_mpi_init(&s);
		err = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
		if (err || p + len != end) goto done;
		if ((err = mbedtls_asn1_get_mpi(&p, end, &r)) != 0 ||
			(err = mbedtls_asn1_get_mpi(&p, end, &s)) != 0) {
			goto done;
		}
		err = ecdsa_verify_restartable(
			&mbed->MBEDTLS_PRIVATE(grp),
			VAL_BIN_AT(val_hash), VAL_LEN(val_hash),
			&mbed->MBEDTLS_PRIVATE(Q), &r, &s, NULL);
		if (err) goto done;
		SET_TRUE(D_RET);
	}

done:
	mbedtls_mpi_free(&r);
	mbedtls_mpi_free(&s);
	if (IS_BINARY(val_key)) {
		mbedtls_ecdh_free(ctx_ecdh);
		free(ctx_ecdh);
	}
	return err ? R_NONE : R_RET;

}


/***********************************************************************
**
*/	REBNATIVE(chacha20)
/*
//  chacha20: native [
//		"Encrypt/decrypt data using ChaCha20 algorithm. Returns stream cipher context handle or encrypted/decrypted data."
//		ctx [handle! binary!] "ChaCha20 handle and or binary key for initialization (16 or 32 bytes)"
//		/init
//			nonce [binary!] "Initialization nonce (IV) - 8 or 12 bytes."
//			count [integer!] "A 32-bit block count parameter"
//		/aad sequence [integer!] "Sequence number used with /init to modify nonce"
//		/stream
//			data [binary!]  "Data to encrypt/decrypt."
//		/into
//			out [binary!]   "Output buffer (NOT YET IMPLEMENTED)"
//  ]
***********************************************************************/
{
#ifndef INCLUDE_CHACHA20POLY1305_DEPRECATED
	Trap0(RE_FEATURE_NA);
#else
	REBVAL *val_ctx       = D_ARG(1);
	REBOOL  ref_init      = D_REF(2);
    REBVAL *val_nonce     = D_ARG(3);
	REBVAL *val_counter   = D_ARG(4);
	REBOOL  ref_aad       = D_REF(5);
	REBVAL *val_sequence  = D_ARG(6);
    REBOOL  ref_stream    = D_REF(7);
    REBVAL *val_data      = D_ARG(8);
	REBOOL  ref_into      = D_REF(9);
	
	if(ref_into)
		Trap0(RE_FEATURE_NA);

	REBINT  len;
	REBU64  sequence;

	if (IS_BINARY(val_ctx)) {
		len = VAL_LEN(val_ctx);
		if (!(len == 32 || len == 16)) {
			//printf("ChaCha20 key must be of size 32 or 16 bytes! Is: %i\n", len);
			Trap1(RE_INVALID_DATA, val_ctx);
			return R_NONE;
		}
		REBYTE *bin_key = VAL_BIN_AT(val_ctx);
		
		MAKE_HANDLE(val_ctx, SYM_CHACHA20);
		chacha20_keysetup((chacha20_ctx*)VAL_HANDLE_CONTEXT_DATA(val_ctx), bin_key, len);
	}
	else {
		if (NOT_VALID_CONTEXT_HANDLE(val_ctx, SYM_CHACHA20)) {
    		Trap0(RE_INVALID_HANDLE);
			return R_NONE; // avoid wornings later
		}
	}

	if (ref_init) {
		// initialize nonce with counter
		
		len = VAL_LEN(val_nonce);

		if (!(len == 12 || len == 8)) {
			Trap1(RE_INVALID_DATA, val_nonce);
			return R_NONE;
		}

		sequence = (ref_aad) ? VAL_INT64(val_sequence) : 0;
		chacha20_ivsetup((chacha20_ctx*)VAL_HANDLE_CONTEXT_DATA(val_ctx), VAL_BIN_AT(val_nonce), len, VAL_INT64(val_counter), (u8 *)&sequence);
    }
	
	if (ref_stream) {

    	len = VAL_LEN(val_data);
    	if (len == 0) return R_NONE;

		REBYTE *data = VAL_BIN_AT(val_data);
		REBSER  *binaryOut = Make_Binary(len);

		chacha20_encrypt(
			(chacha20_ctx *)VAL_HANDLE_CONTEXT_DATA(val_ctx),
			(const uint8_t*)data,
			(      uint8_t*)BIN_DATA(binaryOut),
			len
		);

		SET_BINARY(val_ctx, binaryOut);
		VAL_TAIL(val_ctx) = len;

    }
#endif // EXCLUDE_CHACHA20POLY1305
	return R_ARG1;
}



/***********************************************************************
**
*/	REBNATIVE(poly1305)
/*
//  poly1305: native [
//		"poly1305 message-authentication"
//		ctx [handle! binary!] "poly1305 handle and or binary key for initialization (32 bytes)"
//		/update data [binary!] "data to authenticate"
//		/finish                "finish data stream and return raw result as a binary"
//		/verify                "finish data stream and compare result with expected result (MAC)"
//			mac      [binary!] "16 bytes of verification MAC"
//  ]
***********************************************************************/
{
#ifndef INCLUDE_CHACHA20POLY1305_DEPRECATED
	Trap0(RE_FEATURE_NA);
#else
	REBVAL *val_ctx       = D_ARG(1);
	REBOOL  ref_update    = D_REF(2);
	REBVAL *val_data      = D_ARG(3);
	REBOOL  ref_finish    = D_REF(4);
	REBOOL  ref_verify    = D_REF(5);
	REBVAL *val_mac       = D_ARG(6);
    
    REBVAL *ret = D_RET;
//	REBSER *ctx_ser;
	REBINT  len;
//	REBCNT  i;
	REBYTE  mac[16];

	if (IS_BINARY(val_ctx)) {
		len = VAL_LEN(val_ctx);
		if (len < POLY1305_KEYLEN) {
			Trap1(RE_INVALID_DATA, val_ctx);
			return R_NONE;
		}
		
		REBYTE *bin_key = VAL_BIN_AT(val_ctx);
		
		MAKE_HANDLE(val_ctx, SYM_POLY1305);
		poly1305_init((poly1305_context*)VAL_HANDLE_CONTEXT_DATA(val_ctx), bin_key);
	}
	else {
		if (NOT_VALID_CONTEXT_HANDLE(val_ctx, SYM_POLY1305)) {
    		Trap0(RE_INVALID_HANDLE);
		}
	}

	if (ref_update) {
		poly1305_update((poly1305_context*)VAL_HANDLE_CONTEXT_DATA(val_ctx), VAL_BIN_AT(val_data), VAL_LEN(val_data));
	}

	if (ref_finish) {
		SET_BINARY(ret, Make_Series(16, 1, FALSE));
		VAL_TAIL(ret) = 16;
		poly1305_finish((poly1305_context*)VAL_HANDLE_CONTEXT_DATA(val_ctx), VAL_BIN(ret));
		return R_RET;
	}

	if (ref_verify) {
		if (VAL_LEN(val_mac) != POLY1305_TAGLEN)
			return R_FALSE; // or error?
		CLEARS(mac);
		poly1305_finish((poly1305_context*)VAL_HANDLE_CONTEXT_DATA(val_ctx), mac);
		return (poly1305_verify(VAL_BIN_AT(val_mac), mac)) ? R_TRUE : R_FALSE;
	}

#endif // EXCLUDE_CHACHA20POLY1305
	return R_ARG1;
}

/***********************************************************************
**
*/	REBNATIVE(chacha20poly1305)
/*
//  chacha20poly1305: native [
//		"..."
//		ctx [none! handle!]
//		/init 
//			local-key     [binary!]
//			local-iv      [binary!]
//			remote-key    [binary!]
//			remote-iv     [binary!]
//		/encrypt
//			data-out      [binary!]
//			aad-out       [binary!]
//		/decrypt
//			data-in       [binary!]
//			aad-in        [binary!]
//  ]
***********************************************************************/
{
#ifndef INCLUDE_CHACHA20POLY1305_DEPRECATED
	Trap0(RE_FEATURE_NA);
#else
	REBVAL *val_ctx        = D_ARG(1);
	REBOOL  ref_init       = D_REF(2);
	REBVAL *val_local_key  = D_ARG(3);
	REBVAL *val_local_iv   = D_ARG(4);
	REBVAL *val_remote_key = D_ARG(5);
	REBVAL *val_remote_iv  = D_ARG(6);
	REBOOL  ref_encrypt    = D_REF(7);
	REBVAL *val_plain      = D_ARG(8);
	REBVAL *val_local_aad  = D_ARG(9);
	REBOOL  ref_decrypt    = D_REF(10);
	REBVAL *val_cipher     = D_ARG(11);
	REBVAL *val_remote_aad = D_ARG(12);

	REBSER *ctx_ser;
	REBINT  len;
	chacha20poly1305_ctx *chacha;
	unsigned char poly1305_key[POLY1305_KEYLEN];
	size_t aad_size;
	REBU64 sequence = 0;

	if (ref_init) {
		MAKE_HANDLE(val_ctx, SYM_CHACHA20POLY1305);

		chacha = (chacha20poly1305_ctx*)VAL_HANDLE_CONTEXT_DATA(val_ctx);
		len = VAL_LEN(val_local_key);
		if (!(len == 32 || len == 16))
			Trap1(RE_INVALID_DATA, val_local_key);
		chacha20_keysetup(&chacha->local_chacha, VAL_BIN_AT(val_local_key), len);
		len = VAL_LEN(val_remote_key);
		if (!(len == 32 || len == 16))
			Trap1(RE_INVALID_DATA, val_remote_key);
		chacha20_keysetup(&chacha->remote_chacha, VAL_BIN_AT(val_remote_key), len);

		len = VAL_LEN(val_local_iv);
		if (!(len == 12 || len == 8))
			Trap1(RE_INVALID_DATA, val_local_iv);
		chacha20_ivsetup(&chacha->local_chacha, VAL_BIN_AT(val_local_iv), len, 1, (u8 *)&sequence);
		memcpy(chacha->local_iv, VAL_BIN_AT(val_local_iv), len);

		len = VAL_LEN(val_remote_iv);
		if (!(len == 12 || len == 8))
			Trap1(RE_INVALID_DATA, val_remote_iv);
		chacha20_ivsetup(&chacha->remote_chacha, VAL_BIN_AT(val_remote_iv), len, 1, (u8 *)&sequence);
		memcpy(chacha->remote_iv, VAL_BIN_AT(val_remote_iv), len);
		return R_ARG1;
	}

	if (NOT_VALID_CONTEXT_HANDLE(val_ctx, SYM_CHACHA20POLY1305)) {
    	Trap0(RE_INVALID_HANDLE);
		return R_NONE;
	}
	chacha = (chacha20poly1305_ctx*)VAL_HANDLE_CONTEXT_DATA(val_ctx);

	if (ref_encrypt) {
		chacha20_ivsetup(&chacha->local_chacha, chacha->local_iv, 12, 1,  VAL_BIN_AT(val_local_aad));
		chacha20_poly1305_key(&chacha->local_chacha, poly1305_key);
		//puts("poly1305_key:"); Dump_Bytes(poly1305_key, POLY1305_KEYLEN);
		
		len = VAL_LEN(val_plain) + POLY1305_TAGLEN;
		ctx_ser = Make_Series(len, (REBCNT)1, FALSE);

		// AEAD
		// sequence number (8 bytes)
		// content type (1 byte)
		// version (2 bytes)
		// length (2 bytes)
		unsigned char aad[13];
		aad_size = sizeof(aad);
//		unsigned char *sequence = aad;

		chacha20_poly1305_aead(&chacha->local_chacha, VAL_BIN_AT(val_plain), (REBCNT)len-POLY1305_TAGLEN, VAL_BIN_AT(val_local_aad), VAL_LEN(val_local_aad), poly1305_key, ctx_ser->data);

		//chacha->local_sequence++;

		SERIES_TAIL(ctx_ser) = len;
		SET_BINARY(val_ctx, ctx_ser);
		return R_ARG1;
	}

	if (ref_decrypt) {
		static unsigned char zeropad[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		unsigned char trail[16];
		unsigned char mac_tag[POLY1305_TAGLEN];

		chacha20_ivsetup(&chacha->remote_chacha, chacha->remote_iv, 12, 1, VAL_BIN_AT(val_remote_aad));

		len = VAL_LEN(val_cipher) - POLY1305_TAGLEN;
		if (len <= 0)
			return R_NONE;

		ctx_ser = Make_Series(len, (REBCNT)1, FALSE);

		//puts("\nDECRYPT:");

		chacha20_encrypt(&chacha->remote_chacha, VAL_BIN_AT(val_cipher), ctx_ser->data, len);
		//chacha_encrypt_bytes(&chacha->remote_chacha, VAL_BIN_AT(val_cipher), ctx_ser->data, len);
		chacha20_poly1305_key(&chacha->remote_chacha, poly1305_key);
		//puts("poly1305_key:"); Dump_Bytes(poly1305_key, POLY1305_KEYLEN);

		poly1305_context aead_ctx;
		poly1305_init(&aead_ctx, poly1305_key);

		aad_size = VAL_LEN(val_remote_aad);
		poly1305_update(&aead_ctx, VAL_BIN_AT(val_remote_aad), aad_size);
		int rem = aad_size % 16;
        if (rem)
            poly1305_update(&aead_ctx, zeropad, 16 - rem);
		//puts("update:"); Dump_Bytes(VAL_BIN_AT(val_cipher), len);
        poly1305_update(&aead_ctx, VAL_BIN_AT(val_cipher), len);
        rem = len % 16;
        if (rem)
            poly1305_update(&aead_ctx, zeropad, 16 - rem);
            
        U32TO8_LE(&trail[0], aad_size == 5 ? 5 : 13);
        *(int *)&trail[4] = 0;
        U32TO8_LE(&trail[8], len);
        *(int *)&trail[12] = 0;

		//puts("trail:"); Dump_Bytes(trail, 16);

        poly1305_update(&aead_ctx, trail, 16);
        poly1305_finish(&aead_ctx, mac_tag);

		if (!poly1305_verify(mac_tag, VAL_BIN_TAIL(val_cipher) - POLY1305_TAGLEN)) {
			//puts("MAC verification failed!");
			return R_NONE;
		}

		//puts("mac result:"); Dump_Bytes(mac_tag, POLY1305_TAGLEN);

		SERIES_TAIL(ctx_ser) = len;
		SET_BINARY(val_ctx, ctx_ser);
	}
#endif // EXCLUDE_CHACHA20POLY1305
	return R_ARG1;
}
