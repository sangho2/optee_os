srcs-y += core_mmu.c
srcs-$(CFG_WITH_PAGER) += tee_pager.c
#femi: fc5e089470 moves tee_mm.c out of arch
#srcs-y += tee_mm.c
#femi: 52a75a25 moves pgt_cache.c out of arch
#srcs-y += pgt_cache.c
srcs-$(CFG_CORE_FFA) += mobj_ffa.c
ifneq ($(CFG_CORE_FFA),y)
srcs-$(CFG_CORE_DYN_SHM) += mobj_dyn_shm.c
endif
