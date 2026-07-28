// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2020, Huawei Technologies Co., Ltd
 */
/*
 * Support for the Arm TLS variant-I ABI and the x86_64 variant-II ABI.
 * TAs are currently single-threaded.
 */

#ifdef ARM64
#include <arm64_user_sysreg.h>
#endif
#include <assert.h>
#include <link.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include "user_ta_header.h"
#ifdef X86_64
#include "arch/x86_64/tcb.h"
#endif

/* dtv[0].size is the number of modules; dtv[1...] point to TLS blocks. */
union dtv {
	unsigned long size;
	uint8_t *tls;
};

#define DTV_SIZE(size) (sizeof(union dtv) + (size))

#ifdef X86_64
static struct x86_64_tcb *_tcb;
static void *_tcb_allocation;
#else
struct tcb_head {
	/* Two words are reserved as required by the TLS variant-I ABI. */
	union dtv *dtv;
	unsigned long reserved;
	uint8_t tls[];
};

static struct tcb_head *_tcb;
#define TCB_SIZE(tls_size) (sizeof(*_tcb) + (tls_size))
#endif
static size_t _tls_size;

/* Initialize or update the TCB after ldelf has published ELF metadata. */
void __utee_tcb_init(void)
{
	struct dl_phdr_info *dlpi = NULL;
	const Elf_Phdr *phdr = NULL;
	size_t total_size = 0;
#ifndef X86_64
	size_t size = 0;
#endif
	size_t i = 0;
	size_t j = 0;

	for (i = 0; i < __elf_phdr_info.count; i++) {
		dlpi = __elf_phdr_info.dlpi + i;
		for (j = 0; j < dlpi->dlpi_phnum; j++) {
			phdr = dlpi->dlpi_phdr + j;
			if (phdr->p_type == PT_TLS) {
				total_size += phdr->p_memsz;
				break;
			}
		}
	}

	assert(total_size >= _tls_size);
	if (_tcb && total_size == _tls_size)
		return;

#ifdef X86_64
	{
		struct dl_phdr_info *main = __elf_phdr_info.dlpi;
		const Elf_Phdr *tls = NULL;
		union dtv *dtv = NULL;
		size_t allocation_size = 0;
		size_t tcb_offset = 0;
		size_t tls_offset = 0;
		uintptr_t guard = 0;

		/* x86_64 supports local-exec TLS in the main executable only. */
		assert(__elf_phdr_info.count && total_size ==
		       (main->dlpi_tls_modid ? total_size : 0));
		for (j = 0; j < main->dlpi_phnum; j++)
			if (main->dlpi_phdr[j].p_type == PT_TLS)
				tls = main->dlpi_phdr + j;
		assert(!tls || main->dlpi_tls_modid == 1);
		assert(x86_64_tcb_calc_layout(total_size,
					      tls ? tls->p_align : 1,
					      &allocation_size, &tcb_offset,
					      &tls_offset));
		_tcb_allocation = malloc_flags(MAF_ZERO_INIT, NULL,
					       MAX(tls ? (size_t)tls->p_align : 1UL,
						   (size_t)X86_64_TCB_ALIGNMENT),
					       allocation_size);
		if (!_tcb_allocation) {
			EMSG("TCB allocation failed (%zu bytes)", allocation_size);
			abort();
		}
		_tcb = (void *)((uint8_t *)_tcb_allocation + tcb_offset);
		dtv = calloc(2, sizeof(*dtv));
		if (!dtv) {
			EMSG("DTV allocation failed");
			abort();
		}
		dtv[0].size = 1;
		if (tls) {
			dtv[1].tls = (uint8_t *)_tcb_allocation + tls_offset;
			memcpy(dtv[1].tls,
			       (void *)(main->dlpi_addr + tls->p_vaddr),
			       tls->p_filesz);
			memset(dtv[1].tls + tls->p_filesz, 0,
			       tls->p_memsz - tls->p_filesz);
		}
		asm volatile ("movq %%fs:%c1, %0" : "=r" (guard) :
			      "i" (X86_64_TCB_STACK_GUARD_OFFSET));
		_tcb->self = _tcb;
		_tcb->dtv = dtv;
		_tcb->thread_self = _tcb;
		_tcb->stack_guard = guard;
		asm volatile ("wrfsbase %0" : : "r" (_tcb) : "memory");
		_tls_size = total_size;
		return;
	}
#else
	_tcb = malloc_flags(MAF_ZERO_INIT, _tcb, 1, TCB_SIZE(total_size));
	if (!_tcb) {
		EMSG("TCB allocation failed (%zu bytes)", TCB_SIZE(total_size));
		abort();
	}

	size = DTV_SIZE((__elf_phdr_info.count + 1) * sizeof(union dtv));
	_tcb->dtv = malloc_flags(MAF_ZERO_INIT, _tcb->dtv, 1, size);
	if (!_tcb->dtv) {
		EMSG("DTV allocation failed (%zu bytes)", size);
		abort();
	}

	size = 0;
	for (i = 0; i < __elf_phdr_info.count; i++) {
		dlpi = __elf_phdr_info.dlpi + i;
		for (j = 0; j < dlpi->dlpi_phnum; j++) {
			phdr = dlpi->dlpi_phdr + j;
			if (phdr->p_type != PT_TLS)
				continue;
			if (size + phdr->p_memsz <= _tls_size)
				break;
			_tcb->dtv[i + 1].tls = _tcb->tls + size;
			memcpy(_tcb->tls + size,
			       (void *)(dlpi->dlpi_addr + phdr->p_vaddr),
			       phdr->p_filesz);
			memset(_tcb->tls + size + phdr->p_filesz, 0,
			       phdr->p_memsz - phdr->p_filesz);
			size += phdr->p_memsz;
		}
	}
	_tcb->dtv[0].size = i;
	_tls_size = total_size;
#ifdef ARM64
	write_tpidr_el0((vaddr_t)_tcb);
#endif
#endif
}

struct tls_index {
	unsigned long module;
	unsigned long offset;
};

void *__tls_get_addr(struct tls_index *ti);

void *__tls_get_addr(struct tls_index *ti)
{
#ifdef X86_64
	union dtv *dtv = _tcb->dtv;

	return dtv[ti->module].tls + ti->offset;
#else
	return _tcb->dtv[ti->module].tls + ti->offset;
#endif
}

int dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *),
		    void *data)
{
	struct dl_phdr_info *dlpi = NULL;
	size_t id = 0;
	size_t i = 0;
	int st = 0;

	dlpi = calloc(1, sizeof(*dlpi));
	if (!dlpi) {
		EMSG("dl_phdr_info allocation failed");
		abort();
	}
	for (i = 0; i < __elf_phdr_info.count; i++) {
		memcpy(dlpi, __elf_phdr_info.dlpi + i, sizeof(*dlpi));
		dlpi->dlpi_tls_data = NULL;
		id = dlpi->dlpi_tls_modid;
		if (id) {
#ifdef X86_64
			union dtv *dtv = _tcb->dtv;

			dlpi->dlpi_tls_data = dtv[id].tls;
#else
			dlpi->dlpi_tls_data = _tcb->dtv[id].tls;
#endif
		}
		st = callback(dlpi, sizeof(*dlpi), data);
	}
	free(dlpi);
	return st;
}
