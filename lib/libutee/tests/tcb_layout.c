// SPDX-License-Identifier: BSD-2-Clause

#include <assert.h>
#include <stdint.h>

#include "tcb.h"

int main(void)
{
	size_t allocation = 0;
	size_t tcb_offset = 0;
	size_t tls_offset = 0;

	assert(x86_64_tcb_calc_layout(24, 8, &allocation, &tcb_offset,
				      &tls_offset));
	assert(allocation == 80);
	assert(tcb_offset == 32);
	assert(tls_offset == 8);
	assert(tcb_offset - tls_offset == 24);

	assert(x86_64_tcb_calc_layout(9, 32, &allocation, &tcb_offset,
				      &tls_offset));
	assert(allocation == 80);
	assert(tcb_offset == 32);
	assert(tls_offset == 0);
	assert(tcb_offset - tls_offset == 32);
	assert(!x86_64_tcb_calc_layout(8, 3, &allocation, &tcb_offset,
				       &tls_offset));
	assert(!x86_64_tcb_calc_layout(SIZE_MAX, 8, &allocation, &tcb_offset,
				       &tls_offset));
	return 0;
}
