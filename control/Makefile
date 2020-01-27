# Makefile to automatically compile executable and test binary

# Directories
MAINDIR = main
SRCDIR = src
INCDIR = inc
BLDDIR = bld

OBJDIR = $(BLDDIR)/obj
DEPDIR = $(BLDDIR)/dep
BINDIR = $(BLDDIR)/bin

# Locate standard (non-main) source files, and corresponding object files
SRCS = $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.c))
OBJS = $(foreach src,$(notdir $(SRCS)),$(OBJDIR)/$(src:.c=.o))

# Main sources and objects
# MAIN = $(foreach dir,$(MAINDIR),$(wildcard $(dir)/*.c))
MAIN = control_utest.c
# MAINSRCALL = $(wildcard $(MAINDIR)/*.c)
MAINSRCALL = $(MAIN)
MAINALL = $(basename $(notdir $(MAINSRCALL)))
MAINSRC = $(addprefix $(MAINDIR)/, $(addsuffix .c, $(MAINALL)))
MAINOBJ = $(addprefix $(OBJDIR)/, $(addsuffix .o, $(MAINALL)))
MAINBIN = $(addprefix $(BINDIR)/, $(addsuffix .elf, $(MAINALL)))
# MAINBINALL = $(foreach b,$(MAINALL),$(BINDIR)/$(b))
MAINBINALL = $(addprefix $(BINDIR)/,$(MAINALL))
# MAINOBJALL = $(foreach o,$(MAINALL),$(OBJDIR)/$(o).o)
MAINOBJALL = $(addprefix $(OBJDIR)/,$(addsuffix .o, $(MAINALL)))


# Compiler flags
CC = gcc
CFLAGS = -g -Wall $(foreach inc,$(INCDIR) $(MAININCDIR),-iquote $(inc))
#CFLAGS = -g $(foreach inc,$(INCDIR) $(MAININCDIR),-iquote $(inc))
LIBS = cgreen
LIBFLAGS = $(foreach lib,$(LIBS),-l$(lib))


# Set up path for source file searching
SRCPATH = $(MAINDIR: =:):$(SRCDIR: =:)
vpath %.c $(SRCPATH)

vpath %.o $(OBJDIR)

$(info OBJS ${OBJS} MAINOBJALL ${MAINOBJALL})

# $(MAIN): $(MAINBIN)
# 	cp $^ $@

$(MAINBIN): | $(BINDIR)
$(MAINBIN): $(OBJS) $(MAINOBJ)
	$(CC) $(CFLAGS) $(LIBFLAGS) $^ -o $@

#$(MAINALL): %: $(BINDIR)/%
#	@echo cp $^ $@
#
#$(MAINBINALL): $(OBJS)
#	@echo $(CC) $(CFLAGS) $(LIBFLAGS) $^ -o $@

$(OBJS) $(MAINOBJALL): | $(OBJDIR)
# $(OBJS) $(MAINOBJALL): $(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(LIBFLAGS) -c $< -o $@

$(OBJDIR) $(DEPDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(BLDDIR) $(MAINALL)

