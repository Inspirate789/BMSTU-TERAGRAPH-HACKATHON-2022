/*
 * gpc_defs.h
 *
 * host library
 *
 *  Created on: April 23, 2021
 *      Author: A.Popov
 */

#ifndef CORE_DEFS_H_
#define CORE_DEFS_H_

//Macros for multicore
#define  			__foreach_group(group) \
						for (int (group)=0; (group)<LNH_GROUPS_COUNT; (group)++)
#define 			__foreach_core(group,core) \
						for (int (group)=0; (group)<LNH_GROUPS_COUNT; (group)++) \
						for (int (core)=LNH_CORES_LOW[(group)]; (core)<=LNH_CORES_HIGH[(group)]; (core)++)

#define 			LNH_MAX_CORES_IN_GROUP	6

//Cores soft specification
const struct {
	char* KERNEL_NAME[4][6] = {
			{		"gpc_singlecore_cacheu2a_hs_v2:{lnh_g0_dp0_ip6_ir0}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g0_dp1_ip6_ir1}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g0_dp2_ip6_ir2}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g0_dp3_ip6_ir3}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g0_dp4_ip7_ir0}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g0_dp5_ip7_ir1}"
			},
			{		"gpc_singlecore_cacheu2a_hs_v2:{lnh_g2_dp0_ip6_ir0}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g2_dp1_ip6_ir1}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g2_dp2_ip6_ir2}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g2_dp3_ip6_ir3}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g2_dp4_ip7_ir0}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g2_dp5_ip7_ir1}"
			},
			{		"gpc_singlecore_cacheu2a_hs_v2:{lnh_g1_dp0_ip6_ir0}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g1_dp1_ip6_ir1}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g1_dp2_ip6_ir2}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g1_dp3_ip6_ir3}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g1_dp4_ip7_ir0}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g1_dp5_ip7_ir1}"
			},
			{		"gpc_singlecore_cacheu2a_hs_v2:{lnh_g3_dp0_ip6_ir0}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g3_dp1_ip6_ir1}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g3_dp2_ip6_ir2}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g3_dp3_ip6_ir3}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g3_dp4_ip7_ir0}",
					"gpc_singlecore_cacheu2a_hs_v2:{lnh_g3_dp5_ip7_ir1}"
			}
	};
	unsigned int LEONHARD_CONFIG[4][6] = {
			//For control logic = {index_region[1:0],index_partition[2:0],data_partition[2:0]} = {0,6,0} = 8'b00110000
			//Bank 0
			{
					((0<<6)|(6<<3)|(0<<0)), //Index_region = 0, Index_partition = 6, Data_partition = 0
					((1<<6)|(6<<3)|(1<<0)), //Index_region = 1, Index_partition = 6, Data_partition = 1
					((2<<6)|(6<<3)|(2<<0)), //Index_region = 2, Index_partition = 6, Data_partition = 2
					((3<<6)|(6<<3)|(3<<0)), //Index_region = 3, Index_partition = 6, Data_partition = 3
					((0<<6)|(7<<3)|(4<<0)), //Index_region = 0, Index_partition = 7, Data_partition = 4
					((1<<6)|(7<<3)|(5<<0))  //Index_region = 1, Index_partition = 7, Data_partition = 5
			},
			//Bank 1
			{
					((0<<6)|(6<<3)|(0<<0)), //Index_region = 0, Index_partition = 6, Data_partition = 0
					((1<<6)|(6<<3)|(1<<0)), //Index_region = 1, Index_partition = 6, Data_partition = 1
					((2<<6)|(6<<3)|(2<<0)), //Index_region = 2, Index_partition = 6, Data_partition = 2
					((3<<6)|(6<<3)|(3<<0)), //Index_region = 3, Index_partition = 6, Data_partition = 3
					((0<<6)|(7<<3)|(4<<0)), //Index_region = 0, Index_partition = 7, Data_partition = 4
					((1<<6)|(7<<3)|(5<<0))  //Index_region = 1, Index_partition = 7, Data_partition = 5
			},
			//Bank 2
			{
					((0<<6)|(6<<3)|(0<<0)), //Index_region = 0, Index_partition = 6, Data_partition = 0
					((1<<6)|(6<<3)|(1<<0)), //Index_region = 1, Index_partition = 6, Data_partition = 1
					((2<<6)|(6<<3)|(2<<0)), //Index_region = 2, Index_partition = 6, Data_partition = 2
					((3<<6)|(6<<3)|(3<<0)), //Index_region = 3, Index_partition = 6, Data_partition = 3
					((0<<6)|(7<<3)|(4<<0)), //Index_region = 0, Index_partition = 7, Data_partition = 4
					((1<<6)|(7<<3)|(5<<0))  //Index_region = 1, Index_partition = 7, Data_partition = 5
			},
			//Bank 3
			{
					((0<<6)|(6<<3)|(0<<0)), //Index_region = 0, Index_partition = 6, Data_partition = 0
					((1<<6)|(6<<3)|(1<<0)), //Index_region = 1, Index_partition = 6, Data_partition = 1
					((2<<6)|(6<<3)|(2<<0)), //Index_region = 2, Index_partition = 6, Data_partition = 2
					((3<<6)|(6<<3)|(3<<0)), //Index_region = 3, Index_partition = 6, Data_partition = 3
					((0<<6)|(7<<3)|(4<<0)), //Index_region = 0, Index_partition = 7, Data_partition = 4
					((1<<6)|(7<<3)|(5<<0))  //Index_region = 1, Index_partition = 7, Data_partition = 5
			}
	};




} LNH_CORE_DEFS;

//Define group count for Leonhard x64 version
#define 			LNH_GROUPS_COUNT 			1
const unsigned int 	LNH_CORES_LOW[4] 		= 	{	0,0xF,0xF,0xF	};
const unsigned int 	LNH_CORES_HIGH[4] 		= 	{	0,5,5,5	};
const unsigned int	GPC_RESET_HIGH 			= 	1;
const unsigned int	GPC_RESET_LOW 			= 	0;
#define 			GLOBAL_MEMORY_MAX_LENGTH  	32768
#define				MQ_CREDITS					255

//Define register offsets
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read)
//        bit 7  - auto_restart (Read/Write)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0  - enable ap_done interrupt (Read/Write)
//        bit 1  - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0  - ap_done (COR/TOW)
//        bit 1  - ap_ready (COR/TOW)
//        others - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)
// Standard IO registers
#define			   CONTROL_OFFSET				0x000
#define 				AP_START 				(1<<0)
#define					AP_DONE					(1<<1)
#define					AP_IDLE					(1<<2)
#define					AP_READY				(1<<3)
#define					AUTO_RESTART			(1<<7)
#define			   G_IE_OFFSET					0x004
#define 				G_IE_ENABLE				(1<<0)
#define			   IP_IE_OFFSET					0x008
#define 				AP_DONE_IE_ENABLE		(1<<0)
#define 				AP_READY_IE_ENABLE		(1<<1)
#define			   IP_IS_OFFSET					0x00c
#define 				AP_DONE_IS_STATUS		(1<<0)
#define 				AP_READY_IS_STATUS		(1<<1)
// Custom IP registers
#define			   LNH_CONFIG_OFFSET			0x010
#define			   GPC_RESET_OFFSET				0x018
#define			   GPC_CONFIG_OFFSET			0x020
#define			   DDR4_BUS_PTR_OFFSET			0x028
#define			   GLOBAL_MEMORY_PTR_OFFSET		0x034
#define			   EXTERNAL_MEMORY_PTR_OFFSET	0x040
#define			   MQ_SR						0x04C
#define 				HOST2GPC_MQ_FULL		(1<<0)
#define 				HOST2GPC_MQ_AFULL		(1<<1)
#define 				HOST2GPC_MQ_EMPTY		(1<<2)
#define 				GPC2HOST_MQ_FULL		(1<<3)
#define 				GPC2HOST_MQ_AFULL		(1<<4)
#define 				GPC2HOST_MQ_EMPTY		(1<<5)
#define			   MQ_RST						0x04C
#define			   HOST2GPC_MQ					0x050
#define			   GPC2HOST_MQ					0x054

#endif
