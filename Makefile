#3DS_IP := 10.1.36.183
#3DS_IP := 192.168.6.217
3DS_IP := 172.20.10.8

CITRA := /home/halcyon/Desktop/3DS/citra/build/src/citra_qt/citra-qt

all: upload

clean:
	$(MAKE) -C build clean
	
binary:
	$(MAKE) -C build all
	cp build/*.3dsx .
	
upload: binary
	3dslink -a $(3DS_IP) *.3dsx
	
test: binary
	$(CITRA) *.3dsx
