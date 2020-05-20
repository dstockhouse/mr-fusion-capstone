# Invoke makefiles for the four subsystems

DIRS = guidance navigation control imageproc
BINS = $(DIRS:%=%_main.elf)

.PHONY: all test $(DIRS)
all: $(DIRS)

test:
	make -C guidance test
	make -C navigation test
	make -C control test
	make -C imageproc test

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

deploy:
	gcc -coverage -o deploy deploy.c -Lsystem -lsystem -iquote system/inc

