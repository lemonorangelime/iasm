MAKE := make

default: depends default

depends:
	if [ ! -d "./vm" ]; then \
		git clone https://github.com/lemonorangelime/vm; \
	fi
	cd vm; \
	git pull; \
	make; \
	cd ..;

clean:
	rm -rf ./vm/
	make -f Makefile_x86_64 clean
	make -f Makefile_x86 clean

%:
	make -f Makefile_x86_64 $@
	make -f Makefile_x86 $@
