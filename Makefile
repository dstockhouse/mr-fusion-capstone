# Invoke makefiles for the four subsystems

DIRS = guidance navigation control imageproc
BINS = $(DIRS:%=%_main.elf)

.PHONY: all $(DIRS)
all: $(DIRS)

guidance:
	make -C guidance
	cp guidance/guidance_main.elf .

navigation:
	make -C navigation
	cp navigation/navigation_main.elf .

control:
	make -C control
	cp control/control_main.elf .

imageproc:
	make -C imageproc
	cp imageproc/imageproc_main.elf .

