all:dph prod cons
.PHONY:all
dph:dph.c
	gcc -o dph dph.c -lpthread -lm
prod:prod.c
	gcc -o prod prod.c -lpthread -lm
cons:cons.c
	gcc -o cons cons.c -lpthread -lm
clean:
	rm -f *.o dph prod cons
