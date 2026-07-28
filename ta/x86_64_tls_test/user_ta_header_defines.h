/* SPDX-License-Identifier: BSD-2-Clause */

#ifndef USER_TA_HEADER_DEFINES_H
#define USER_TA_HEADER_DEFINES_H

#define TA_UUID { 0x4f65c0e0, 0x5c8f, 0x4d5f, \
		  { 0xa1, 0xb2, 0x2d, 0x93, 0xd1, 0x0d, 0x7a, 0x01 } }
#define TA_FLAGS 0
#define TA_STACK_SIZE (4 * 1024)
#define TA_DATA_SIZE (16 * 1024)
#define TA_VERSION "1.0"
#define TA_DESCRIPTION "x86_64 local-exec TLS test"

#endif
