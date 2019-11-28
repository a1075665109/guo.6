all: oss user

clean:
	-rm oss user logFile

dt:
	gcc -o user user.c
	gcc -o oss oss.c	
