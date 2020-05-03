# Poor man's makefile wrapper for rust's build system

.PHONY: main debug test release rust_exe_wrapper
main: debug
debug: | guidance_main.elf
	cd rust_code && cargo build && ln -fs target/debug/rust_code guidance_main.elf

test:
	cd rust_code && cargo test -- --test-threads=1

release: | guidance_main.elf
	cd rust_code && cargo build --release && ln -fs target/release/rust_code guidance_main.elf

guidance_main.elf:
	echo '#!/bin/sh\ncd rust_code && ./guidance_main.elf' > $@
	chmod +x $@

