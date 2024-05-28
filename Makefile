.PHONY: all debug clear

all: libplugin.so 
	ipfixcol2 -c xml/config.xml -p libplugin.so

libplugin.so: json_proto.c 
	gcc -c json_proto.c -fPIC
	gcc json_proto.o -shared -o libplugin.so

debug: libplugin.so
	valgrind --show-leak-kinds=all --leak-check=full ipfixcol2 -c config.xml -p libplugin.so

clear: 
	rm -f *.o *.so
