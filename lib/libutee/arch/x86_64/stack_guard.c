// SPDX-License-Identifier: BSD-2-Clause
/* Copyright (c) 2026, Microsoft Corporation */

#include <compiler.h>
#include <config.h>
#include <stdint.h>
#include <user_ta_header.h>
#include "tcb.h"
#ifdef __LDELF__
#include <ldelf_syscalls.h>
#else
#include <stdlib.h>
#include <utee_syscalls.h>
#endif

static struct x86_64_tcb bootstrap_tcb;

static void __no_stack_protector init_stack_guard(void)
{
#ifdef __LDELF__
	if (IS_ENABLED(_CFG_CORE_STACK_PROTECTOR) &&
	    !bootstrap_tcb.stack_guard) {
		if (_ldelf_gen_rnd_num(&bootstrap_tcb.stack_guard,
				       sizeof(bootstrap_tcb.stack_guard)))
			_ldelf_panic(1);
		/* Keep a NUL byte to terminate unterminated string overwrites. */
		bootstrap_tcb.stack_guard &= ~0xffUL;
	}
#else
	if (IS_ENABLED(_CFG_TA_STACK_PROTECTOR) &&
	    !bootstrap_tcb.stack_guard) {
		if (_utee_cryp_random_number_generate(
			    &bootstrap_tcb.stack_guard,
			    sizeof(bootstrap_tcb.stack_guard)))
			abort();
		/* Keep a NUL byte to terminate unterminated string overwrites. */
		bootstrap_tcb.stack_guard &= ~0xffUL;
	}
#endif
}

void __no_stack_protector __utee_tcb_init_bootstrap(void)
{
	bootstrap_tcb.self = &bootstrap_tcb;
	bootstrap_tcb.dtv = NULL;
	bootstrap_tcb.thread_self = &bootstrap_tcb;
	/* The execution platform enables FSGSBASE and preserves guest FS. */
	asm volatile ("wrfsbase %0" : : "r" (&bootstrap_tcb) : "memory");
	init_stack_guard();
}
