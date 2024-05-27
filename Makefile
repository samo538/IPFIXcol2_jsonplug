all: libplugin.so 
	ipfixcol2 -c config.xml -p libplugin.so

libplugin.so: json_proto.c lib/selectorlib.c
	gcc -c json_proto.c lib/selectorlib.c -fPIC
	gcc json_proto.o selectorlib.o -shared -o libplugin.so

debug: libplugin.so
	valgrind --show-leak-kinds=all --leak-check=full ipfixcol2 -c config.xml -p libplugin.so
