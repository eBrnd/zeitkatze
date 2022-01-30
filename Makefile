###################################################
# definitions                                     #
###################################################
CC = g++

EXEC = main 
SRC_DIR = src
INCL_DIR = include 
CFLAGS = -std=c++14 -I$(INCL_DIR) -Wall
LDFLAGS = 
SOURCES = $(SRC_DIR)/zeitkatze.cpp $(SRC_DIR)/zeitkatze_runner.cpp main.cpp
OBJECTS = $(SOURCES:%.cpp=%.o)
RM = rm -rf

###################################################
# project                                         #
###################################################
all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS) 

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	$(RM) $(OBJECTS) 
	$(RM) $(EXEC)
