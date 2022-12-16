CXXFLAGS=-std=c++17 -g -Wall -Wextra -Wpedantic -D_GNU_SOURCE -Werror=all -lpthread
CXX=g++
#-fsanitize=undefined,leak,address

db.o: db.cpp db.hpp
	$(CXX) -c db.cpp $(CXXFLAGS)

student.o: student.cpp student.hpp
	$(CXX) -c student.cpp $(CXXFLAGS)

queries.o: queries.cpp queries.hpp
	$(CXX) -c queries.cpp $(CXXFLAGS)

smalldb: sdbsh.cpp smalldb.cpp db.o student.o queries.o
	$(CXX) -o smalldb smalldb.cpp db.o student.o queries.o $(CXXFLAGS)
	$(CXX) -o sdbsh sdbsh.cpp $(CXXFLAGS)

