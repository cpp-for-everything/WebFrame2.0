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

/* Callback: Handles received HEADERS frame */
static int on_header_callback(nghttp2_session* session, const nghttp2_frame* frame, const uint8_t* name, size_t namelen,
                              const uint8_t* value, size_t valuelen, uint8_t flags, void* user_data)
{
	(void)session;
	(void)frame;
	(void)flags;
	(void)user_data;

	printf("Header: %.*s: %.*s\n", (int)namelen, name, (int)valuelen, value);
	return 0;
}

/* Callback: Handles received DATA frame */
static int on_data_chunk_recv_callback(nghttp2_session* session, uint8_t flags, int32_t stream_id, const uint8_t* data,
                                       size_t len, void* user_data)
{
	(void)session;
	(void)flags;
	(void)user_data;

	printf("DATA (Stream %d, %zu bytes): %.*s\n", stream_id, len, (int)len, data);
	return 0;
}

/* Callback: Handles various frame types */
static int on_frame_recv_callback(nghttp2_session* session, const nghttp2_frame* frame, void* user_data)
{
	(void)session;
	(void)user_data;
	printf("%d", frame->hd.type);
	switch (frame->hd.type)
	{
		case NGHTTP2_HEADERS:
			printf("\n[HEADERS] Stream ID: %d\n", frame->hd.stream_id);
			break;
		case NGHTTP2_DATA:
			printf("\n[DATA] Stream ID: %d\n", frame->hd.stream_id);
			break;
		case NGHTTP2_SETTINGS:
			printf("\n[SETTINGS] Frame received\n");
			break;
		case NGHTTP2_PING:
			printf("\n[PING] Received, Opaque Data: ");
			for (int i = 0; i < 8; i++)
			{
				printf("%02X ", frame->ping.opaque_data[i]);
			}
			printf("\n");
			break;
		case NGHTTP2_RST_STREAM:
			printf("\n[RST_STREAM] Stream ID: %d, Error Code: %d\n", frame->hd.stream_id, frame->rst_stream.error_code);
			break;
		case NGHTTP2_WINDOW_UPDATE:
			printf("\n[WINDOW_UPDATE] Stream ID: %d, Increment: %d\n", frame->hd.stream_id,
			       frame->window_update.window_size_increment);
			break;
		default:
			printf("\n[OTHER FRAME] Type: %d, Stream ID: %d\n", frame->hd.type, frame->hd.stream_id);
			break;
	}
	return 0;
}

/* Process HTTP/2 raw byte array */
void process_http2_data(const uint8_t* data, size_t length)
{
	nghttp2_session* session;
	nghttp2_session_callbacks* callbacks;

	/* Create and set up callbacks */
	nghttp2_session_callbacks_new(&callbacks);
	nghttp2_session_callbacks_set_on_header_callback(callbacks, on_header_callback);
	nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, on_data_chunk_recv_callback);
	nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, on_frame_recv_callback);

	/* Create HTTP/2 decoding session */
	nghttp2_session_client_new(&session, callbacks, NULL);

	/* Process raw byte array */
	auto processed = nghttp2_session_mem_recv2(session, data, length);
	if (processed < 0)
	{
		fprintf(stderr, "Failed to process HTTP/2 data: %s\n", nghttp2_strerror((int)processed));
	}

	/* Clean up */
	nghttp2_session_callbacks_del(callbacks);
	nghttp2_session_del(session);
}

int main()
{
	/* Example HTTP/2 byte array with HEADERS, DATA, PING, and SETTINGS frames */
	uint8_t http2_data[] = {
	    /* HEADERS frame (HPACK compressed) */
	    0x00, 0x00, 0x12, 0x01, 0x05, 0x00, 0x00, 0x00, 0x01, 0x82, 0x86, 0x41, 0x8c, 0xf1, 0xe3, 0xc2, 0x02, 0x0f,
	    0x77, 0x77, 0x77, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x6f, 0x72, 0x67,

	    /* DATA frame */
	    0x00, 0x00, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 'H', 'e', 'l', 'l', 'o', ',', ' ', 'H', 'T', 'T', 'P',
	    '/', '2', '!', '\n',

	    /* PING frame */
	    0x00, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0xad, 0xbe, 0xef, 0x00, 0x11, 0x22, 0x33,

	    /* SETTINGS frame */
	    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};

	printf("Processing HTTP/2 byte array...\n");
	process_http2_data(http2_data, sizeof(http2_data));
	return 0;
}
