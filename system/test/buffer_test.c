#include <cgreen/cgreen.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"

// Name of test context
Describe(Buffer);

// Execute in the context immediately before each "Ensure" test
BeforeEach(Buffer) {
}

// Execute after each test
AfterEach(Buffer) {}


/**** Start test suite ****/

Ensure(Buffer, is_emptied_after_init) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    assert_that(BufferLength(&buf), is_equal_to(0));
}

Ensure(Buffer, has_length_one_after_add) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    int rc = BufferAdd(&buf, 42);
    assert_that(BufferLength(&buf), is_equal_to(1));
    assert_that(rc, is_equal_to(1));
    assert_that(BufferIsFull(&buf), is_false);
}

Ensure(Buffer, has_one_indexed_element_after_add) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    int val = 42;
    int rc = BufferAdd(&buf, val);
    assert_that(BufferIndex(&buf, 0), is_equal_to(val));
    assert_that(BufferIndex(&buf, 1), is_equal_to(0));
    assert_that(BufferIsFull(&buf), is_false);
}

Ensure(Buffer, has_correct_elements_after_add_array) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    const int numElements = 100; unsigned char data[numElements];
    int i;
    for (i = 0; i < numElements; i++) {
        data[i] = (unsigned char) i;
    }

    int rc = BufferAddArray(&buf, data, numElements);

    assert_that(rc, is_equal_to(numElements));
    assert_that(BufferLength(&buf), is_equal_to(numElements));
    assert_that(BufferIsFull(&buf), is_false);

    // Spot check certain indices
    i = 0;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 1;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 22;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 49;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 50;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 51;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 81;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 98;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 99;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
}

Ensure(Buffer, add_max_elements) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    const int numElements = BYTE_BUFFER_LEN - 1;
    unsigned char data[numElements];
    int i;
    for (i = 0; i < numElements; i++) {
        data[i] = (unsigned char) i;
    }

    int rc = BufferAddArray(&buf, data, numElements);

    assert_that(rc, is_equal_to(numElements));
    assert_that(BufferLength(&buf), is_equal_to(numElements));
    assert_that(BufferIsFull(&buf), is_true);

    // Spot check certain indices
    i = 0;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 1;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 492;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 5003;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 9999;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 16382;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 16383;
    assert_that(BufferIndex(&buf, i), is_equal_to(0));
}

Ensure(Buffer, add_too_many_elements) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    const int numElements = BYTE_BUFFER_LEN + 2000;
    unsigned char data[numElements];
    int i;
    for (i = 0; i < numElements; i++) {
        data[i] = (unsigned char) i;
    }

    int rc = BufferAddArray(&buf, data, numElements);

    assert_that(rc, is_equal_to(BYTE_BUFFER_LEN - 1));
    assert_that(BufferLength(&buf), is_equal_to(BYTE_BUFFER_LEN - 1));
    assert_that(BufferIsFull(&buf), is_true);

    // Spot check certain indices
    i = 0;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 1;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 492;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 5003;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 9999;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 16381;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 16382;
    assert_that(BufferIndex(&buf, i), is_equal_to(data[i]));
    i = 16383;
    assert_that(BufferIndex(&buf, i), is_equal_to(0));
    i = 20000;
    assert_that(BufferIndex(&buf, i), is_equal_to(0));
}

Ensure(Buffer, delete_empty_element) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    int rc = BufferRemove(&buf, 1);
    assert_that(rc, is_equal_to(0));
    assert_that(BufferLength(&buf), is_equal_to(0));
    assert_that(BufferIsFull(&buf), is_false);
}

Ensure(Buffer, delete_only_element) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    int val = 68;
    BufferAdd(&buf, val);
    assert_that(BufferLength(&buf), is_equal_to(1));
    int rc = BufferRemove(&buf, 1);
    assert_that(rc, is_equal_to(1));
    assert_that(BufferLength(&buf), is_equal_to(0));
}

Ensure(Buffer, delete_too_many_elements) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    int val = 412;
    BufferAdd(&buf, val);
    assert_that(BufferLength(&buf), is_equal_to(1));
    int rc = BufferRemove(&buf, 100);
    assert_that(rc, is_equal_to(1));
    assert_that(BufferLength(&buf), is_equal_to(0));
}

Ensure(Buffer, add_many_and_delete_many) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);
    const int numElements = 10000;
    unsigned char data[numElements];
    int i;
    for (i = 0; i < numElements; i++) {
        data[i] = (unsigned char) i;
    }

    BufferAddArray(&buf, data, numElements);

    assert_that(BufferLength(&buf), is_equal_to(numElements));

    int numToRemove = 1000;
    int rc = BufferRemove(&buf, numToRemove);
    assert_that(rc, is_equal_to(numToRemove));
    i = numElements - numToRemove;
    assert_that(BufferLength(&buf), is_equal_to(i));
}

Ensure(Buffer, add_many_and_empty_a_lot) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);

    int iter;
    for (iter = 0; iter < 10; iter++) {
        const int numElements = 10000;
        unsigned char data[numElements];
        int i;
        for (i = 0; i < numElements; i++) {
            data[i] = (unsigned char) i;
        }

        BufferAddArray(&buf, data, numElements);

        assert_that(BufferLength(&buf), is_equal_to(numElements));

        int numToRemove = 1000;
        int rc = BufferRemove(&buf, numToRemove);
        assert_that(rc, is_equal_to(numToRemove));
        i = numElements - numToRemove;
        assert_that(BufferLength(&buf), is_equal_to(i));

        BufferEmpty(&buf);
        assert_that(BufferLength(&buf), is_equal_to(0));
    }
}

Ensure(Buffer, complete_wringer) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);

    int iter;
    for (iter = 0; iter < 10000; iter++) {
        const int numElements = 10000-987;
        unsigned char data[numElements];
        int i;
        for (i = 0; i < numElements; i++) {
            data[i] = (unsigned char) i;
        }

        BufferAddArray(&buf, data, numElements);

        int numToRemove = 9000-543;
        int rc = BufferRemove(&buf, numToRemove);
        i = numElements - numToRemove;

        if (BufferLength(&buf) > 15000) {
            BufferRemove(&buf, 14998);
        }

    }
    int len = BufferLength(&buf);
    int rc = BufferRemove(&buf, len);
    assert_that(rc, is_equal_to(len));
    assert_that(BufferLength(&buf), is_equal_to(0));
}

Ensure(Buffer, copies_correctly) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);

    const int numElements = 100; unsigned char data[numElements];
    int i, rc;
    for (i = 0; i < numElements; i++) {
        data[i] = (unsigned char) i;
    }

    // Add and remove a few times to make sure it doesn't have index starting at 0
    BufferAddArray(&buf, data, numElements);
    BufferRemove(&buf, numElements);
    BufferAddArray(&buf, data, numElements);
    BufferRemove(&buf, numElements);
    BufferAddArray(&buf, data, numElements);

    unsigned char copied[numElements];

    rc = BufferCopy(&buf, copied, 0, numElements);

    assert_that(rc, is_equal_to(numElements));

    // Spot check certain indices
    i = 0;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 1;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 49;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 50;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 51;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 98;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 99;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
}

Ensure(Buffer, copies_starting_not_at_beginning) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);

    const int numElements = 200; unsigned char data[numElements];
    int i, rc;
    for (i = 0; i < numElements; i++) {
        data[i] = (unsigned char) i;
    }

    // Add and remove a few times to make sure it doesn't have index starting at 0
    for (i = 0; (i-1)*numElements < BYTE_BUFFER_MAX_LEN; i++) {
        BufferAddArray(&buf, data, numElements);
        BufferRemove(&buf, numElements);
    }
    BufferAddArray(&buf, data, numElements);

    unsigned char copied[numElements];

    rc = BufferCopy(&buf, copied, 0, numElements);

    assert_that(rc, is_equal_to(numElements));

    // Spot check certain indices
    i = 0;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 1;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 32;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 99;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 100;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 101;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 169;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 198;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));
    i = 199;
    assert_that(BufferIndex(&buf, i), is_equal_to(copied[i]));

    // Copy half as many elements
    rc = BufferCopy(&buf, copied, numElements/2, numElements);

    assert_that(rc, is_equal_to(numElements - numElements/2));

    // Spot check certain indices
    i = 0;
    assert_that(BufferIndex(&buf, numElements/2 + i), is_equal_to(copied[i]));
    i = 1;
    assert_that(BufferIndex(&buf, numElements/2 + i), is_equal_to(copied[i]));
    i = 32;
    assert_that(BufferIndex(&buf, numElements/2 + i), is_equal_to(copied[i]));
    i = 49;
    assert_that(BufferIndex(&buf, numElements/2 + i), is_equal_to(copied[i]));
    i = 50;
    assert_that(BufferIndex(&buf, numElements/2 + i), is_equal_to(copied[i]));
    i = 51;
    assert_that(BufferIndex(&buf, numElements/2 + i), is_equal_to(copied[i]));
    i = 98;
    assert_that(BufferIndex(&buf, numElements/2 + i), is_equal_to(copied[i]));
    i = 99;
    assert_that(BufferIndex(&buf, numElements/2 + i), is_equal_to(copied[i]));
}

Ensure(Buffer, catches_invalid_accesses) {
    BYTE_BUFFER buf;
    BufferEmpty(&buf);

    int rc;
    unsigned char data[BYTE_BUFFER_LEN] = {0};

    // Null pointer checks
    rc = BufferAdd(NULL, 0);
    assert_that(rc, is_equal_to(-1));

    rc = BufferAddArray(NULL, NULL, 0);
    assert_that(rc, is_equal_to(-1));

    rc = BufferRemove(NULL, 0);
    assert_that(rc, is_equal_to(0));

    rc = (int)BufferIndex(NULL, 0);
    assert_that(rc, is_equal_to(0));

    rc = BufferEmpty(NULL);
    assert_that(rc, is_equal_to(-1));

    rc = BufferIsFull(NULL);
    assert_that(rc, is_equal_to(-1));

    rc = BufferLength(NULL);
    assert_that(rc, is_equal_to(-1));

    rc = BufferCopy(NULL, NULL, 0, 0);
    assert_that(rc, is_equal_to(-1));

    // Bounds rejection
    rc = BufferAddArray(&buf, data, BYTE_BUFFER_MAX_LEN);
    assert_that(rc, is_equal_to(BYTE_BUFFER_MAX_LEN));
    rc = BufferAdd(&buf, 0);
    assert_that(rc, is_equal_to(0));

    BufferRemove(&buf, 100);
    rc = (int)BufferIndex(&buf, -1);
    assert_that(rc, is_equal_to(0));
    rc = (int)BufferIndex(&buf, BufferLength(&buf));
    assert_that(rc, is_equal_to(0));

    rc = BufferCopy(&buf, data, -1, 0);
    assert_that(rc, is_equal_to(0));
    rc = BufferCopy(&buf, data, 0, -1);
    assert_that(rc, is_equal_to(0));
}

