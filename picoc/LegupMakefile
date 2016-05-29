NAME = picoc
ifeq ($(NO_OPT),)
	NO_OPT=1
endif
ifeq ($(NO_INLINE),)
	NO_INLINE=1
endif

LEVEL = ../..
include $(LEVEL)/Makefile.common

simulator: picoc.c
	clang -o picoc -DSIMULATOR picoc.c
