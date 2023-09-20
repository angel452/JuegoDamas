all:
	g++ -o main.exe main.cpp -lsfml-window -lsfml-system -lsfml-graphics

clean:
	rm -f main.exe