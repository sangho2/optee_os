/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdint.h>
#include <config.h>
#include <link.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#define TLS_INITIAL_VALUE UINT64_C(0x1122334455667788)
#define TLS_MUTATED_VALUE UINT64_C(0x8877665544332211)
#define TLS_ZERO_MUTATED_VALUE UINT64_C(0xa5a5a5a55a5a5a5a)
#define TLS_ZERO_TWO_MUTATED_VALUE UINT64_C(0x5a5a5a5aa5a5a5a5)

static __thread uint64_t tls_initialized = TLS_INITIAL_VALUE;
static __thread uint64_t tls_zero;
static __thread uint64_t tls_zero_two;

struct tls_index {
	unsigned long module;
	unsigned long offset;
};

extern void *__tls_get_addr(struct tls_index *ti);

struct phdr_tls_check {
	void *tls_data;
	bool found;
};

static int check_phdr_tls(struct dl_phdr_info *info, size_t size __unused,
			  void *data)
{
	struct phdr_tls_check *check = data;

	if (info->dlpi_tls_modid == 1 &&
	    info->dlpi_tls_data == check->tls_data)
		check->found = true;
	return 0;
}

static uint64_t protected_tls_read(void)
{
	volatile uint8_t force_stack_guard[16] = { 0 };

	force_stack_guard[0] = 1;
	return tls_initialized + force_stack_guard[0] - 1;
}

static bool tcb_is_valid(void)
{
	struct tls_index ti = { .module = 1 };
	struct phdr_tls_check check = { };
	uintptr_t self = 0;
	uintptr_t dtv = 0;
	uintptr_t thread_self = 0;
	uintptr_t guard = 0;

	asm volatile ("movq %%fs:0, %0" : "=r" (self));
	asm volatile ("movq %%fs:8, %0" : "=r" (dtv));
	asm volatile ("movq %%fs:0x10, %0" : "=r" (thread_self));
	asm volatile ("movq %%fs:0x28, %0" : "=r" (guard));
	if (!dtv)
		return false;
	if ((uintptr_t)&tls_initialized != self - 24 ||
	    (uintptr_t)&tls_zero_two != self - 16 ||
	    (uintptr_t)&tls_zero != self - 8 ||
	    *(uintptr_t *)(dtv + 8) != self - 24)
		return false;
	ti.offset = 0;
	if (__tls_get_addr(&ti) != &tls_initialized)
		return false;
	ti.offset = 16;
	if (__tls_get_addr(&ti) != &tls_zero)
		return false;
	ti.offset = 8;
	if (__tls_get_addr(&ti) != &tls_zero_two)
		return false;
	check.tls_data = *(void **)(dtv + 8);
	dl_iterate_phdr(check_phdr_tls, &check);
	return self && dtv && thread_self == self && check.found &&
	       (!IS_ENABLED(_CFG_TA_STACK_PROTECTOR) || guard);
}

TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types __unused,
				    TEE_Param params[4] __unused,
				    void **session_context __unused)
{
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *session_context __unused)
{
}

TEE_Result TA_InvokeCommandEntryPoint(void *session_context __unused,
				      uint32_t command,
				      uint32_t param_types __unused,
				      TEE_Param params[4] __unused)
{
	switch (command) {
	case 0:
		if (!tcb_is_valid() ||
		    protected_tls_read() != TLS_INITIAL_VALUE || tls_zero ||
		    tls_zero_two)
			return TEE_ERROR_BAD_STATE;
		tls_initialized = TLS_MUTATED_VALUE;
		tls_zero = TLS_ZERO_MUTATED_VALUE;
		tls_zero_two = TLS_ZERO_TWO_MUTATED_VALUE;
		return TEE_SUCCESS;
	case 1:
		if (tls_initialized != TLS_MUTATED_VALUE ||
		    tls_zero != TLS_ZERO_MUTATED_VALUE ||
		    tls_zero_two != TLS_ZERO_TWO_MUTATED_VALUE)
			return TEE_ERROR_BAD_STATE;
		return TEE_SUCCESS;
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
