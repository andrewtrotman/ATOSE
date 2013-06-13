CFLAGS = -Wall

all : hid_list.mac imx.mac

hid_list.mac : hid_list.c
	g++ $(CFLAGS) hid_list.c -framework CoreFoundation -framework IOKit -o hid_list.mac

imx.mac : imx.c
	g++ $(CFLAGS) imx.c -framework CoreFoundation -framework IOKit -o imx.mac

clean :
	rm hid_list.mac imx.mac
