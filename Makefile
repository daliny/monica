USER_DIR = base
TESTS_DIR = base/tests
OBJ_DIR = base/obj

CPPFLAGS += -I $(USER_DIR)

CXXFLAGS += -g -Wall -Wextra -std=c++11

TESTS = BlockingQueue_test BoundedBlockingQueue_test

all : $(TESTS)

clean : 
		rm -rf $(OBJ_DIR)
		rm -f  $(TESTS)
		mkdir  $(OBJ_DIR)

$(OBJ_DIR)/BlockingQueue_test.o: $(TESTS_DIR)/BlockingQueue_test.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^
	mv BlockingQueue_test.o $(OBJ_DIR)/BlockingQueue_test.o


BlockingQueue_test : $(OBJ_DIR)/BlockingQueue_test.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -lpthread -o $@

$(OBJ_DIR)/BoundedBlockingQueue_test.o: $(TESTS_DIR)/BoundedBlockingQueue_test.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $^
	mv BoundedBlockingQueue_test.o $(OBJ_DIR)/BoundedBlockingQueue_test.o

BoundedBlockingQueue_test : $(OBJ_DIR)/BoundedBlockingQueue_test.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -lpthread -o $@

