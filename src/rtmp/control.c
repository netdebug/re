/**
 * @file rtmp/control.c  Real Time Messaging Protocol (RTMP) -- Control
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <re_types.h>
#include <re_fmt.h>
#include <re_mem.h>
#include <re_mbuf.h>
#include <re_net.h>
#include <re_sa.h>
#include <re_list.h>
#include <re_rtmp.h>
#include "rtmp.h"


int rtmp_control(struct rtmp_conn *conn, enum rtmp_packet_type type, ...)
{
	struct mbuf *mb;
	uint32_t u32;
	uint16_t event;
	va_list ap;
	int err = 0;

	if (!conn)
		return EINVAL;

	mb = mbuf_alloc(8);
	if (!mb)
		return ENOMEM;

	va_start(ap, type);

	switch (type) {

	case RTMP_TYPE_SET_CHUNK_SIZE:
	case RTMP_TYPE_WINDOW_ACK_SIZE:
		u32 = va_arg(ap, uint32_t);
		err = mbuf_write_u32(mb, htonl(u32));
		break;

	case RTMP_TYPE_USER_CONTROL_MSG:
		event = va_arg(ap, unsigned);
		err  = mbuf_write_u16(mb, htons(event));
		err |= mbuf_write_u32(mb, htonl(va_arg(ap, uint32_t)));
		break;

	case RTMP_TYPE_SET_PEER_BANDWIDTH:
		err  = mbuf_write_u32(mb, htonl(va_arg(ap, uint32_t)));
		err |= mbuf_write_u8(mb, va_arg(ap, unsigned));
		break;

	default:
		err = ENOTSUP;
		break;
	}

	va_end(ap);

	if (err)
		goto out;

	err = rtmp_conn_send_msg(conn, 0, RTMP_CHUNK_ID_CONTROL, 0, 0, type,
				 RTMP_CONTROL_STREAM_ID, mb->buf, mb->end);

 out:
	mem_deref(mb);

	return err;
}
