/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2026, Microsoft Corporation */

#ifndef LIBUTEE_X86_64_TCB_H
#define LIBUTEE_X86_64_TCB_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define X86_64_TCB_ALIGNMENT 16
#define X86_64_TCB_DTV_OFFSET 0x8
#define X86_64_TCB_THREAD_SELF_OFFSET 0x10
#ifndef X86_64_STACK_PROTECTOR_GUARD_OFFSET
#define X86_64_STACK_PROTECTOR_GUARD_OFFSET 0x28
#endif
#define X86_64_TCB_STACK_GUARD_OFFSET \
	X86_64_STACK_PROTECTOR_GUARD_OFFSET

struct x86_64_tcb {
	struct x86_64_tcb *self;
	/* Dynamic thread vector shared with the common TLS runtime. */
	void *dtv;
	struct x86_64_tcb *thread_self;
	uintptr_t reserved[2];
	uintptr_t stack_guard;
} __attribute__((aligned(X86_64_TCB_ALIGNMENT)));

static inline bool x86_64_tcb_calc_layout(size_t memsz, size_t align,
					  size_t *allocation_size,
					  size_t *tcb_offset,
					  size_t *tls_offset)
{
	size_t mask = 0;
	size_t tls_size = 0;
	size_t tcb_align = align > X86_64_TCB_ALIGNMENT ? align :
			 X86_64_TCB_ALIGNMENT;

	if (!allocation_size || !tcb_offset || !tls_offset ||
	    (align > 1 && (align & (align - 1))) ||
	    (tcb_align & (tcb_align - 1)))
		return false;
	mask = align > 1 ? align - 1 : 0;
	if (memsz > SIZE_MAX - mask)
		return false;
	tls_size = (memsz + mask) & ~mask;
	mask = tcb_align - 1;
	if (tls_size > SIZE_MAX - mask)
		return false;
	*tcb_offset = (tls_size + mask) & ~mask;
	if (*tcb_offset > SIZE_MAX - sizeof(struct x86_64_tcb))
		return false;
	*allocation_size = *tcb_offset + sizeof(struct x86_64_tcb);
	*tls_offset = *tcb_offset - tls_size;
	return true;
}

_Static_assert(offsetof(struct x86_64_tcb, self) == 0,
	       "x86_64 TCB self pointer offset");
_Static_assert(offsetof(struct x86_64_tcb, dtv) == X86_64_TCB_DTV_OFFSET,
	       "x86_64 TCB DTV pointer offset");
_Static_assert(offsetof(struct x86_64_tcb, thread_self) ==
	       X86_64_TCB_THREAD_SELF_OFFSET,
	       "x86_64 TCB thread self pointer offset");
_Static_assert(offsetof(struct x86_64_tcb, stack_guard) ==
	       X86_64_TCB_STACK_GUARD_OFFSET, "x86_64 TCB stack guard offset");

#endif
