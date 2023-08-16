//
// Created by kana on 8/11/23.
//

#ifndef LINUX_KERNEL_TASK_H
#define LINUX_KERNEL_TASK_H

#include "../config.h"
#include "../memory/paging/paging.h"
#include "../memory/memory.h"
#include "../status.h"
#include "../memory/heap/kheap.h"

// CPU registers
struct Registers {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	uint32_t ip;    // program counter
	uint32_t cs;
	uint32_t flags;
	uint32_t esp;
	uint32_t ss;
};

struct Process;

struct Task {
	// the page directory of the task
	struct Paging4GbChunk* page_directory;

	// the registers of the task when the task is not running
	struct Registers registers;

	// the next task in the linked list
	struct Task* next;

	// the process of the task
	struct Process* process;

	// the previous task in the linked list
	struct Task* prev;
};

struct Task* task_new(struct Process* process);
struct Task* task_current();
struct Task* task_get_next();

void task_run_first_ever_task();

void task_return(struct Registers* regs); // this function drops us into the user land
void restore_general_purpose_registers(struct Registers* regs);
void user_registers();


int task_free(struct Task* task);

int task_switch(struct Task* task);

#endif //LINUX_KERNEL_TASK_H
