MAKE := make

default: depends share default

depends:
	if [ ! -d "./vm" ]; then \
		git clone https://github.com/lemonorangelime/vm; \
	fi
	cd vm; \
	git pull; \
	make; \
	cd ..;

install:
	make -f Makefile_x86_64 install

clean:
	rm -rf ./vm/ ./bios/
	make -f Makefile_x86_64 clean
	make -f Makefile_x86 clean

share:
	mkdir share/bios/
	cp bios/bios.bin share/bios/

%:
	make -f Makefile_x86_64 $@
	make -f Makefile_x86 $@
