//
// Created by kana on 8/11/23.
//
#include "task.h"
#include "../kernel.h"

// The current task that is running
struct Task* current_task = 0;

// Task linked list
struct Task* task_tail = 0;
struct Task* task_head = 0;

int task_init(struct Task* task, struct Process* process);

struct Task* task_current() {
	return current_task;
}

struct Task* task_new(struct Process* process) {
	int res = PEACHOS_ALL_OK;
	struct Task* task = kzalloc(sizeof(struct Task));
	if (!task)	{
		res = -ENOMEM;
		goto out;
	}

	res = task_init(task, process);
	if (res != PEACHOS_ALL_OK) {
		goto out;
	}

	if (task_head == 0)	{
		task_head = task;
		task_tail = task;
		current_task = task;
		goto out;
	}

	task_tail->next = task;
	task->prev = task_tail;
	task_tail = task;

out:
	if (IS_ERROR(res))	{
		task_free(task);
		return ERROR(res);
	}

	return task;
}

struct Task* task_get_next() {
	if (!current_task->next) {
		return task_head;
	}

	return current_task->next;
}

static void task_list_remove(struct Task* task){
	if (task->prev)	{
		task->prev->next = task->next;
	}

	if (task == task_head)	{
		task_head = task->next;
	}

	if (task == task_tail)	{
		task_tail = task->prev;
	}

	if (task == current_task)	{
		current_task = task_get_next();
	}
}

int task_free(struct Task* task){
	paging_free_4gb(task->page_directory);
	task_list_remove(task);

	// Finally free the task data
	kfree(task);
	return 0;
}

int task_page() {
	user_registers();
	task_switch(current_task);
	return PEACHOS_ALL_OK;
}

int task_switch(struct Task* task) {
	current_task = task;
	paging_switch(task->page_directory->directory_entry);
	return PEACHOS_ALL_OK;
}

void task_run_first_ever_task() {
	if (!current_task) {
		panic("task_run_first_ever_task(): no current task exists!\n");
	}

	task_switch(task_head);
	task_return(&task_head->registers);
}

int task_init(struct Task* task, struct Process* process){
	memset(task, 0, sizeof(struct Task));
	// Map the entire 4GB address space to its self
	task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
	if (!task->page_directory)	{
		return -EIO;
	}

	task->registers.ip = PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
	task->registers.ss = USER_DATA_SEGMENT;
	task->registers.esp = PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;
	task->registers.cs = USER_CODE_SEGMENT;

	task->process = process;

	return 0;
}