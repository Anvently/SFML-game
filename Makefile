all: compile link

compile:
	g++ -IExternal/include -ISources -c *.cpp
	
link: 
	g++ *.o -o main -LExternal/lib -lsfml-graphics -lsfml-window -lsfml-system	