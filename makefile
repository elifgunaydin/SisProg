all: compile run

compile:
	g++ -I ./include/ -o ./lib/fs.o -c ./src/fs.cpp
	g++ -I ./include/ -o ./bin/main ./lib/fs.o ./src/main.cpp

run:
	./bin/main

clean:
	rm -r ./lib/* ./bin/*