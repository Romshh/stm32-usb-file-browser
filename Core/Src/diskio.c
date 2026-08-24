#include "ff.h"
#include "diskio.h"
#include "tusb.h"

static volatile bool busy_flag;
bool diskio_callback(uint8_t dev_addr, const tuh_msc_complete_data_t *cb_data){
	busy_flag = false;
	return true;
}
void wait_for_disk_io (void){
	while(busy_flag == true){
		tuh_task();
	}

}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count){
	busy_flag = true;
	tuh_msc_read10(pdrv + 1, 0, buff, (uint32_t)sector, (uint16_t)count, diskio_callback, 0);
	wait_for_disk_io();
	return RES_OK;
}
