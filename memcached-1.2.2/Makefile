
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
GLIBC225INC = inc
EKLIB  = $(KITSUNE_LIB) -lkitsune-threads
CFLAGS_SHARED = -Wall -ldl -shared -fPIC -u kitsune_init_inplace $(KITSUNE_INCLUDE)
CFLAGS = -O2 -Wall -ggdb -fvisibility=hidden -ldl -fPIC  -I $(GLIBC225INC) $(EKLIB)  $(KITSUNE_INCLUDE)
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

all: glibc255-headers $(APP).so

OBJS = $(patsubst %.c,%.o,$(SRCS)) glibc225.o

glibc225.o: ../glibc-2.25/glibc225.c
	$(EKCC) $(CFLAGS) $(IFLAGS) -c $< --doktsavetypes --typesfile-out=$*.ktt

$(GLIBC225INC)/bits/sysmacros.h:
	mkdir -p $(GLIBC225INC)/bits
	cp /usr/include/bits/sysmacros.h $(GLIBC225INC)/bits/sysmacros.h
	patch -p1 < ../sysmacros.h.patch

$(GLIBC225INC)/stdlib.h:
	cp /usr/include/stdlib.h $(GLIBC225INC)/stdlib.h
	patch -p1 < ../stdlib.h.patch

glibc255-headers: $(GLIBC225INC)/bits/sysmacros.h $(GLIBC225INC)/stdlib.h

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
	rm -rf *.o *.swp *.so *.cil.c *.cil.i *.i *.ktt dsu.c $(GLIBC225INC)
