/*
 * nghttp2 - HTTP/2 C Library
 *
 * Modified to take a byte array for decompression
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* !HAVE_CONFIG_H */

#define NGHTTP2_NO_SSIZE_T

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nghttp2/nghttp2.h>

#define MAKE_NV(K, V)                                                                 \
	{                                                                                 \
	    (uint8_t*)K, (uint8_t*)V, sizeof(K) - 1, sizeof(V) - 1, NGHTTP2_NV_FLAG_NONE, \
	}

static void deflate(nghttp2_hd_deflater* deflater, nghttp2_hd_inflater* inflater, const nghttp2_nv* const nva,
                    size_t nvlen);

static int inflate_header_block(nghttp2_hd_inflater* inflater, const uint8_t* in, size_t inlen);

int main(void)
{
	int rv;
	nghttp2_hd_deflater* deflater;
	nghttp2_hd_inflater* inflater;

	/* Define 1st header set */
	nghttp2_nv nva1[] = {MAKE_NV(":scheme", "https"), MAKE_NV(":authority", "example.org"), MAKE_NV(":path", "/"),
	                     MAKE_NV("user-agent", "libnghttp2"), MAKE_NV("accept-encoding", "gzip, deflate")};

	/* Define 2nd header set */
	nghttp2_nv nva2[] = {MAKE_NV(":scheme", "https"),
	                     MAKE_NV(":authority", "example.org"),
	                     MAKE_NV(":path", "/stylesheet/style.css"),
	                     MAKE_NV("user-agent", "libnghttp2"),
	                     MAKE_NV("accept-encoding", "gzip, deflate"),
	                     MAKE_NV("referer", "https://example.org")};

	/* Initialize the deflater */
	rv = nghttp2_hd_deflate_new(&deflater, 4096);
	if (rv != 0)
	{
		fprintf(stderr, "nghttp2_hd_deflate_init failed with error: %s\n", nghttp2_strerror(rv));
		exit(EXIT_FAILURE);
	}

	/* Initialize the inflater */
	rv = nghttp2_hd_inflate_new(&inflater);
	if (rv != 0)
	{
		fprintf(stderr, "nghttp2_hd_inflate_init failed with error: %s\n", nghttp2_strerror(rv));
		exit(EXIT_FAILURE);
	}

	/* Encode and decode 1st header set */
	deflate(deflater, inflater, nva1, sizeof(nva1) / sizeof(nva1[0]));

	/* Encode and decode 2nd header set */
	deflate(deflater, inflater, nva2, sizeof(nva2) / sizeof(nva2[0]));

	/* Cleanup */
	nghttp2_hd_inflate_del(inflater);
	nghttp2_hd_deflate_del(deflater);

	return 0;
}

static void deflate(nghttp2_hd_deflater* deflater, nghttp2_hd_inflater* inflater, const nghttp2_nv* const nva,
                    size_t nvlen)
{
	nghttp2_ssize rv;
	uint8_t* buf;
	size_t buflen;
	size_t outlen;
	size_t i;
	size_t sum = 0;

	/* Calculate total input size */
	for (i = 0; i < nvlen; ++i)
	{
		sum += nva[i].namelen + nva[i].valuelen;
	}

	/* Print headers before compression */
	printf("Input (%zu byte(s)):\n\n", sum);
	for (i = 0; i < nvlen; ++i)
	{
		fwrite(nva[i].name, 1, nva[i].namelen, stdout);
		printf(": ");
		fwrite(nva[i].value, 1, nva[i].valuelen, stdout);
		printf("\n");
	}

	/* Get buffer size needed for compression */
	buflen = nghttp2_hd_deflate_bound(deflater, nva, nvlen);
	buf = (uint8_t*)malloc(buflen);
	if (!buf)
	{
		fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Perform HPACK compression */
	rv = nghttp2_hd_deflate_hd2(deflater, buf, buflen, nva, nvlen);
	if (rv < 0)
	{
		fprintf(stderr, "nghttp2_hd_deflate_hd2() failed: %s\n", nghttp2_strerror((int)rv));
		free(buf);
		exit(EXIT_FAILURE);
	}

	outlen = (size_t)rv;

	/* Print compressed output */
	printf("\nDeflate (%zu byte(s), ratio %.02f):\n\n", outlen, sum == 0 ? 0 : (double)outlen / (double)sum);

	/* Print binary data */
	for (i = 0; i < outlen; ++i)
	{
		if ((i & 0x0fu) == 0) printf("%08zX: ", i);

		printf("%02X ", buf[i]);

		if (((i + 1) & 0x0fu) == 0) printf("\n");
	}
	printf("\n");

	/* Decompress the data using the updated function */
	printf("\nInflate:\n\n");
	rv = inflate_header_block(inflater, buf, outlen);
	if (rv != 0)
	{
		free(buf);
		exit(EXIT_FAILURE);
	}

	free(buf);
}

int inflate_header_block(nghttp2_hd_inflater* inflater, const uint8_t* in, size_t inlen)
{
	nghttp2_ssize rv;

	while (inlen > 0)
	{
		nghttp2_nv nv;
		int inflate_flags = 0;
		size_t proclen;

		/* Decompress one header */
		rv = nghttp2_hd_inflate_hd3(inflater, &nv, &inflate_flags, in, inlen, 1);

		if (rv < 0)
		{
			fprintf(stderr, "inflate failed with error code %td\n", rv);
			return -1;
		}

		proclen = (size_t)rv;
		in += proclen;
		inlen -= proclen;

		/* Print decompressed headers */
		if (inflate_flags & NGHTTP2_HD_INFLATE_EMIT)
		{
			fwrite(nv.name, 1, nv.namelen, stdout);
			printf(": ");
			fwrite(nv.value, 1, nv.valuelen, stdout);
			printf("\n");
		}

		/* End of headers */
		if (inflate_flags & NGHTTP2_HD_INFLATE_FINAL)
		{
			nghttp2_hd_inflate_end_headers(inflater);
			break;
		}
	}

	return 0;
}
