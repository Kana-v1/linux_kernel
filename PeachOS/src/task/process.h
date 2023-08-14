//
// Created by kana on 8/14/23.
//

#ifndef LINUX_KERNEL_PROCESS_H
#define LINUX_KERNEL_PROCESS_H

#include <stdint.h>
#include "../config.h"
#include "../task/task.h"
#include "../fs/file.h"
#include "../memory/paging/paging.h"

struct Process {
	uint16_t id;
	char filename[PEACHOS_MAX_PATH];

	// the main process task
	struct Task* task;

	// the memory (malloc) allocations of the process
	void* allocations[PEACHOS_MAX_PROCESS_ALLOCATIONS];

	// the physical pointer to the process memory;
	void* ptr;

	// the physical pointer to the stack memory
	void* stack;

	// the size of the data pointer to by "ptr"
	uint32_t size;
};

int process_load_for_slot(const char* filename, struct Process** process, int process_slot);

#endif //LINUX_KERNEL_PROCESS_H
