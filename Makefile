all: libplugin.so 
	ipfixcol2 -c config.xml -p libplugin.so

libplugin.so: json_proto.c
	gcc -shared -o libplugin.so -fPIC json_proto.c

debug: libplugin.so
	valgrind --show-leak-kinds=all --leak-check=full ipfixcol2 -c config.xml -p libplugin.so
