all: usuario gestor

gestor: gestor.o gestor.h
	gcc -o gestor gestor.o -pthread

gestor.o: gestor.c gestor.h
	gcc -c gestor.c -pthread

usuario: usuario.o usuario.h
	gcc -o usuario usuario.o -pthread

usuario.o: usuario.c usuario.h
	gcc -c usuario.c -pthread
