#include "ff.h"
#include "diskio.h"
#include "tusb.h"

static volatile bool busy_flag;
static volatile bool io_ok;

bool diskio_callback(uint8_t dev_addr, const tuh_msc_complete_data_t *cb_data){
	if(cb_data->csw->status == MSC_CSW_STATUS_PASSED){
		io_ok = true;
	}
	else{
		io_ok = false;
	}
	busy_flag = false;
	return true;
}
void wait_for_disk_io (uint8_t dev_addr){
	while(busy_flag == true){
		tuh_task();
		if(tuh_msc_mounted(dev_addr) == false){
			io_ok = false;
			busy_flag = false;
		}
	}

}

DSTATUS disk_initialize (BYTE pdrv){
	return disk_status(pdrv);
}

DSTATUS disk_status (BYTE pdrv){
	bool diskio_status;
	diskio_status = tuh_msc_mounted(pdrv+1);
	if(diskio_status == true){
		return 0;
	}
	else{
		return STA_NODISK;
	}
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count){
	bool accepted;
	busy_flag = true;
	accepted = tuh_msc_read10(pdrv + 1, 0, buff, (uint32_t)sector, (uint16_t)count, diskio_callback, 0);
	if(accepted == false){
		busy_flag = false;
		return RES_ERROR;
	}
	wait_for_disk_io(pdrv + 1);
	if(io_ok == false){
		return RES_ERROR;
	}
	return RES_OK;
}

DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count){
	bool accepted;
	busy_flag = true;
	accepted = tuh_msc_write10(pdrv + 1, 0, buff, (uint32_t)sector, (uint16_t)count, diskio_callback, 0);
	if(accepted == false){
		busy_flag = false;
		return RES_ERROR;
	}
	wait_for_disk_io(pdrv + 1);
	if(io_ok == false){
		return RES_ERROR;
	}
	return RES_OK;

}
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff){
	if(cmd == CTRL_SYNC){
		return RES_OK;
	}
	else{
		return RES_PARERR;
	}

}
