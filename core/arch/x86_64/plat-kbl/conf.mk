$(call force,CFG_GENERIC_BOOT,y)
$(call force,CFG_APIC,y)
$(call force,CFG_UART,y)
$(call force,CFG_PM_STUBS,y)
$(call force,CFG_SECURE_TIME_SOURCE_REE,y)

$(call force,CFG_WITH_LPAE,y)
#femi: CFG_DYN_CONFIG did not exist on 3.13. It is conflicting
# with static thread definition in arch x86_64 threads.c
$(call force,CFG_DYN_CONFIG,n)

CFG_WITH_STACK_CANARIES ?= n
CFG_WITH_STATS ?= y
CFG_TEE_CORE_EMBED_INTERNAL_TESTS ?= y

CFG_TEE_CORE_NB_CORE = 1
CFG_NUM_THREADS ?= 2

CFG_TA_DYNLINK ?= y

CFG_CORE_ASLR ?= n
CFG_CORE_DYN_SHM ?= n

# use mbedtls lib
CFG_CRYPTOLIB_NAME ?= mbedtls
CFG_CRYPTOLIB_DIR ?= lib/libmbedtls

# Tested on Kaby Lake NUC
CFG_TZDRAM_START ?= 0x12f52000 #runtime_addr + 1 Page
CFG_TZDRAM_SIZE  ?= 0x00dff000
CFG_SHMEM_START  ?= 0x13d51000
CFG_SHMEM_SIZE   ?= 0x00200000
CFG_TEE_RAM_VA_SIZE ?= 0x00200000

$(call force,CFG_BOOT_SECONDARY_REQUEST,n)
$(call force,CFG_DT,n)
