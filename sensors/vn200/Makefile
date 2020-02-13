
CC = gcc
CFLAGS = -g -Wall $(foreach inc,$(INCDIR) $(MAININCDIR),-iquote $(inc))
#CFLAGS = -g $(foreach inc,$(INCDIR) $(MAININCDIR),-iquote $(inc))
LIBS =
LIBFLAGS = $(foreach lib,$(LIBS),-l$(lib))

MAINDIR = main
APIDIR = api
BLDDIR = bld

OBJDIR = $(BLDDIR)/obj
DEPDIR = $(BLDDIR)/dep
BINDIR = $(BLDDIR)/bin

SRCDIR = $(foreach dir,$(APIDIR),$(dir)/src)
INCDIR = $(foreach dir,$(APIDIR),$(dir)/inc)
SRCS = $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.c))
OBJS = $(foreach src,$(notdir $(SRCS)),$(OBJDIR)/$(src:.c=.o))

MAINSRCDIR = $(MAINDIR)/src
MAININCDIR = $(MAINDIR)/inc

#MAIN = vn200rawsimpletest
MAIN = oldparsetest
MAINSRC = $(MAINSRCDIR)/$(MAIN).c
MAINOBJ = $(OBJDIR)/$(MAIN).o
MAINBIN = $(BINDIR)/$(MAIN)

MAINSRCALL = $(wildcard $(MAINSRCDIR)/*.c)
MAINALL = $(basename $(notdir $(MAINSRCALL)))
MAINBINALL = $(foreach b,$(MAINALL),$(BINDIR)/$(b))
MAINOBJALL = $(foreach o,$(MAINALL),$(OBJDIR)/$(o).o)

# Set up path for source file searching
SRCPATH = $(MAINSRCDIR: =:):$(SRCDIR: =:)
vpath %.c $(SRCPATH)

vpath %.o $(OBJDIR)

$(MAIN): $(MAINBIN)
	cp $^ $@

$(MAINBIN): | $(BINDIR)
$(MAINBIN): $(OBJS) $(MAINOBJ)
	$(CC) $(CFLAGS) $(LIBFLAGS) $^ -o $@

#$(MAINALL): %: $(BINDIR)/%
#	@echo cp $^ $@
#
#$(MAINBINALL): $(OBJS)
#	@echo $(CC) $(CFLAGS) $(LIBFLAGS) $^ -o $@

$(OBJS) $(MAINOBJALL): | $(OBJDIR)
$(OBJS) $(MAINOBJALL): $(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(LIBFLAGS) -c $< -o $@

$(OBJDIR) $(DEPDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(BLDDIR) $(MAINALL)
