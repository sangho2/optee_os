#femi: d8e4ae07 moves ldelf_loader.c out of arch
#srcs-$(CFG_WITH_USER_TA) += ldelf_loader.c
#femi: 14c0df4e moves tee_time.c out of arch
#srcs-y += tee_time.c
#femi: 683b6d2c6f moves otp_stubs.c out of arch
#srcs-y += otp_stubs.c
srcs-y += descriptor.c
srcs-y += fault.c
srcs-y += gdt.S
srcs-y += exceptions.S
srcs-y += fpu.c

#femi:see tee_time.c above
#srcs-$(CFG_SECURE_TIME_SOURCE_REE) += tee_time_ree.c

srcs-y += spin_lock_64.S
#femi: ec835942 moves spin_lock_debug.c out of arch
#srcs-$(CFG_TEE_CORE_DEBUG) += spin_lock_debug.c

srcs-y += thread_64.S
srcs-y += thread.c
srcs-y += thread_optee_smc.c
#femi:5305bce1 moves trace_ext.c out of arch
#srcs-y += trace_ext.c
srcs-y += misc.c

srcs-y += boot.c
srcs-y += entry_64.S

srcs-$(CFG_VIRTUALIZATION) += virtualization.c

srcs-y += link_dummies_paged.c
srcs-y += link_dummies_init.c

asm-defines-y += asm-defines.c

ifeq ($(CFG_SYSCALL_FTRACE),y)
# We would not like to profile thread.c file as it provide common APIs
# that are needed for ftrace framework to trace syscalls. So profiling
# this file could create an incorrect cyclic behaviour.
cflags-remove-thread.c-y += -pg
cflags-remove-spin_lock_debug.c-$(CFG_TEE_CORE_DEBUG) += -pg
# Tracing abort dump files corrupts the stack trace. So exclude them
# from profiling.
cflags-remove-abort.c-y += -pg
ifeq ($(CFG_UNWIND),y)
cflags-remove-unwind_arm32.c-y += -pg
cflags-remove-unwind_arm64.c-$(CFG_ARM64_core) += -pg
endif
endif
