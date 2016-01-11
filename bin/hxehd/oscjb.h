/* @(#)85	1.2  src/htx/usr/lpp/htx/bin/hxehd/oscjb.h, exer_hd, htxubuntu 6/4/04 14:28:05 */

/* oscjb.h                               
 *                                                                             
 * Optical Media common definitions and declarations    
 *                                                                           
 *****************************************************************************/

#ifndef __OSCJB_H
#define __OSCJB_H

#include <sys/ioctl.h>
#ifdef	__HTX_LINUX__
#include <scsi/sg.h>
#include <scsi/scsi.h>
#else
#include <sys/scsi.h>
#endif 

#define ENEEDIES 200
#define EMANINT  201
#define EDOOROPN 202

/* IOCTL command type macros */
#define JBIOC_INVENTORY     _IOWR('L',1,sizeof(jb_Inventory))
#define JBIOC_INQUIRY       _IOWR('L',2,sizeof(jb_Inquiry))
#define JBIOC_MOVE          _IOWR('L',3,sizeof(jb_Move))
#define JBIOC_EXCHANGE      _IOWR('L',4,sizeof(jb_Exchange))
#define JBIOC_PREVENTALLOW  _IOWR('L',5,sizeof(jb_PreventAllow))
#define JBIOC_IES           _IOWR('L',6,sizeof(jb_IOCTL))

/* IOCTL INFO - Command parameter structure */
typedef struct _jb_DevInfo {
   char devtype;
   char flags;
   char devsubtype;
} jb_DevInfo;

/* IOCTL INQUIRY - Command parameter structure */
typedef struct _jb_Inquiry {
   uchar DeviceType;                /* 0x08 = Media Changer           */
   uchar Removeable;                /* 1    = media is removeable     */
   uchar Flipable;                  /* 1    = media flip is supported */
   uchar ProductType[5];
   uchar ProductModel[5];
} jb_Inquiry;

/* IOCTL INVENTORY - Command parameter structure */
typedef struct _jb_TransportElemStat {
   ushort ElementAddress;
   uchar  Exception;               /* 0 = normal state.  1 = abnormal state */
   uchar  Full;                    /* 1 = contains media.  0 = empty        */
} jb_TransportElemStat;

typedef struct _jb_StorageElemStat {
   ushort ElementAddress;          
   uchar  Exception;              /* 0 = normal state.  1 = abnormal state  */
   uchar  Full;                   /* 1 = contains media.  0 = empty         */
   uchar  Access;                 /* 1 = access by trans element is allowed */
} jb_StorageElemStat;

typedef struct _jb_ImportExportElemStat {
   ushort ElementAddress;
   uchar  Exception;               /* 0 = normal state.  1 = abnormal state  */
   uchar  Full;                    /* 1 = contains media.  0 = empty         */
   uchar  Access;                  /* 1 = access by trans element is allowed */
   uchar  OperatorInsert;          /* 1 = media inserted by operator         */
   uchar  ImportEnabled;           /* 1 = import to trans element is allowed */
   uchar  ExportEnabled;           /* 1 = export from trans is enabled       */
} jb_ImportExportElemStat;
 
typedef struct _jb_DataTransferElemStat {
   ushort ElementAddress;          
   uchar  Exception;              /* 0 = normal state.  1 = abnormal state  */
   uchar  Full;                   /* 1 = contains media.  0 = empty         */
   uchar  Access;                 /* 1 = access by trans element is allowed */
} jb_DataTransferElemStat;

typedef struct _jb_Inventory {
   ushort TransportElements;      /* Specify how many element status        */
   ushort StorageElements;        /* descriptor blocks to return. if set    */
   ushort ImportExportElements;   /* to 0, field is updated to show number  */
   ushort DataTransferElements;   /* available.                             */
   ushort FirstTransportElem;     /* Specify the 1st element in the list of */
   ushort FirstStorageElem;       /* elements to return. if set to 0,       */
   ushort FirstImportExportElem;  /* field is updated to show the first     */
   ushort FirstDataTransferElem;  /* element address of the element type.   */
   jb_TransportElemStat    *TransportElemStat;    /* Returned element       */
   jb_StorageElemStat      *StorageElemStat;      /* status descriptor      */
   jb_ImportExportElemStat *ImportExportElemStat; /* blocks.                */
   jb_DataTransferElemStat *DataTransferElemStat; 
} jb_Inventory;

/* IOCTL MOVE - Command parameter structure */
typedef struct _jb_Move {
   ushort TransportElement;
   ushort SourceElement;
   ushort DestinationElement;
   uchar  Invert;                 /* invert media before placing in         */
} jb_Move;

/* IOCTL EXCHANGE - Command parameter structure */
typedef struct _jb_Exchange {
   ushort TransportElement;
   ushort SourceElement;
   ushort DestinationElement1;
   ushort DestinationElement2;
   uchar  Invert1;                /* invert media before placing in         */
   uchar  Invert2;                /* destination element                    */
} jb_Exchange;

/* IOCTL PREVENT/ALLOW - Command parameter structure */
typedef struct _jb_PreventAllow {
   uchar Prevent;                 /* 1 = prevent removal of media            */
} jb_PreventAllow;

/* IOCTL Command Structure */
typedef struct _jb_IOCTL {
   union {
      jb_Inquiry      Inquiry;
      jb_Inventory    Inventory;
      jb_Move         Move;
      jb_Exchange     Exchange;
      jb_PreventAllow PreventAllow;
   /*   struct sc_iocmd PassThrough;   */
   } Cmd;
   char  SenseBytes[256];
   uchar status_validity;        /* 0 = no valid status                      */
                                 /* 1 = scsi_bus_status valid                */
                                 /* 2 = adapter_status valid                 */
   uchar scsi_bus_status;        /* SCSI bus status                          */
   uchar adapter_status;         /* Adapter return status                    */
} jb_IOCTL;

#endif
