all:
	gcc -fPIC -g -c -Wl,-export-dynamic -Wall -I.. -I../../.. -I../../../src TestPlugin.cc
	gcc -shared -Wl,-soname,libtestplugin.so.1 -Wl,-export-dynamic -o libtestplugin.so.1 TestPlugin.o
