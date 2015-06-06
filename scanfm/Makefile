CC=g++
CFLAGS=-O2

SOURCES = $(wildcard *.cpp)
OBJECTS = $(subst .cpp,.o,$(SOURCES))

%.o: %.cpp
	$(CC) -c $(CFLAGS) $^ -o $@

scanfm: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o scanfm
	#chmod +x scanfm

clean:
	rm -f scanfm $(OBJECTS)
