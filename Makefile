MAKE := make

default: depends default

depends:
	if [ ! -d "./zydis" ]; then \
		git clone --recursive https://github.com/zyantific/zydis; \
	fi
	cd zydis; \
	git pull; \
	if [ ! -d "./amalgamated-dist/" ]; then \
		python3 assets/amalgamate.py; \
		gcc -fomit-frame-pointer -rdynamic -mhard-float -flto -s -Oz -msse2 amalgamated-dist/Zydis.c -Iamalgamated-dist/ -c -o zydis64.o; \
		gcc -fomit-frame-pointer -rdynamic -mhard-float -flto -s -Oz -m32 amalgamated-dist/Zydis.c -Iamalgamated-dist/ -c -o zydis32.o; \
		strip zydis64.o; \
		strip zydis32.o; \
	fi
	cd ..;

clean:
	rm -rf ./vm/
	make -f Makefile_x86_64 clean
	make -f Makefile_x86 clean

%:
	make -f Makefile_x86_64 $@
	make -f Makefile_x86 $@
