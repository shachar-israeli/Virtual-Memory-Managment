//
// Created by shachar on 6/11/18.
//

#include "mem_sim.h"

int main()
{
	char val;
    struct sim_database * mem_sim = init_system ("exec_file.txt", "swap.txt" ,25, 50, 50);
   val = load (mem_sim , 64);
val = load (mem_sim , 66);
val = load (mem_sim , 2);
store(mem_sim , 98,'X');
val = load (mem_sim ,16);
val = load (mem_sim ,70);
store(mem_sim ,32,'Y');
store (mem_sim ,15,'Z');
val = load (mem_sim ,23);
 print_memory(mem_sim); 
 print_swap(mem_sim);
  clear_system(mem_sim);
    
    return 0;
}



