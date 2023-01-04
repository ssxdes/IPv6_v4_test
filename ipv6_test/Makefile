MAKE_FLAG := -DLINUX
CFLAG := -lpthread
MAKE := gcc 
MAKE_FILE := Makefile
TARGRT := server client
TARGET_BUILD := 
TARGRT_CLEAN := clean
TARGET_DEPENDENCE := etSockLib.c etSockV6Lib.c etSockCommLib.c
SERVER_DEPENDENCE := main_server.c
CLIENT_DEPENDENCE := main_client.c
DO_MAKE := $(MAKE) $(MAKE_FLAG) $(TARGET_DEPENDENCE) $(CFLAG)
DO_MAKE_CLEAN := rm -rf

.PHONY: ipv6addr all client server clean server-clean client-clean
.DEFAULT_GOAL := all

ipv6addr:
	ifconfig eno16777736 inet6 add 2022:da8:e001::6:6:6
	ifconfig eno16777736 inet6 add 2022:da8:e002::6:6:6
	ifconfig eno16777736 inet6 add 2022:da8:e003::6:6:6
	ifconfig eno16777736 inet6 add 2022:da8:e000::8:8:8
	
all:$(TARGET_DEPENDENCE) $(CLIENT_DEPENDENCE) $(SERVER_DEPENDENCE)
	$(MAKE) $(MAKE_FLAG) $(TARGET_DEPENDENCE) $(CLIENT_DEPENDENCE) $(CFLAG) -o client
	$(MAKE) $(MAKE_FLAG) $(TARGET_DEPENDENCE) $(SERVER_DEPENDENCE) $(CFLAG) -o server
	
client:$(TARGET_DEPENDENCE) $(CLIENT_DEPENDENCE)
	$(MAKE) $(MAKE_FLAG) $(TARGET_DEPENDENCE) $(CLIENT_DEPENDENCE) $(CFLAG) -o client
	
server:$(TARGET_DEPENDENCE) $(SERVER_DEPENDENCE)
	$(MAKE) $(MAKE_FLAG) $(TARGET_DEPENDENCE) $(SERVER_DEPENDENCE) $(CFLAG) -o server
	
clean:
	$(DO_MAKE_CLEAN) $(TARGRT)
	
server-clean:
	$(DO_MAKE_CLEAN) server
	
client-clean:
	$(DO_MAKE_CLEAN) client