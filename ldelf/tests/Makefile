CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror
override CFLAGS += -UNDEBUG

.PHONY: check clean

check: tcb_layout
	./tcb_layout

tcb_layout: ../../lib/libutee/tests/tcb_layout.c
	$(CC) $(CFLAGS) -std=c11 -I../../lib/libutee/arch/x86_64 $< -o $@

clean:
	rm -f tcb_layout
