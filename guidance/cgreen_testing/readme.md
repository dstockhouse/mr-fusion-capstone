# To install cgreen
https://cgreen-devs.github.io/index.html#_installing_cgreen

# To compile the executable with no unit testing
```bash
gcc thefile.c -lcgreen
```

# To compile unit tests
```bash
gcc -shared -o unit_tests.so -fPIC first_test.c -lcgreen
```

# To run unit tests
cgreen-runner unit_tests.so