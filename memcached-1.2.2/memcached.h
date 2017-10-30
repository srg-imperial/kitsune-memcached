/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* $Id: memcached.h 512 2007-04-16 23:33:43Z plindner $ */

#include "config.h"
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <event.h>

#define DATA_BUFFER_SIZE 2048
#define UDP_READ_BUFFER_SIZE 65536
#define UDP_MAX_PAYLOAD_SIZE 1400
#define UDP_HEADER_SIZE 8
#define MAX_SENDBUF_SIZE (256 * 1024 * 1024)

/* Initial size of list of items being returned by "get". */
#define ITEM_LIST_INITIAL 200

/* Initial size of the sendmsg() scatter/gather array. */
#define IOV_LIST_INITIAL 400

/* Initial number of sendmsg() argument structures to allocate. */
#define MSG_LIST_INITIAL 10

/* High water marks for buffer shrinking */
#define READ_BUFFER_HIGHWAT 8192
#define ITEM_LIST_HIGHWAT 400
#define IOV_LIST_HIGHWAT 600
#define MSG_LIST_HIGHWAT 100

/* Get a consistent bool type */
#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
  typedef enum {false = 0, true = 1} bool;
#endif

#if HAVE_STDINT_H
# include <stdint.h>
#else
 typedef unsigned char             uint8_t;
#endif

/* Time relative to server start. Smaller than time_t on 64-bit systems. */
typedef unsigned int rel_time_t;

struct stats {
    unsigned int  curr_items;
    unsigned int  total_items;
    uint64_t      curr_bytes;
    unsigned int  curr_conns;
    unsigned int  total_conns;
    unsigned int  conn_structs;
    uint64_t      get_cmds;
    uint64_t      set_cmds;
    uint64_t      get_hits;
    uint64_t      get_misses;
    uint64_t      evictions;
    time_t        started;          /* when the process was started */
    uint64_t      bytes_read;
    uint64_t      bytes_written;
};

#define MAX_VERBOSITY_LEVEL 2

struct settings {
    size_t maxbytes;
    int maxconns;
    int port;
    int udpport;
    struct in_addr interf;
    int verbose;
    rel_time_t oldest_live; /* ignore existing items older than this */
    bool managed;          /* if 1, a tracker manages virtual buckets */
    int evict_to_free;
    char *socketpath;   /* path to unix socket if using local socket */
    double factor;          /* chunk size growth factor */
    int chunk_size;
    int num_threads;        /* number of libevent threads to run */
    char prefix_delimiter;  /* character that marks a key prefix (for stats) */
    int detail_enabled;     /* nonzero if we're collecting detailed stats */
};

extern struct stats stats;
extern struct settings settings;

#define ITEM_LINKED 1
#define ITEM_DELETED 2

/* temp */
#define ITEM_SLABBED 4

typedef struct _stritem {
    struct _stritem *next;
    struct _stritem *prev;
    struct _stritem *h_next;    /* hash chain next */
    rel_time_t      time;       /* least recent access */
    rel_time_t      exptime;    /* expire time */
    int             nbytes;     /* size of data */
    unsigned short  refcount;
    uint8_t         nsuffix;    /* length of flags-and-length string */
    uint8_t         it_flags;   /* ITEM_* above */
    uint8_t         slabs_clsid;/* which slab class we're in */
    uint8_t         nkey;       /* key length, w/terminating null and padding */
    void * end[0];
    /* then null-terminated key */
    /* then " flags length\r\n" (no terminating null) */
    /* then data with terminating \r\n (no terminating null; it's binary!) */
} item;

#define ITEM_key(item) ((char*)&((item)->end[0]))

/* warning: don't use these macros with a function, as it evals its arg twice */
#define ITEM_suffix(item) ((char*) &((item)->end[0]) + (item)->nkey + 1)
#define ITEM_data(item) ((char*) &((item)->end[0]) + (item)->nkey + 1 + (item)->nsuffix)
#define ITEM_ntotal(item) (sizeof(struct _stritem) + (item)->nkey + 1 + (item)->nsuffix + (item)->nbytes)

enum conn_states {
    conn_listening,  /* the socket which listens for connections */
    conn_read,       /* reading in a command line */
    conn_write,      /* writing out a simple response */
    conn_nread,      /* reading in a fixed number of bytes */
    conn_swallow,    /* swallowing unnecessary bytes w/o storing */
    conn_closing,    /* closing this connection */
    conn_mwrite      /* writing out many items sequentially */
};

#define NREAD_ADD 1
#define NREAD_SET 2
#define NREAD_REPLACE 3

typedef struct _conn { /**DSU xfgen_ignore */
    int    sfd;
    int    state;
    struct event event;
    short  ev_flags;
    short  which;   /* which events were just triggered */

    char   * E_OPAQUE rbuf;   /* buffer to read commands into */ /**DSU xfgen */
    char   * E_OPAQUE rcurr;  /* but if we parsed some already, this is where we stopped */ /**DSU xfgen */
    int    rsize;   /* total allocated size of rbuf */
    int    rbytes;  /* how much data, starting from rcur, do we have unparsed */

    char   * E_OPAQUE wbuf; /**DSU xfgen */
    char   * E_OPAQUE wcurr; /**DSU xfgen */
    int    wsize;
    int    wbytes;

    int    write_and_go; /* which state to go into after finishing current write */
    void   * E_OPAQUE write_and_free; /* free this memory after finishing writing */ /**DSU xfgen */

    char   * E_OPAQUE ritem;  /* when we read in an item's value, it goes here */ /**DSU xfgen */
    int    rlbytes;

    /* data for the nread state */

    /*
     * item is used to hold an item structure created after reading the command
     * line of set/add/replace commands, but before we finished reading the actual
     * data. The data is read into ITEM_data(item) to avoid extra copying.
     */

    void   * E_OPAQUE item;     /* for commands set/add/replace  */ /**DSU xfgen */
    int    item_comm; /* which one is it: set/add/replace */

    /* data for the swallow state */
    int    sbytes;    /* how many bytes to swallow */

    /* data for the mwrite state */
    struct iovec * E_OPAQUE iov;
    int    iovsize;   /* number of elements allocated in iov[] */
    int    iovused;   /* number of elements used in iov[] */

    struct msghdr * E_OPAQUE msglist;
    int    msgsize;   /* number of elements allocated in msglist[] */
    int    msgused;   /* number of elements used in msglist[] */
    int    msgcurr;   /* element in msglist[] being transmitted now */
    int    msgbytes;  /* number of bytes in current msg */

    item   * E_OPAQUE * E_PTRARRAY(self.isize) ilist;   /* list of items to write out */ /**DSU xfgen */
    int    isize;
    item   * E_OPAQUE * E_PTRARRAY(self.isize) icurr; /**DSU xfgen */
    int    ileft;

    /* data for UDP clients */
    bool   udp;       /* is this is a UDP "connection" */
    int    request_id; /* Incoming UDP request ID, if this is a UDP "connection" */
    struct sockaddr request_addr; /* Who sent the most recent request */
    socklen_t request_addr_size;
    unsigned char *hdrbuf; /* udp packet headers */
    int    hdrsize;   /* number of headers' worth of space is allocated */

    int    binary;    /* are we in binary mode */
    int    bucket;    /* bucket number for the next command, if running as
                         a managed instance. -1 (_not_ 0) means invalid. */
    int    gen;       /* generation requested for the bucket */

    struct event_base * E_OPAQUE base;

} conn;

/* number of virtual buckets for a managed instance */
#define MAX_BUCKETS 32768

/* current time of day (updated periodically) */
extern volatile rel_time_t current_time;

/* temporary hack */
/* #define assert(x) if(!(x)) { printf("assert failure: %s\n", #x); pre_gdb(); }
   void pre_gdb (); */

/*
 * Functions
 */

conn *do_conn_from_freelist();
int do_conn_add_to_freelist(conn *c);
char *do_defer_delete(item *item, time_t exptime);
void do_run_deferred_deletes(void);
char *do_add_delta(item *item, int incr, unsigned int delta, char *buf);
int do_store_item(item *item, int comm);
conn *conn_new(const int sfd, const int init_state, const int event_flags, const int read_buffer_size, const bool is_udp, struct event_base *base);


#include "stats.h"
#include "slabs.h"
#include "assoc.h"
#include "items.h"


/*
 * In multithreaded mode, we wrap certain functions with lock management and
 * replace the logic of some other functions. All wrapped functions have
 * "mt_" and "do_" variants. In multithreaded mode, the plain version of a
 * function is #define-d to the "mt_" variant, which often just grabs a
 * lock and calls the "do_" function. In singlethreaded mode, the "do_"
 * function is called directly.
 *
 * Functions such as the libevent-related calls that need to do cross-thread
 * communication in multithreaded mode (rather than actually doing the work
 * in the current thread) are called via "dispatch_" frontends, which are
 * also #define-d to directly call the underlying code in singlethreaded mode.
 */
#ifdef USE_THREADS

void thread_init(int nthreads, struct event_base *main_base);
int  dispatch_event_add(int thread, conn *c);
void dispatch_conn_new(int sfd, int init_state, int event_flags, int read_buffer_size, int is_udp);

/* Lock wrappers for cache functions that are called from main loop. */
char *mt_add_delta(item *item, int incr, unsigned int delta, char *buf);
conn *mt_conn_from_freelist(void);
int   mt_conn_add_to_freelist(conn *c);
char *mt_defer_delete(item *it, time_t exptime);
int   mt_is_listen_thread(void);
item *mt_item_alloc(char *key, size_t nkey, int flags, rel_time_t exptime, int nbytes);
void  mt_item_flush_expired(void);
item *mt_item_get_notedeleted(char *key, size_t nkey, bool *delete_locked);
item *mt_item_get_nocheck(char *key, size_t nkey);
int   mt_item_link(item *it);
void  mt_item_remove(item *it);
int   mt_item_replace(item *it, item *new_it);
void  mt_item_unlink(item *it);
void  mt_item_update(item *it);
void  mt_run_deferred_deletes(void);
void *mt_slabs_alloc(size_t size);
void  mt_slabs_free(void *ptr, size_t size);
int   mt_slabs_reassign(unsigned char srcid, unsigned char dstid);
char *mt_slabs_stats(int *buflen);
void  mt_stats_lock(void);
void  mt_stats_unlock(void);
int   mt_store_item(item *item, int comm);


# define add_delta(x,y,z,a)          mt_add_delta(x,y,z,a)
# define assoc_move_next_bucket()    mt_assoc_move_next_bucket()
# define conn_from_freelist()        mt_conn_from_freelist()
# define conn_add_to_freelist(x)     mt_conn_add_to_freelist(x)
# define defer_delete(x,y)           mt_defer_delete(x,y)
# define is_listen_thread()          mt_is_listen_thread()
# define item_alloc(x,y,z,a,b)       mt_item_alloc(x,y,z,a,b)
# define item_flush_expired()        mt_item_flush_expired()
# define item_get_nocheck(x,y)       mt_item_get_nocheck(x,y)
# define item_get_notedeleted(x,y,z) mt_item_get_notedeleted(x,y,z)
# define item_link(x)                mt_item_link(x)
# define item_remove(x)              mt_item_remove(x)
# define item_replace(x,y)           mt_item_replace(x,y)
# define item_update(x)              mt_item_update(x)
# define item_unlink(x)              mt_item_unlink(x)
# define run_deferred_deletes()      mt_run_deferred_deletes()
# define slabs_alloc(x)              mt_slabs_alloc(x)
# define slabs_free(x,y)             mt_slabs_free(x,y)
# define slabs_reassign(x,y)         mt_slabs_reassign(x,y)
# define slabs_stats(x)              mt_slabs_stats(x)
# define store_item(x,y)             mt_store_item(x,y)

# define STATS_LOCK()                mt_stats_lock()
# define STATS_UNLOCK()              mt_stats_unlock()

#else /* !USE_THREADS */

# define add_delta(x,y,z,a)          do_add_delta(x,y,z,a)
# define assoc_move_next_bucket()    do_assoc_move_next_bucket()
# define conn_from_freelist()        do_conn_from_freelist()
# define conn_add_to_freelist(x)     do_conn_add_to_freelist(x)
# define defer_delete(x,y)           do_defer_delete(x,y)
# define dispatch_conn_new(x,y,z,a,b) conn_new(x,y,z,a,b,main_base)
# define dispatch_event_add(t,c)     event_add(&(c)->event, 0)
# define is_listen_thread()          1
# define item_alloc(x,y,z,a,b)       do_item_alloc(x,y,z,a,b)
# define item_flush_expired()        do_item_flush_expired()
# define item_get_nocheck(x,y)       do_item_get_nocheck(x,y)
# define item_get_notedeleted(x,y,z) do_item_get_notedeleted(x,y,z)
# define item_link(x)                do_item_link(x)
# define item_remove(x)              do_item_remove(x)
# define item_replace(x,y)           do_item_replace(x,y)
# define item_unlink(x)              do_item_unlink(x)
# define item_update(x)              do_item_update(x)
# define run_deferred_deletes()      do_run_deferred_deletes()
# define slabs_alloc(x)              do_slabs_alloc(x)
# define slabs_free(x,y)             do_slabs_free(x,y)
# define slabs_reassign(x,y)         do_slabs_reassign(x,y)
# define slabs_stats(x)              do_slabs_stats(x)
# define store_item(x,y)             do_store_item(x,y)
# define thread_init(x,y)            0

# define STATS_LOCK()                /**/
# define STATS_UNLOCK()              /**/

#endif /* !USE_THREADS */


