#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define TRACK_SIZE 3328
#define SECTOR_SIZE 128
#define BUFFER_SIZE 128
#define RXCS_WRITE_MASK   040137

#define RXCS_ADDR 0777170
#define RXDB_ADDR 0777172

typedef enum _rxcs_flag {
    RXCS_FLAG_GO   = 0000001,
    RXCS_FLAG_FS   = 0000016,
    RXCS_FLAG_US   = 0000020,
    RXCS_FLAG_DONE = 0000040,
    RXCS_FLAG_INT  = 0000100,
    RXCS_FLAG_XFER = 0000200
} rxcs_flag;

typedef enum _disk_op {
    DSK_OP_BEGIN,
    DSK_OP_SECTOR,
    DSK_OP_TRACK,
    DSK_OP_FILL,
    DSK_OP_EMPTY,
    DSK_OP_WRITE_SECTOR,
    DSK_OP_READ_SECTOR,
    DSK_OP_DONE
} disk_op;

typedef enum _disk_func {
    DSK_FUNC_FILL_BUFFER,
    DSK_FUNC_EMPTY_BUFFER,
    DSK_FUNC_WRITE_SECTOR,
    DSK_FUNC_READ_SECTOR,
    DSK_FUNC_NOT_USED,
    DSK_FUNC_READ_STATUS,
    DSK_FUNC_WRITE_DEL,
    DSK_FUNC_READ_ERR,
    DSK_FUNC_IDLE
} disk_func;

typedef struct _disk_state {
	disk_op op; /* was m_state */
	disk_func func;
	uint32_t track;
	uint32_t sector;
	uint16_t rxcs;
	uint16_t rxdb;
	uint32_t buffer_idx;
	uint16_t internal_buffer[BUFFER_SIZE];
	int disk_media;
} disk_state;

#endif

