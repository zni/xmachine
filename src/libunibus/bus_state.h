#ifndef LIBUNIBUS_BUS_STATE_H
#define LIBUNIBUS_BUS_STATE_H

#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

/* Type of data bus request to make. */
typedef enum _data_op {
	R_IN,
	R_INB,
	R_OUT,
	R_OUTB,
	R_BLOCK_IN,
	R_BLOCK_INB,
	R_BLOCK_OUT,
	R_BLOCK_OUTB,
	R_DONE,
	R_NONE
} data_op;

/* (Unnecessary?) struct to hold info on data bus request to make. */
typedef struct _data_xfer_spec {
	data_op op;
	uint32_t addr;
	uint16_t value;
} data_xfer_spec;

/*
 * Shared state between bus processors and main device.
 * Grab state_mutex before using if touching:
 * - is_master
 * - req_master
 * - rel_master
 * - shutdown
 * - l_sock
 * - sock
 * - r_sock
 */
typedef struct _bus_state {
	/* Shared state between buses. */
	pthread_mutex_t state_mutex;
	uint8_t is_master;
	uint8_t req_master;
	uint8_t rel_master;
	uint8_t shutdown;

	char *l_sock;
	char *sock;
	char *r_sock;

	/* Signal that pr bus is initialized and ready. */
	pthread_mutex_t pr_ready_mutex;
	pthread_cond_t cond_pr_ready;

	/* Signal that data bus is initialized and ready. */
	pthread_mutex_t data_ready_mutex;
	pthread_cond_t cond_data_ready;

	/* Signal that we have bus master. */
	pthread_mutex_t pr_master_mutex;
	pthread_cond_t cond_pr_master;

	/* Signal that we are no longer bus master. */
	pthread_mutex_t pr_rel_master_mutex;
	pthread_cond_t cond_pr_rel_master;

	/*
	 * Data bus operations.
	 * - master_op_mutex protects master.
	 * - slave_op_mutex protects slave.
	 */
	pthread_mutex_t master_xfer_mutex;
	data_xfer_spec master;

	pthread_mutex_t slave_xfer_mutex;
	data_xfer_spec slave;

	/*
	 * Data bus conditions.
	 * - grab master_data_mutex
	 *   -> signal/wait cond_master_data
	 *
	 * - grab slave_data_mutex
	 *   -> signal/wait cond_slave_data
	 */
	pthread_mutex_t master_data_mutex;
	pthread_cond_t cond_master_data;

	pthread_mutex_t slave_data_mutex;
	pthread_cond_t cond_slave_data;

	/* Only really valid for the memory device. */
	pthread_mutex_t perma_slave_mutex;
	pthread_cond_t cond_perma_slave;

	/* Function pointer to determine if the requested address
	 * on the data bus applies to the current device.
	 */
	uint8_t (*addr_internal_to_device)(uint32_t);
} bus_state;

#endif
