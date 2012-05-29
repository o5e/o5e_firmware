
/****************************************************************
* WARNING this file is automatically generated DO NOT EDIT IT!   
*                                                                
* This file provides an interface between eTPU code and CPU      
* code. All references to the HD function should be made with    
* information in this file. This allows only symbolic            
* information to be referenced which allows the eTPU code to be  
* optimized without effecting the CPU code.                      
*****************************************************************/
#ifndef _ETPU_HD_AUTO_H_
#define _ETPU_HD_AUTO_H_

/****************************************************************
* Function Configuration Information.
****************************************************************/
#define FS_ETPU_HD_FUNCTION_NUMBER        11 
#define FS_ETPU_HD_TABLE_SELECT           1 
#define FS_ETPU_HD_NUM_PARMS              0x0040 

/****************************************************************
* Host Service Request Definitions.
****************************************************************/
#define FS_ETPU_HD_HSR_INIT               7 
#define FS_ETPU_HD_HSR_COMMUT_ON          5 
#define FS_ETPU_HD_HSR_COMMUT_OFF         3 

/****************************************************************
* Parameter Definitions.
****************************************************************/
#define FS_ETPU_HD_SECTORMASKLH_OFFSET     0x0001
#define FS_ETPU_HD_SECTORMASKHL_OFFSET     0x0005
#define FS_ETPU_HD_DIRECTIONMASKLH_OFFSET  0x0009
#define FS_ETPU_HD_DIRECTIONMASKHL_OFFSET  0x000D
#define FS_ETPU_HD_COMMCMDLHI0_OFFSET      0x0010
#define FS_ETPU_HD_COMMCMDLHI1_OFFSET      0x0014
#define FS_ETPU_HD_COMMCMDLHD0_OFFSET      0x0018
#define FS_ETPU_HD_COMMCMDLHD1_OFFSET      0x001C
#define FS_ETPU_HD_COMMCMDHLI0_OFFSET      0x0020
#define FS_ETPU_HD_COMMCMDHLI1_OFFSET      0x0024
#define FS_ETPU_HD_COMMCMDHLD0_OFFSET      0x0028
#define FS_ETPU_HD_COMMCMDHLD1_OFFSET      0x002C
#define FS_ETPU_HD_COMMONSTR_OFFSET        0x0039
#define FS_ETPU_HD_PWMMCHAN_OFFSET         0x0000

/****************************************************************
* Common Structure Size and Offset Definitions.
****************************************************************/
#define FS_ETPU_HD_COMMON_STR_SIZE               0x14

#define FS_ETPU_HD_COMMONSTR_DIRECTION_OFFSET    0x00
#define FS_ETPU_HD_COMMONSTR_SECTOR_OFFSET       0x01
#define FS_ETPU_HD_COMMONSTR_TIMER_OFFSET        0x04
#define FS_ETPU_HD_COMMONSTR_REVPERIOD_OFFSET    0x05
#define FS_ETPU_HD_COMMONSTR_LASTEDGE_OFFSET     0x09
#define FS_ETPU_HD_COMMONSTR_SECTORPERIOD_OFFSET 0x0D
#define FS_ETPU_HD_COMMONSTR_REVCOUNTER_OFFSET   0x11

#endif