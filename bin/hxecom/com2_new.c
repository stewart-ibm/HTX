
/* @(#)89	1.2.1.4  src/htx/usr/lpp/htx/bin/hxecom/com2.c, exer_com, htx53A 5/24/04 17:26:34 */

/*
 *   COMPONENT_NAME: exer_com
 *
 *   FUNCTIONS: AttachGLOBALSems
 *		AttachShmHxecom
 *		Detach_HXECOM
 *		DownSem
 *		GateSem
 *		GlobalSignal
 *		GlobalWait
 *		IncSem
 *		OthMsg
 *		SetSem
 *		UpSem
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <sys/shm.h>
#include "hxecomdef.h"

#ifdef __HTX_LINUX__
#include <sys/stat.h>
#endif

extern int com_hxfmsg(struct htx_data *p, int err, enum sev_code  sev, char *text);

#define GLOBAL_SEMID		shm_HXECOM->MasterSemid
#ifndef __HTX_LINUX__
union semun {
               int val;                  
               struct semid_ds *buf;       
               unsigned short int *array; 
               struct seminfo *__buf;      
       };
#endif



void GlobalWait(int SemNo, struct htx_data * stats)
{
   DownSem(GLOBAL_SEMID, SemNo, stats);
}

/*twm*/
void DecSem(int SemId, int SemNo, struct htx_data *stats)
{
   int rc;
   struct sembuf Slock   = { 0, -1, 0x0000 };

   Slock.sem_num = SemNo;
   do {
      errno = 0;
      rc = semop(SemId, &Slock, 1);
   }
   while(rc == -1 && errno == EINTR);

   if(rc == -1) {
      sprintf(stats->msg_text, 
          "DecSem: Error decrementing semaphore. SemId = %d, SemNo = %d\n %s\n",
                             (int) SemId, (int) SemNo, STRERROR(errno));
      com_hxfmsg(stats, HTXERROR(EX_SEMDWN2,errno), HTX_HE_SOFT_ERROR, stats->msg_text);
      HE_exit(EX_SEMDWN2);
   }
}


void DownSem(int SemId, int SemNo, struct htx_data *stats)
{
   int rc;
   struct sembuf Slock   = { 0, -1, SEM_UNDO };

   Slock.sem_num = SemNo;
   do {
      errno = 0;
      rc = semop(SemId, &Slock, 1);
   }
   while(rc == -1 && errno == EINTR);

   if(rc == -1) {
      sprintf(stats->msg_text, 
          "DownSem: Error decrementing semaphore. SemId = %d, SemNo = %d\n %s\n",
                             (int) SemId, (int) SemNo, STRERROR(errno));
      com_hxfmsg(stats, HTXERROR(EX_SEMDWN2,errno), HTX_HE_SOFT_ERROR, stats->msg_text);
      HE_exit(EX_SEMDWN2);
   }
}



void GlobalSignal(int SemNo, struct htx_data * stats)
{
   UpSem(GLOBAL_SEMID, SemNo, stats);
}



void UpSem(int SemId, int SemNo, struct htx_data *stats)
{
   int rc;
   struct sembuf Sunlock = { 0, +1, SEM_UNDO };

   Sunlock.sem_num = SemNo;
   do {
      errno = 0;
      rc = semop(SemId, &Sunlock, 1);
   }
   while(rc == -1 && errno == EINTR);

   if(rc == -1) {
      sprintf(stats->msg_text, 
          "UpSem: Error incrementing semaphore. SemId = %d, SemNo = %d\n %s\n",
                             (int) SemId, (int) SemNo, STRERROR(errno));
      com_hxfmsg(stats, HTXERROR(EX_SEMUP1,errno), HTX_HE_SOFT_ERROR, stats->msg_text);
      HE_exit(EX_SEMUP1);
   }
}



void IncSem(int SemId, int SemNo, int inc, struct htx_data *stats)
{
   int rc;
   struct sembuf SemBuf;

   SemBuf.sem_num = SemNo;
   SemBuf.sem_op  = inc;
   /*SemBuf.sem_flg = SEM_UNDO; twm */
   SemBuf.sem_flg = 0x0000;
   do {
      errno = 0;
      rc = semop(SemId, &SemBuf, 1);
   }
   while(rc == -1 && errno == EINTR);

   if(rc == -1) {
      sprintf(stats->msg_text, 
          "IncSem: Error incrementing semaphore. SemId = %d, SemNo = %d SemInc = %d\n %s\n",
                             (int) SemId, (int) SemNo, inc, STRERROR(errno));
      com_hxfmsg(stats, HTXERROR(EX_SEMUP2,errno), HTX_HE_SOFT_ERROR, stats->msg_text);
      HE_exit(EX_SEMUP2);
   }
}



void GateSem(int SemId, int SemNo, struct htx_data *stats)
{
   int rc;
   struct sembuf SemBuf[2] = {0, -1, SEM_UNDO,
                              0, +1, SEM_UNDO };

   SemBuf[0].sem_num = SemNo;
   SemBuf[1].sem_num = SemNo;
   do {
      errno = 0;
      rc = semop(SemId, SemBuf, 2);
   }
   while(rc == -1 && errno == EINTR);

   if(rc == -1) {
      sprintf(stats->msg_text, 
          "GateSem: Error gating semaphore. SemId = %d, SemNo = %d\n %s\n",
                             (int) SemId, (int) SemNo, STRERROR(errno));
      com_hxfmsg(stats, HTXERROR(EX_SEMUP3,errno), HTX_HE_SOFT_ERROR, stats->msg_text);
      HE_exit(EX_SEMUP3);
   }
}




void SetSem(int SemId, int sem_type, int value, struct htx_data *stats)
{

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
} arg;

#ifdef __DEBUG__
   printf("in the function setsem, SemId = %d, sem_type = %d, value = %d \n", SemId, sem_type, value);
#endif
	arg.val = value; 
#ifdef __HTX_LINUX__
   if(semctl(SemId, sem_type, SETVAL, (union semun)arg) == -1){ 
#else
   if(semctl(SemId, sem_type, SETVAL, (union semun)value) == -1){ 
#endif
      sprintf(stats->msg_text, "SetSem: Error initializing sem %d - %s\n", sem_type, STRERROR(errno));
      com_hxfmsg(stats, HTXERROR(EX_SEMINIT1,errno), HTX_HE_SOFT_ERROR, stats->msg_text);
#ifdef __DEBUG__
      printf("error initializing semaphore exiting\n");
#endif
      HE_exit(EX_SEMINIT1);
   }
#ifdef __DEBUG__
  printf("returning\n");
#endif
}



void OthMsg(struct htx_data * stats, char msg[])
{
    if(*(stats->run_type) == 'O') {
        com_hxfmsg(stats, HTXERROR(HEINFO13,0), HTX_HE_INFO, msg);
        fflush(stdout);
    }
}



void Detach_HXECOM(struct shm_hxecom_t * ptr)
{
    int MID;
    int NoExer;
    struct shmid_ds ShmDs;
    int i;
    int SemId;
    struct semid_ds arg;

    if(ptr == (struct shm_hxecom_t *)NULL)
        return;

    MID = ptr->mID;
    NoExer = ptr->NoExer;
    shmdt((char *)ptr);

    shmctl(MID, IPC_STAT, &ShmDs);
    if(ShmDs.shm_nattch == 0) {
        for(i=0; i< NoExer; i++)
            if((SemId = semget(MY_HXECOM_KEY+i, 1, 0)) != -1)
                semctl(SemId, 0, IPC_RMID, arg); 
        shmctl(MID, IPC_RMID, 0);
        if((SemId = semget(MASTER_HXECOM_KEY, MASTER_NO_SEMS, 0)) != -1)
            semctl(SemId, 0, IPC_RMID, arg); 
    }
}



void AttachShmHxecom(void)
{
   int mID;

   if((mID = shmget(SHM_HXECOM_KEY, sizeof(struct shm_hxecom_t),
                        IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                                     S_IROTH | S_IWOTH)) == -1) {
      HE_exit(EX_SHMGET0);
   }
   if((shm_HXECOM = (struct shm_hxecom_t *)shmat(mID, (char *)0, 0)) == (struct shm_hxecom_t *) -1) {
      HE_exit(EX_SHMAT0);
   }
}



void AttachGLOBALSems(struct htx_data *stats)
{
   if((GLOBAL_SEMID = semget((MASTER_HXECOM_KEY), MASTER_NO_SEMS,
                     IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1) {
      HE_exit(EX_SEMCREAT0);
   }
}

