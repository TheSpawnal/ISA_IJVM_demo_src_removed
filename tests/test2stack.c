#include <stdio.h>
#include <string.h>
#include "../include/ijvm.h"
#include "testutil.h"

void test_simple_bipush()
{
    int res = init_ijvm("files/task2/TestBipush1.ijvm");
    assert(res != -1);

    step();
    assert(tos() == 42); 
    destroy_ijvm();
}

void test_signed_bipush()
{
    int res = init_ijvm("files/task2/TestBipush2.ijvm");
    assert(res != -1);

    step();
    assert(tos() == -42); 
    destroy_ijvm();
}

void test_simple_iadd()
{
    int res = init_ijvm("files/task2/TestIadd1.ijvm");
    assert(res != -1);

    step();
    step();
    step();
    assert(tos() == 60); 
}

void test_dup()
{
    int res = init_ijvm("files/task2/TestDup.ijvm");
    assert(res != -1);

    step();
    step();
    step();
    step();
    step();
    assert(tos() == 60); 
    destroy_ijvm();
}

void test_signed_iadd()
{
    int res = init_ijvm("files/task2/TestIadd2.ijvm");
    assert(res != -1);

    step();
    step();
    step();
    assert(tos() == -60); 
    destroy_ijvm();
}


void test_simple_isub()
{
    int res = init_ijvm("files/task2/TestIsub1.ijvm");
    assert(res != -1);

    step();
    step();
    step();
    assert(tos() == -10);
    destroy_ijvm();
}

void test_signed_isub()
{
    int res = init_ijvm("files/task2/TestIsub2.ijvm");
    assert(res != -1);

    step();
    step();
    step();
    assert(tos() == 10); 
    destroy_ijvm();
}

void test_simple_iand()
{
    int res = init_ijvm("files/task2/TestIAND1.ijvm");
    assert(res != -1);

    step();
    step();
    step();
    step();
    step();
    assert(tos() == 1); 
    destroy_ijvm();
}

void test_simple_ior()
{
    int res = init_ijvm("files/task2/TestIOR1.ijvm");
    assert(res != -1);

    step();
    step();
    step();
    step();
    step();
    assert(tos() == 127); 
    destroy_ijvm();
}

void test_swap()
{
    int res = init_ijvm("files/task2/TestSwap1.ijvm");
    assert(res != -1);

    step();
    assert(tos() == 10);
    step();
    assert(tos() == 20);
    step();
    assert(tos() == 10);
    step();
    assert(tos() == 20);
    destroy_ijvm();
}

void test_simple_stack_operations()
{
    int res = init_ijvm("files/task2/TestPop1.ijvm");
    assert(res != -1);

    steps(3);
    assert(tos() == 10);
    steps(3);
    assert(tos() == 50);
    step();
    assert(tos() == 10);
    destroy_ijvm();
}

void test_in_out()
{
    int res = init_ijvm("files/task2/TestInOut.ijvm");
    assert(res != -1);

    FILE * input, * output;
    input  = fopen("tmp_input", "w+");
    output = fopen("tmp_output", "w+");
    fprintf(input, "%s", "ABCDE");
    rewind(input);

    set_input(input);
    set_output(output);

    run();

    char buf[128] = {0};
    rewind(output);
    fread(buf, 1, 5, output);
    buf[5] = 0;
    // in case something goes wrong,
    // you can print the 5 bytes of output with this
    // printf(stderr,"Output inout test: %s\n", buf)
    assert(strncmp(buf, "EDCBA", 5) == 0);
    destroy_ijvm();
}


int main()
{
    RUN_TEST(test_simple_bipush);
    RUN_TEST(test_signed_bipush);
    RUN_TEST(test_simple_iadd);
    RUN_TEST(test_dup);
    RUN_TEST(test_signed_iadd);
    RUN_TEST(test_simple_isub);
    RUN_TEST(test_signed_isub);
    RUN_TEST(test_simple_iand);
    RUN_TEST(test_simple_ior);
    RUN_TEST(test_swap);
    RUN_TEST(test_simple_stack_operations);
    RUN_TEST(test_in_out);
    return END_TEST();
}
