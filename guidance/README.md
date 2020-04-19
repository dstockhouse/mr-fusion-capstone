# Mr. Fusion - Guidance

## Overview

The guidance system is responsible for the following

1. Acting as a server for the other subsystems
2. Reporting the status of the robot to the User Interface
3. Planning a path from the robot's current location to the user selected destination
4. Giving step by step instructions during traversal to the Control's system for where the robot should go. 

For video explanation of the subsystem see [video 25:31-38:20](https://pluto.pr.erau.edu/~icarus/Mr-Fusion-Capstone/4-CDR/GMT20200415-160439_Mr--Fusion_1920x1080.mp4)

For diagrams see [slides 23-36](https://pluto.pr.erau.edu/~icarus/Mr-Fusion-Capstone/4-CDR/Mr%20Fusion%20CDR.pdf)

## Resources
The Guidance subsystem is written entirely in rust. Here are some resources that will help you get aquatinted with the language.

1. [The Rust Book](https://doc.rust-lang.org/book/). A must read. Pay careful attention to the section about ownership. If you are having issues getting your code to compile, chances are its an issue with ownership and you should re-read that section.
2. [Rust Cookbook](https://rust-lang-nursery.github.io/rust-cookbook/about.html). Great resource when you find you need to do a common programming task, for example, file I/O or networking, and need an example implementated in rust.
3. [The Rustlings Course](https://github.com/rust-lang/rustlings/). I personally did not do the exercises but I found the resources within the repo to be extremely helpful.
4. [The Little Book of Rust Macros](https://danielkeep.github.io/tlborm/book/index.html). Chances are you will not need to write your own macro but if you get tired of rust macros being black magic, this is the place to demystify them.
5. [The Cargo Book](https://doc.rust-lang.org/cargo/reference/manifest.html). Probably more than you will ever need to know about cargo (the dependency manager for rust) but has useful infomation about the `Cargo.toml` and `Cargo.lock` files.
6. [The Rust Reference](https://doc.rust-lang.org/stable/reference/items/modules.html). As of writing this document `The Rust Refernce` is incomplete but I found it useful for knowing when to use a `mod` vs a `use` statement.

## Things to be weary

At the time of writing this documentation, Rust does not have a fully featured mocking library. Conveniently mocking out hardware dependencies is the only thing that I think is fundamentally missing from the rust ecosystem. A comparison of the mocking libraries that exist can be found [here](https://asomers.github.io/mock_shootout/) should you decide to update or change our current mocking library `mockall`.

### Things to look for if you decide to pick a different mocking library.

1. Being able to mock external structs and functions
2. Libraries that do not require you to manually create a trait in order to mock.