app-amitest: clean libnet.so
	@echo "Generate app-amitest...."
	@gcc -DDEBUG -o app-amitest app-amitest.h app-amitest.c -lami -L. -lnet
clean:
	@rm -f app-amitest net.o libnet.so
libnet.so:
	@echo "Build libnet.so...."
	@gcc -c -fPIC net.c
	@gcc -shared net.o -o libnet.so
	@cp -f libnet.so /lib64
libnet.a:
	gcc -c -fPIC net.c
	 ar -r libnet.a net.o

