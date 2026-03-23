#include <stdio.h>
#include <stdlib.h>// Global variables
int global_init = 100;      // .data
int global_uninit;          // .bssconst 
int global_const = 200; // .rodatavoid 
int demo_function()
{
    int local_var = 10;  // stack    
    int *heap_var = (int*)malloc(sizeof(int)); // heap
    *heap_var = 50;    printf("\nInside demo_function:\n");
    printf("Address of local_var (stack): %p\n", &local_var);
    printf("Address of heap_var (heap):  %p\n", heap_var);    free(heap_var);
}
int main()
{
    static int static_var = 300; // .data    
    printf("Process Memory Mapping Demo\n\n");    
    printf("Address of main (code/text): %p\n", main);
    printf("Address of demo_function (text): %p\n", demo_function);    
    printf("\nGlobal / Static Segments:\n");
    printf("global_init (.data):   %p\n", &global_init);
    printf("global_uninit (.bss):  %p\n", &global_uninit);
    printf("global_const (.rodata):%p\n", &global_const);
    printf("static_var (.data):    %p\n", &static_var);    
    demo_function();    
    return 0;
}
