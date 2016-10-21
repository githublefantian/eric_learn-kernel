obj-m = tmain.o
tmain-y = main.o  src/other.o
#tmain-y = main.o  

all:
	 make -C /lib/modules/`uname -r`/build EXTRA_CFLAGS=-I$(shell pwd)/include    M=`pwd`
#       /home/eric/kvm/scripts/sign-file sha512 /home/eric/kvm/signing_key.priv /home/eric/kvm/signing_key.x509 tmain.ko
#
clean:
	rm -f *.o *.ko *.mod.c modules.order Module.symvers  src/*.o
