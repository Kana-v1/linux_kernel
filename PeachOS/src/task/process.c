//
// Created by kana on 8/14/23.
//

#include "process.h"

struct Process* current_process = 0; // the current process that is running

static struct Process* processes[PEACHOS_MAX_PROCESSES] = {}; // all processes

static void process_init(struct Process* process) {
	memset(process, 0, sizeof(struct Process));
}

struct Process* process_current() {
	return current_process;
}

struct Process* process_get(int process_id) {
	if (process_id < 0 || process_id >= PEACHOS_MAX_PROCESSES) {
		return NULL;
	}

	return processes[process_id];
}


static int process_load_binary(const char* filename, struct Process* process) {
	int fd = fopen(filename, "r");
	if (!fd) {
		return EIO;
	}

	struct FileStat stat;
	int res = fstat(fd, &stat);
	if (res != PEACHOS_ALL_OK) {
		return res;
	}

	void* program_data_ptr = kzalloc(stat.file_size);
	if (!program_data_ptr) {
		return ENOMEM;
	}

	if (fread(program_data_ptr, stat.file_size, 1, fd) != 1) {
		return EIO;
	}

	process->ptr = program_data_ptr;
	process->size = stat.file_size;

	fclose(fd);
	return PEACHOS_ALL_OK;
}

static int process_load_data(const char* filename, struct Process* process) {
	return process_load_binary(filename, process);
}

int process_map_binary(struct Process* process) {
	return paging_map_to(
			process->task->page_directory->directory_entry,
			(void*)PEACHOS_PROGRAM_VIRTUAL_ADDRESS,
			process->ptr,
			paging_align_address(process->ptr + process->size),
			PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE
	);
}

int process_map_memory(struct Process* process) {
	return process_map_binary(process);
}

int process_get_free_slot() {
	for (int i = 0; i < PEACHOS_MAX_PROCESSES; i++) {
		if (processes[i] == 0) {
			return i;
		}
	}

	return EISTKN;
}

int process_load(const char* filename, struct Process** process) {
	int process_slot = process_get_free_slot();
	if (process_slot < 0) {
		return ENOMEM;
	}

	return process_load_for_slot(filename, process, process_slot);
}

int process_load_for_slot(const char* filename, struct Process** process, int process_slot) {
	int res = 0;
	struct Task* task = 0;
	struct Process* _process;
	void* program_stack_ptr = 0;
	if (process_get(process_slot) != 0) {
		res = EISTKN;
		goto out;
	}

	_process = kzalloc(sizeof(struct Process));
	if (!_process) {
		res = ENOMEM;
		goto out;
	}

	process_init(_process);
	res = process_load_data(filename, _process);
	if (res < 0) {
		goto out;
	}

	program_stack_ptr = kzalloc(PEACHOS_USER_PROGRAM_STACK_SIZE);
	if (!program_stack_ptr) {
		res = ENOMEM;
		goto out;
	}

	strncpy(_process->filename, filename, sizeof(_process->filename));
	_process->stack = program_stack_ptr;
	_process->id = process_slot;

	// create a task
	task = task_new(_process);
	if (ERROR_I(task) == 0) {
		res = ERROR_I(task);
		goto out;
	}

	_process->task = task;
	res = process_map_memory(_process);
	if (res < 0) {
		goto out;
	}

	*process = _process;

	// add the process to the array
	processes[process_slot] = _process;


out:
	if (IS_ERROR(res)) {
		if (_process && _process->task) {
			task_free(_process->task);
		}

		// TODO: free the process data
	}

	return res;
}