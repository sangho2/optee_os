cppflags-y += -I$(sub-dir)/../..

srcs-y += utee_syscalls_64.S
srcs-y += stack_guard.c
cflags-stack_guard.c-y += -fno-stack-protector

ifneq ($(sm),ldelf)
#femi: 31b31015 moves tcb and user_ta_entry out of arch
#srcs-y += tcb.c
#srcs-y += user_ta_entry.c
subdirs-y += gprof
endif #$(sm-$(sm)-is-ld)
