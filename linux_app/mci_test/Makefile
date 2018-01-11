CUR_DIR=$(shell pwd)

TARGET = mci_test

PRJ_PATH=/home/wangkai/workspace/test-version/Supervyse_3.0-20171009
CC=$(PRJ_PATH)/ToolChain/Host/AST2500/x-tools/arm-aspeed-linux-gnueabi/bin/arm-aspeed-linux-gnueabi-gcc

#CFLAGS += -isysroot $(BUILD_SYSROOT) 
CFLAGS += -I$(PRJ_PATH) -I$(PRJ_PATH)/libmci/inc  -I$(PRJ_PATH)/RCM/lib  -I$(PRJ_PATH)/ipmid/IPMI/Include
CFLAGS += 

# need link to libs that this lib needed: -lrcm
LIBS +=  -L. -lsafec -lmci -lrcm -ldbglog -lpthread -lm
LDFLAGS += -L$(PRJ_PATH)/libs $(LIBS)

FS_PATH=/tmp/fs

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LDFLAGS) 
	[ ! -z $FS_PATH ]; mkdir -p $FS_PATH 
	mkdir -p $(FS_PATH)/sbin/
	chmod +x $(TARGET)
	cp $(TARGET)  $(FS_PATH)/sbin/	
	#$(STRIP) $(FS_PATH)/sbin/$(TARGET)

clean:
	- rm $(TARGET)
	- rm  $(CUR_DIR)/*.o
	
install:	
	mkdir -p $(FS_PATH)/sbin/
	chmod +x $(TARGET)
	cp $(TARGET)  $(FS_PATH)/sbin/
	$(STRIP) $(FS_PATH)/sbin/$(TARGET)
	
