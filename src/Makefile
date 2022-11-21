CXXFLAGS+=-std=c++17 -fsanitize=undefined,leak,address -g -Wall -Wextra -Wpedantic -D_GNU_SOURCE -Werror=all
CXX=g++
LDLIBS+=-lpthread
SOURCES = $(wildcard *.cpp */*.cpp)
HEADERS = $(wildcard *.hpp */*.hpp)

OBJ=db.o student.o

.PHONY: main
main: smalldb check run

smalldb: smalldb.cpp ${OBJ}
	$(CXX) $(LDFLAGS) $^ -o $@ $(LOADLIBES) $(LDLIBS)

%.o: %.cpp %.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^

.PHONY: clean
clean:
	-rm *.o
