
LASTVERSION=1.2.2
VERSION=1.2.2

# Kitsune
ifdef KITSUNE_PATH
KITSUNE_INCLUDE = -I $(KITSUNE_PATH)/include
KITSUNE_LIB = -L $(KITSUNE_PATH)/lib
KITSUNE_BIN = $(KITSUNE_PATH)/bin/
else
KITSUNE_INCLUDE =
KITSUNE_LIB =
KITSUNE_BIN =
endif
EKLIB  = $(KITSUNE_LIB) -lkitsune-threads
CFLAGS_SHARED = -Wall -ldl -shared -fPIC -u kitsune_init_inplace $(KITSUNE_INCLUDE)
CFLAGS = -O2 -Wall -ggdb -fvisibility=hidden -ldl -fPIC  $(EKLIB)  $(KITSUNE_INCLUDE)
EKDRV = $(KITSUNE_BIN)driver
EKCC = $(KITSUNE_BIN)ktcc --dostackvars  --doglobalreg --automigrate --keepunused -include ktthreads.h
EKJOIN = $(KITSUNE_BIN)kttjoin
EKGEN = $(KITSUNE_BIN)xfgen

APP=memcached
USER_XFORM=$(APP).xf
GENERATED_XFORM=dsu.c

LIBS = -pthread -levent -lrt -ldl

IFLAGS =

# ICECAST CFLAGS
CONFIG_HEADER = config.h
CFLAGS += -DHAVE_CONFIG_H -include $(CONFIG_HEADER) -D_GNU_SOURCE

SRCS = assoc.c  daemon.c  items.c  memcached.c  slabs.c  stats.c  thread.c

all: $(APP).so

OBJS = $(patsubst %.c,%.o,$(SRCS))

.c.o:
	$(EKCC) $(CFLAGS) $(IFLAGS) -c $*.c --doktsavetypes --typesfile-out=$*.ktt

$(APP)-$(VERSION).ktt: $(OBJS)
	$(EKJOIN) $@ $(patsubst %.o,%.ktt,$(OBJS))

$(GENERATED_XFORM): $(APP)-$(VERSION).ktt $(USER_XFORM)
	$(EKGEN) $@ ../$(APP)-$(LASTVERSION)/$(APP)-$(LASTVERSION).ktt $(APP)-$(VERSION).ktt $(USER_XFORM)

$(APP).so: $(OBJS) $(GENERATED_XFORM)
	$(CC) $(CFLAGS_SHARED) -ggdb3 -o $(APP).so $(OBJS) $(GENERATED_XFORM) $(EKINC) $(EKLIB) $(LINK) $(LIBS)
	cp $(APP).so $(APP)-2.so 

clean:
	rm -f *.o *.swp *.so *.cil.c *.cil.i *.i *.ktt dsu.c
