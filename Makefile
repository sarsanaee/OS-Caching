obj-m += memcached_nf_tcp.o
obj-m += memcached_nf_udp.o
# obj-m += socket.o

# obj-m += testing.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean