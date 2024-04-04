libplugin.so: json_proto.c
	gcc -shared -o libplugin.so -fPIC json_proto.c
	ipfixcol2 -c config.xml -p libplugin.so

