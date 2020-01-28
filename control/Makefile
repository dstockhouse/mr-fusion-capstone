# Makefile to automatically compile executable and test binary

# Get current base directory
MAKEDIR = $(realpath $(firstword $(MAKEFILE_LIST)))
DIRNAME = $(dir $(MAKEDIR:%/=%))

# Get the name of the directory we are in (i.e. guidance, navigation, or control)
# Does not work if the current directory has spaces in the name
BASENAME = $(shell basename $(lastword $(DIRNAME)))

# Directories
MAINDIR = main
SRCDIR = src
INCDIR = inc ../system
BLDDIR = bld

OBJDIR = $(BLDDIR)/obj
DEPDIR = $(BLDDIR)/dep
BINDIR = $(BLDDIR)/bin

# Locate standard (non-main) source files, and corresponding object files
SRCS = $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.c))
OBJS = $(foreach src,$(notdir $(SRCS)),$(OBJDIR)/$(src:.c=.o))

# Main sources and objects
# MAIN = $(foreach dir,$(MAINDIR),$(wildcard $(dir)/*.c))
MAINSRC = $(BASENAME)_main.c
# MAINSRCALL = $(wildcard $(MAINDIR)/*.c)
MAINSRCALL = $(MAIN)
MAINBASE = $(basename $(notdir $(MAINSRC)))
# MAINSRC = $(addprefix $(MAINDIR)/, $(addsuffix .c, $(MAINBASE)))
MAINOBJ = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(MAINBASE)))
MAINBIN = $(addprefix $(BINDIR)/, $(addsuffix .elf, $(MAINBASE)))
# MAINBINALL = $(foreach b,$(MAINALL),$(BINDIR)/$(b))
MAINBINALL = $(addprefix $(BINDIR)/,$(MAINALL))
# MAINOBJALL = $(foreach o,$(MAINALL),$(OBJDIR)/$(o).o)
MAINOBJALL = $(addprefix $(OBJDIR)/,$(addsuffix .o, $(MAINALL)))

TESTSRC = $(BASENAME)_test.c
TESTSRCALL = $(TEST)
TESTBASE = $(basename $(notdir $(TESTSRC)))
TESTOBJ = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(TESTBASE)))
TESTBIN = $(addprefix $(BINDIR)/, $(addsuffix .elf, $(TESTBASE)))

# Compiler flags
CC = gcc
CFLAGS = -g -Wall $(foreach inc,$(INCDIR),-iquote $(inc))
LIBS = 
LIBFLAGS = $(foreach lib,$(LIBS),-l$(lib))


# Set up path for source file searching
SRCPATH = $(MAINDIR: =:):$(SRCDIR: =:)
vpath %.c $(SRCPATH)

vpath %.o $(OBJDIR)

# Useful debug print for make variables
# $(info MAKEDIR ${MAKEDIR} DIRNAME ${DIRNAME} BASENAME ${BASENAME} OBJS ${OBJS} MAINOBJ ${MAINOBJ} TESTOBJ ${TESTOBJ})



### Start rules

# Default
$(MAINBIN):

# Test
.PHONY: test
test: LIBS += cgreen
test: $(TESTBIN)
	cp $(TESBIN) .
	./$(notdir TESTBIN)
	

# $(MAIN): $(MAINBIN)
# 	cp $^ $@

# $(MAINBIN): $(OBJS) $(MAINOBJ)
# $(CC) $(CFLAGS) $(LIBFLAGS) $^ -o $@

# $(TESTBIN): $(OBJS) $(TESTOBJ)

# Directory for all executable binaries
$(MAINBIN) $(TESTBIN): | $(BINDIR)

# Rule for all executable binaries
$(BINDIR)/%.elf: $(OBJDIR)/%.o $(OBJS)
	$(CC) $(CFLAGS) $(LIBFLAGS) $^ -o $@

$(OBJS) $(MAINOBJ) $(TESTOBJ): | $(OBJDIR)
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(LIBFLAGS) -c $< -o $@

$(OBJDIR) $(DEPDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(BLDDIR) $(notdir $(MAINBIN)) $(notdir $(TESTBIN))


### Below this line are sandbox commands for possible extensions
#    * Auto detecting main source files (now only supports *_main and *_test)


# 
#$(MAINALL): %: $(BINDIR)/%
#	@echo cp $^ $@
#
#$(MAINBINALL): $(OBJS)
#	@echo $(CC) $(CFLAGS) $(LIBFLAGS) $^ -o $@
