include ../../htx.mk

TARGET= hxecom hxecom2 mkintfmap auto
#all = ${TARGET}

LDFLAGS +=

OBJ_SUFF=.o
hxecom_OBJECTS = hxecom.o \
        comread.o \
        comwrite.o \
        comsock.o \
        comrw.o \
        rule.o \
        com1_new.o \
        com2_new.o 

hxecom2_OBJECTS = coord.o \
    comsock.o \
    com2_new.o

mkintfmap_OBJECTS = mkintfmap.o

auto_OBJECTS = automate.o

hxecom_SRCS = $(patsubst %.o, %.c, $(hxecom_OBJECTS))
hxecom2_SRCS = $(patsubst %.o, %.c, $(hxecom2_OBJECTS))
mkintfmap_SRCS = $(patsubst %.o, %.c, $(mkintfmap_OBJECTS))
auto_SRCS = $(patsubst %.o, %.c, $(auto_OBJECTS))

all: hxecom hxecom2 mkintfmap auto

CFLAGS += -D__RELGR2600__
LIBS = -lhtx64 -lpthread 
 
.PHONY: all clean

hxecom: ${hxecom_OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${hxecom_OBJECTS} ${LIBS} -o hxecom 
	${CP} hxecom ${SHIPBIN}

hxecom2: ${hxecom2_OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${hxecom2_OBJECTS} ${LIBS} -o hxecom2 
	${CP} hxecom2 ${SHIPBIN}

mkintfmap: ${mkintfmap_OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${mkintfmap_OBJECTS} ${LIBS} -o mkintfmap 
	${CP} mkintfmap ${SHIPBIN}

auto: ${auto_OBJECTS}
	${CC} ${LDFLAGS} ${LIBPATH} ${auto_OBJECTS} ${LIBS} -o auto 
	${CP} auto ${SHIPBIN}

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

clean:
	${RM} -f *.o ${TARGET} ${SHIPBIN}/${TARGET}
