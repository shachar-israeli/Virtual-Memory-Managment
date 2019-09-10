

This program simulates access to the physical memory while using paging mechanism

how to install the program:

open linux terminal, navigate to the folder containing ex5
using the "cd" command (confirm it by using ls command)
incase you have makefile, type make and the program will
automaticily be compiled, if you don't, type gcc -Wall mem_sim.c main.c -o ex5

and your program will automaticily be compiled

to activate program:
open linux terminal, navigate to ex5 executeable file
location using "cd" command (confirm it using ls command) and type
./ex5

there are 2 structs - sim_database, page_descriptor, array of free frame in physical address and frame counter


#### how to install ####

open linux terminal, navigate to the folder containing ex5
using the "cd" command (confirm it by using ls command)

incase you have makefile, type make and the program will
automaticily be compiled, if you don't, type gcc -Wall ex3q1.c -o ex3q1
and your program will automaticily be compiled

make sure there is exec file that contain the "data" that you want to save

------------------program functions -------------------------------


* incase of memory allocation failure, program will exit with
message "Memory allocation failed"


struct sim_database* init_system(char exe_file_name[], char swap_file_name[] , int text_size, int data_bss_size, int heap_stack_size);
input: the names of execute and swap files, the number of text, data bss and heap_stack in the simulator
output: returns a pointer to sim_database 


struct sim_database* init_system(char exe_file_name[], char swap_file_name[] , int text_size, int data_bss_size, int heap_stack_size );

char load(struct sim_database *mem_sim, int address);

void store(struct sim_database *mem_sim, int address, char value);

void replace_char(struct sim_database *mem_sim, int offset, int page, char value);

void insert_to_memory(struct sim_database *mem_sim, char *copy, int page);

char* read_from_swap(struct sim_database *mem_sim, int page);

void clear_v_f(struct sim_database *mem_sim);

int check_vaild_address(struct sim_database *mem_sim ,int address);

void clear_system(struct sim_database* mem_sim);

void getDataFromExetuable(struct sim_database *mem_sim, int page,char temp[]);

int CheckPlaceAvailable(int index_to_add);

void move_data_to_swap(struct sim_database *mem_sim, int page);

char return_char(struct sim_database *mem_sim, int offset, int page);

// ##################################//


void print_memory(struct sim_database* mem_sim);
void print_swap(struct sim_database* mem_sim);
void print_page_table(struct sim_database* mem_sim);


int main() command:
input: none
output: This program simulates access to the physical memory while using paging mechanism
that has been fully tested with valgrind to test for memory
leak, no leak was found.


##########################




