#include <string.h>

#include "stm32f10x.h"
#include "core_cm3.h"

#include "flash.h"
#include "partitions.h"

partition_t partitions[PARTITIONS_MAX_NUM] = {
	PARTITION_TABLE(EXPAND_AS_PARTITIONS_SIZES_INIT)
};

static prtn_table_t *partition_table;
static prtn_callbacks_t *callbacks;

void partitions_register_callbacks(prtn_callbacks_t *cb) {
	callbacks = cb;
}

void partition_table_init(prtn_table_t *prtns) {
	partition_table = prtns;
}

#if 0
bool partitions_init(uint32_t *addr) {
	int8_t i;
	for (i = 0; i < PARTITIONS_MAX_NUM; i++) {
		/* first partition */
		if (i == 0)
			partitions[i].origin = (uint32_t)addr;
		else
			partitions[i].origin = partitions[i-1].origin + partitions[i-1].size;
	}
	return true;
}
#endif

int partition_copy(partition_id_t dst_id, partition_id_t src_id) {
	int res;
	if (partitions[dst_id].size < partitions[src_id].size)
		return -3;
	partition_erase(dst_id);
	res = flash_write_block(partitions[dst_id].origin, 
				partitions[src_id].origin,
				partitions[src_id].size);
	return res;
}

bool partition_erase(partition_id_t id) {
	bool res = true;
	uint16_t start_page_num = FLASH_PAGE_NUM(partitions[id].origin);
	uint16_t end_page_num = start_page_num + (partitions[id].size / FLASH_PAGE_SIZE);
	uint16_t cur_page_num = start_page_num;
	__disable_irq();
	flash_unlock();
	for (; cur_page_num< end_page_num; cur_page_num++) {
		if (flash_erase_page(FLASH_PAGE_START_ADDR(cur_page_num)) < 0) {
			res = false;
			break;
		}
	}
	flash_lock();
	__enable_irq();
	return res;
}

bool partition_is_empty(partition_id_t id) {
	int i;
	int size = partitions[id].size / sizeof(int);
	for (i = 0; i < size; i++) {
		if (*(unsigned int*)(partitions[id].origin + i * sizeof(int)) != 0xffffffff) {
			return false;
			break;
		}
	}
	return true;
}

int partition_get_origin(partition_id_t id) {
	return partitions[id].origin;
}

int partition_get_size(partition_id_t id) {
	return partitions[id].size;
}
