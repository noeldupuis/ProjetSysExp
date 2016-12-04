all:
	clear
	gcc -Wall -c liste_proc.c sdcom.c
	gcc liste_proc.o sock_server.o cmds.o -o sdcom sdcom.c
	gcc -o rcom rcom.c

clean:
	rm lcmnd liste_proc.o