/*
 * gpc_host.h
 *
 * host library (XRT runtime version)
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#ifndef GPC_H_
#define GPC_H_

#include <cstdlib>
#include <mutex>
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_bo.h"
#include "ert.h"
//#include "gpc_handlers.h"
#include "gpc_defs.h"

//=============================================
// Класс для управления Graph Processing Core
//=============================================

class gpc
{
protected:
  // Descriptors
  xrt::device *device;
  xrt::uuid *uuid;
  xrt::run run;
  // Core defines
  unsigned int group;
  unsigned int core;
  unsigned int load_file_to_memory(const char *filename, char **result);

public:
  // Constructor
  gpc(xrt::device *xrt_device,xrt::uuid *xrt_uuid,unsigned int group_number,unsigned int core_number);
  // Destructor
  ~gpc();
  // Kernel
  xrt::kernel *kernel;
  // Map to IO memory
  char* global_memory_map;
  char* external_memory_map;
  // Basic methods
  void start_sync(const unsigned int event_handler);
  void start_async(const unsigned int event_handler);
  void finish();

protected:
  // Buffers
  xrt::bo *global_memory_pointer;
  xrt::bo *external_memory_pointer;
  xrt::bo *ddr_memory_pointer;
  void load_sw_kernel(char *gpnbin);
};

#endif
