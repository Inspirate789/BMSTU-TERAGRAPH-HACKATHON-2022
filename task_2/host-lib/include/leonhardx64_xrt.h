/*
 * leonhardx64.h
 *
 * host library (XRT runtime version)
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#ifndef LEONHARDx64_H_
#define LEONHARDx64_H_

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "gpc_io_xrt_host.h"
#include "gpc_defs.h"

//====================================================
// Класс для управления картой ускорителя Leonhard x64
//====================================================

class leonhardx64
{
private:
	xrt::device *xrt_device;
	xrt::uuid *xrt_uuid;
	xrt::bo *global_memory[LNH_GROUPS_COUNT];                          	  // device memory used for a gpc IO
	xrt::bo *ddr4_memory[LNH_GROUPS_COUNT];                               // device memory used for a structures
    void device_reset();
public:
  //GPC instances
  gpc_io *gpc[LNH_GROUPS_COUNT][LNH_MAX_CORES_IN_GROUP]; 		 // global memory queue
  // Constructor
  leonhardx64(
			  unsigned int 		dev_index,
			  char 				*xclbin
			  );
  // Destructor
  ~leonhardx64();
  // Reset device
  void load_sw_kernel(char *gpnbin, unsigned int group, unsigned int core);

protected:
};

#endif
