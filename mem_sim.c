#include "mem_sim.h"


/*
NAME: Shachar Israeli
ID: 203713094
DATE: 18/06/2018

" how Memory works in the computer". works with pages and frames and use the swap to save all the data.
*/

int frameIndex[MEMORY_SIZE / PAGE_SIZE];   // use this array to know which page belong to which frame
int frame_index_fifo = 0;


/*prints the error recieved*/
void error(char *msg)
{
    perror(msg);
}

struct sim_database* init_system(char exe_file_name[], char swap_file_name[] , int text_size, int data_bss_size, int heap_stack_size )  // initialization of the program.
{
    if (exe_file_name == NULL)
    {
        error("exe file doesnt exist"); // we must get the exe file to get the memory
        exit(0);
    }

    struct sim_database *mem = (struct sim_database*) malloc(sizeof(struct sim_database));
    mem->program_fd = open(exe_file_name, O_RDONLY);
    if( mem->program_fd == -1)
    {

        error("can't open executable file");
        clear_system(mem);
        exit(0);
    }
    mem->swapfile_fd  = open(swap_file_name, O_RDWR | O_CREAT , 0777);
    if( mem->swapfile_fd == -1)
    {
        error("can't open swap file");
        clear_system(mem);
    }

    mem->text_size = text_size;  //the code
    mem->data_bss_size = data_bss_size;  //the global variables
    mem->heap_stack_size = heap_stack_size;  //the heap and stack variables

    int i = 0;
    for(; i < text_size / PAGE_SIZE ; i++)
    {
        mem->page_table[i].V = 0;
        mem->page_table[i].D = 0;
        mem->page_table[i].P = 0;
        mem->page_table[i].frame = -1;
    }



    for(; i < NUM_OF_PAGES ; i++)  // in esti its untill (text_size+data_bss_size+heap_stack_size)/PAGE_SIZE) and its equal to NUM_OF_PAGES!!!
    {
        mem->page_table[i].V = 0;
        mem->page_table[i].P = 1;
        mem->page_table[i].D = 0;
        mem->page_table[i].frame = -1;
    }

    for (int j = 0; j < MEMORY_SIZE; j++)
        mem->main_memory[j] = '0';


    for (int i = 0; i < MEMORY_SIZE / PAGE_SIZE; i++)
        frameIndex[i] = -1;

    char charInit={' '};   // will be the initialization of the swap file

    for(int i = 0; i < (text_size + data_bss_size + heap_stack_size) ; i++)
    {
        lseek(mem->swapfile_fd, i, SEEK_SET);
        write(mem->swapfile_fd, &charInit, sizeof(char));           // clear the swap
    }

    return mem;
}


char load(struct sim_database *mem_sim, int address)
{
    if (check_vaild_address(mem_sim, address) == 0)     // if the adress is not valid    
        return '\0';

    int offset = address % PAGE_SIZE;
    int page = address / PAGE_SIZE;

    if(mem_sim->page_table[page].V == 1)
    {
        int placed = mem_sim->page_table[page].frame * PAGE_SIZE + offset;     
        return mem_sim->main_memory[placed];   //return the char was asked from the physical memory
    }

                                             // now we know that from here V == 0
    else if(mem_sim->page_table[page].P == 0)
    {
        char *file_input;
        char tempCopy[FRAME_SIZE];         // create arr to copy there the data
        getDataFromExetuable(mem_sim, page,tempCopy);
        file_input = tempCopy;

        if(CheckPlaceAvailable(frame_index_fifo) != NO_PLACE_AVAILABLE) //the start index of the frame available in main_mem
            insert_to_memory(mem_sim, file_input, page);

        else    //the physical memory is full, need to check if need to move the block to swap or override it
        {
            int oldIndex = frameIndex[frame_index_fifo];  //oldindex = the old page
            if(mem_sim->page_table[oldIndex].D == 0)
            {
                clear_v_f(mem_sim);
                insert_to_memory(mem_sim, file_input, page);
            }
            else // D = 1
            {
                move_data_to_swap(mem_sim, oldIndex);
                clear_v_f(mem_sim);                       // clear the page table V and F
                insert_to_memory(mem_sim, file_input, page);

            }
        }

        return return_char(mem_sim, offset, page);

    }
    else  // P == 1
    {
        if (mem_sim->page_table[page].D == 0)
        {
            char *ptrB;

            if (address > mem_sim->text_size + mem_sim->data_bss_size )  //it's in the heap_stack place, create new frame
            {
                char block_s_h[FRAME_SIZE] = {'\0'};
                ptrB = block_s_h;
            }
            else  //it's in the data_bss place, need to read from the exec_file
            {
                char tempCopy[FRAME_SIZE];
                getDataFromExetuable(mem_sim, page,tempCopy);
                ptrB = tempCopy;

            }

            if (CheckPlaceAvailable(frame_index_fifo) != NO_PLACE_AVAILABLE) //the start index of the frame available in main_mem
                insert_to_memory(mem_sim, ptrB, page);

            else //the physical memory is full, need to check if move to swap or override
            {
                int oldIndex = frameIndex[frame_index_fifo]; //the old page
                if (mem_sim->page_table[oldIndex].D == 1)
                    move_data_to_swap(mem_sim, oldIndex);

                clear_v_f(mem_sim);
                insert_to_memory(mem_sim, ptrB, page);
                //suppose to return '\0'

            }

            return return_char(mem_sim, offset, page);


        }
        else     // D == 1, need to read new page from swap
        {
            int oldIndex = frameIndex[frame_index_fifo]; //the old page
            if (mem_sim->page_table[oldIndex].D == 1)
                move_data_to_swap(mem_sim, oldIndex);

            char *dataFromSwap = read_from_swap(mem_sim, page);     // take the data from spawn and after this insert to the main memory
            clear_v_f(mem_sim);
            insert_to_memory(mem_sim, dataFromSwap, page);
            return return_char(mem_sim, offset, page);
        }
    }
}


void store(struct sim_database *mem_sim, int address, char value)
{
    if (check_vaild_address(mem_sim, address) == 0)
        return;

    int offset = address % PAGE_SIZE;
    int page = address / PAGE_SIZE;


    if (address < mem_sim->text_size)
    {
        error("can't change text");
        return;
    }

    if(mem_sim->page_table[page].V == 1)
    {
        int location = (mem_sim->page_table[page].frame * PAGE_SIZE) + offset;
        mem_sim->main_memory[location] = value;   //put the char "value" in the physical memory
        mem_sim->page_table[page].D = 1;
    }

        /*from here  V == 0*/
    else if(mem_sim->page_table[page].P == 0)
    {
        char tempCopy[FRAME_SIZE];
        getDataFromExetuable(mem_sim, page,tempCopy);
        char *file_Ptr = tempCopy;

        if(CheckPlaceAvailable(frame_index_fifo) != NO_PLACE_AVAILABLE) //the start index of the frame available in main_mem
        {
            insert_to_memory(mem_sim, file_Ptr, page);

            replace_char(mem_sim, offset, page, value);
        }

        else    //the physical memory is full, need to check if need to move the block to swap or override it
        {
            int oldIndex = frameIndex[frame_index_fifo];  //index = the page
            if(mem_sim->page_table[oldIndex].D == 0)
            {
                clear_v_f(mem_sim);
                insert_to_memory(mem_sim, file_Ptr, page);
                replace_char(mem_sim, offset, page, value);
            }
            else // D = 1
            {
                move_data_to_swap(mem_sim, oldIndex);
                clear_v_f(mem_sim);
                insert_to_memory(mem_sim, file_Ptr, page);
                replace_char(mem_sim, offset, page, value);
            }

        }
    }
    else  // P = 1
    {
        if (mem_sim->page_table[page].D == 0)
        {
            char *ptrB;

            if (address > mem_sim->text_size + mem_sim->data_bss_size )  //it's in the heap_stack place, create new frame
            {
                char block_s_h[FRAME_SIZE] = {'\0'};       // the char of the init in the heap stack
                ptrB = block_s_h;
            }
            else  //it's in the data_bss place, need to read from the exec_file
            {
                char tempCopy[FRAME_SIZE];
                getDataFromExetuable(mem_sim, page,tempCopy);
                ptrB = tempCopy;

            }
            if (CheckPlaceAvailable(frame_index_fifo) != NO_PLACE_AVAILABLE) //the start index of the frame available in main_mem
            {
                insert_to_memory(mem_sim, ptrB, page);
                replace_char(mem_sim, offset, page, value);
            }
            else  //the physical memory is full, need to check if move to swap or override
            {
                int oldIndex = frameIndex[frame_index_fifo];  //oldindex = the old page

                /*need to check if need to move the frame to the swap*/
                if (mem_sim->page_table[oldIndex].D == 1)  //we know that we need to move to swap
                    move_data_to_swap(mem_sim, oldIndex);
                clear_v_f(mem_sim);
                insert_to_memory(mem_sim, ptrB, page);
                replace_char(mem_sim, offset, page, value);
            }

        }
        else     // D == 1, need to read new page from swap
        {
            int index = frameIndex[frame_index_fifo]; //the old page
            if (mem_sim->page_table[index].D == 1)
                move_data_to_swap(mem_sim, index);
            char * data_from_swap = read_from_swap(mem_sim, page);
            clear_v_f(mem_sim);
            insert_to_memory(mem_sim, data_from_swap, page);
            replace_char(mem_sim, offset, page, value);
        }
    }
}



void insert_to_memory(struct sim_database *mem_sim, char *tempCopy, int page)
{
    for (int i = 0; i < FRAME_SIZE; i++)
        mem_sim->main_memory[(FRAME_SIZE * frame_index_fifo) + i] = tempCopy[i];

    frameIndex[frame_index_fifo] = page;   //update the frame_available table
            
    mem_sim->page_table[frameIndex[frame_index_fifo]].V = 1; //update the Valid char in the index needed in page table
    mem_sim->page_table[frameIndex[frame_index_fifo]].frame = frame_index_fifo;  //update the frame in the index needed in page table
    frame_index_fifo++;

    if (frame_index_fifo == MEMORY_SIZE/FRAME_SIZE)   // here we do the FIFO of the main memory, if we got to index 4 go back to index 0
        frame_index_fifo = 0;
}



void clear_system(struct sim_database* mem_sim)
{
    close(mem_sim->program_fd);
    close(mem_sim->swapfile_fd);
    free(mem_sim);
}


void getDataFromExetuable(struct sim_database *mem_sim, int page,char temp[])
{
    //char *blockFromFile = (char*)malloc(sizeof(char*)*FRAME_SIZE);
    //assert(blockFromFile != NULL);

    if(lseek(mem_sim->program_fd, page * PAGE_SIZE , SEEK_SET) == -1)   //move the pointer in the file to the place needed
        error("failed doing lseek()");
    ssize_t check = read(mem_sim->program_fd, temp, sizeof(char) * PAGE_SIZE);   //read 5 chars (when the first one is what we want)
    if(check < 0)
        error("can't open file");

}



void move_data_to_swap(struct sim_database *mem_sim, int page)
{

    char tempCopy[FRAME_SIZE];
    for (int i = 0; i < FRAME_SIZE; i++)
    {
        tempCopy[i] = mem_sim->main_memory[frame_index_fifo * FRAME_SIZE + i];
    }

    if(lseek(mem_sim->swapfile_fd, page * PAGE_SIZE, SEEK_SET) == -1)
        error("failed doing lseek()");
    if(write(mem_sim->swapfile_fd, tempCopy, sizeof(char)*FRAME_SIZE) == -1) //read 5 chars (when the first one is what we want)
        error("can't write to file");
    mem_sim->page_table[frameIndex[frame_index_fifo]].D = 1;

}



char return_char(struct sim_database *mem_sim, int offset, int page)
{
    int location = mem_sim->page_table[page].frame * PAGE_SIZE + offset;
    return mem_sim->main_memory[location];   //return the char was asked from the physical memory
}


void clear_v_f(struct sim_database *mem_sim)
{
    mem_sim->page_table[frameIndex[frame_index_fifo]].V = 0;
    mem_sim->page_table[frameIndex[frame_index_fifo]].frame = -1;
}


void replace_char(struct sim_database *mem_sim, int offset, int page, char value)
{
    int location = mem_sim->page_table[page].frame * PAGE_SIZE + offset;
    mem_sim->main_memory[location] = value;
    mem_sim->page_table[page].D = 1;
}


int CheckPlaceAvailable(int index_to_add)
{
    if(frameIndex[index_to_add] == -1)
        return 1;
    return NO_PLACE_AVAILABLE;
}

char* read_from_swap(struct sim_database *mem_sim, int page)
{
    char tempCopy[FRAME_SIZE];

    if(lseek(mem_sim->swapfile_fd, page * PAGE_SIZE, SEEK_SET) == -1)   //TODO check if right offset in lseek
        error("failed doing lseek()");
    if(read(mem_sim->swapfile_fd, tempCopy, sizeof(char) * FRAME_SIZE) == -1) //TODO- not working!!! blockFromFile stays empty  read 5 chars (when the first one is what we want)
        error("can't read from file");
    char *ptr = tempCopy;
    return ptr;
}


int check_vaild_address(struct sim_database *mem_sim ,int address)
{
    int max_address = mem_sim->text_size + mem_sim->data_bss_size + mem_sim->heap_stack_size;
    if (address < 0 || address >= max_address)
    {
        error("Invalid address");
        return 0;
    }
    return 1;
}


/************************************************************************************************************************/

/**************************************************************************************/
void print_memory(struct sim_database* mem_sim)
{
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++)
    {
       printf("[%c]\n", mem_sim->main_memory[i]);
    }
}
/************************************************************************************/
void print_swap(struct sim_database* mem_sim)
{
    char str[PAGE_SIZE];
    int i;
    printf("\n Swap memory\n");
    lseek(mem_sim->swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(mem_sim->swapfile_fd, str, PAGE_SIZE) == PAGE_SIZE)
    {
        for(i = 0; i < PAGE_SIZE; i++)
        {
            printf("[%c]\t", str[i]);
        }
        printf("\n");
    }
}
/***************************************************************************************/
void print_page_table(struct sim_database* mem_sim)
{
    int i;
    printf("\n page table \n");
    printf("Valid\t Dirty\t Permission \t Frame\n");
    for(i = 0; i < NUM_OF_PAGES; i++)
    {
       printf("[%d]\t[%d]\t[%d]\t[%d]\n", mem_sim->page_table[i].V,
        mem_sim->page_table[i].D,
        mem_sim->page_table[i].P, mem_sim->page_table[i].frame);
    }
}

