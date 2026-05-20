# OP-TEE Trusted OS (For x86_64-LiteBox)
This git contains modified (for LiteBox on x86-64) source code for the
secure side implementation of OP-TEE project.

To build:
make CFG_TEE_CORE_LOG_LEVEL=4 DEBUG=1 ARCH=x86_64 PLATFORM=kbl CROSS_COMPILE64= ldelf

All official OP-TEE documentation has moved to http://optee.readthedocs.io.

// OP-TEE core maintainers
