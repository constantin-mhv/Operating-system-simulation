build:
	gcc -o main main.c utils.c data_structures.c -g -Wall

run:
	./main

clean:
	rm -rf main
	