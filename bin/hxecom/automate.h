#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>

#ifndef __HTX_LINUX__
	#include <sys/ndd_var.h>
	#include <sys/kinfo.h>
#else
	#include <netpacket/packet.h>
    #include <net/ethernet.h> /* the L2 protocols */
	#include <linux/if_ether.h>
#endif
#define ETHERTYPE_ARP   0x421D
#define SNAP_TYPE_IP    0x0800
#define ARPHRD_ETHER    1       /* ethernet hardware address    */
#define ARPOP_REQUEST   1       /* request to resolve address */
#define ARPOP_REPLY     2       /* response to previous request */
#define MAX_INTERFACE  512
#define NUM_RETRIES    5
#define MAX_COUNT 	   5
extern int errno;

/*
 * Ethernet packet format...
 */
struct  arphdr1 {
        u_short ar_hrd;         /* format of hardware address */
        u_short ar_pro;         /* format of protocol address */
        u_char  ar_hln;         /* length of hardware address */
        u_char  ar_pln;         /* length of protocol address */
        u_short ar_op;          /* one of: */
};

struct  ether_arp1 {
        struct  arphdr1 ea_hdr;  /* fixed-size header */
        u_char  arp_sha[6];     /* sender hardware address */
        u_char  arp_spa[4];     /* sender protocol address */
        u_char  arp_tha[6];     /* target hardware address */
        u_char  arp_tpa[4];     /* target protocol address */
};

typedef struct {
    unsigned char dst[6];
    unsigned char src[6];
    unsigned short ethertype;
    struct  ether_arp1 arp_pkt;
    u_char  pad[18];
} xmit;

hwaddr_aton(a, n)
char *a;
u_char *n;
{
        int i, o[6];
        i = sscanf(a, "%x:%x:%x:%x:%x:%x", &o[0], &o[1], &o[2],&o[3], &o[4], &o[5]);
        if (i != 6) {
                fprintf(stderr, "invalid hardware address .%s.\n");
                return (0);
        }

        for (i=0; i<6; i++)
                n[i] = o[i];

        return (6);
}

/*
 * Hex print function...
 */
pit(str, buf, len)
u_char *str;
u_char *buf;
int len;
{
        int i;
        printf("%s", str);
        for (i=0; i<len; i++)
                printf("%2.2X", buf[i]);
        fflush(stdout);
}

struct dev_details {
    char name[32];
    char mac_addr[6];
} ;

struct nw_setup {
	char src[32];
	char dest[32] ;
};
/* Thread specific data structure */
struct thread_context {

	/************************** worker thread info ***********************/

    pthread_t worker;
	pthread_mutex_t worker_mutex;
    pthread_attr_t worker_attrs;
    pthread_cond_t worker_cond;
	int worker_join_rc;
    int retries;
	int tid;

	/************************* helper thread info ************************/

    pthread_t helper;
    pthread_mutex_t helper_mutex;
    pthread_attr_t helper_attrs;
    pthread_cond_t helper_cond;
    int helper_join_rc;
    int exit_helper;
    int exit_worker;

	/*********************** Variables used by Worker thread ************/

	struct dev_details source;
	struct dev_details dest[MAX_INTERFACE * MAX_COUNT];
	struct dev_details dev_list[MAX_INTERFACE];
	int num_devices;
	volatile int rcvd_reply;
    volatile int num_reply;

};


/* Function declaration */
void *tc_worker_thread(void * thread_context_w);
void *tc_helper_thread(void *thread_context);

