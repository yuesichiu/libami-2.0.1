# libami-2.0.1

	Welcome to the Asterisk Manager Interface (AMI)

	The intention of the AMI is to create an abstration layer to make communications
	with asterisk PBX's manager interface easier.

	For more information on asterisk, visit asterisk.org and/or digium.com. For more
	information on the asterisk manager interface, visit voip-info.org.

	Installation of the AMI library is just like any other GNU package.
	./configure <options>
	make
	make install

	There are no special requirements for the software. It uses straight ANSI C with no
	external libraries and no GNU extensions.

	If you are planning on using either the Asterisk Manager Proxy (AMP) and/or the Asterisk
	Manager Administrator (AMA), libami is a requirement of both.


	Asterisk have changed mostly since libami-0.96, 2008-11-21, so maintain this code is essential
	in OpenSource world. 
	Add examples in libami-2.0.1 and fix some bugs(2017-03-21 16:39).
	Changelog:
		* add network API in net.c
		* ami_ping in manager.c
		* ami_server_version in info.h
		* add app-amitest
	above bug-fix have been verified in Asterisk-11.15.0
	

Thanks,
Yuesichiu(yuesichiu@126.com)

