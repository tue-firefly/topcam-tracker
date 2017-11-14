.PHONY: sdk test

BIN_FILE            = tracker
BIN_DIR             = bin
OBJ_DIR             = obj
TEST_DIR	    = test
BIN_PATH            = $(BIN_DIR)/$(BIN_FILE)

ARCH                = x86
WORDSIZE            = 64

all: $(BIN_PATH)

BOOST_LIBS          = -L /usr/lib/boost/lib -lboost_system
BOOST_TEST_LIBS     = -lboost_unit_test_framework

OPENCV_CFLAGS       = $(shell pkg-config --cflags opencv)
OPENCV_LIBS	    = -lopencv_highgui -lopencv_imgproc -lopencv_core


VIMBACPP_CFLAGS = -I$(VIMBASDK_DIR)
VIMBACPP_LIBS   = -L$(BIN_DIR) -lVimbaCPP -lVimbaC -Wl,-rpath-link,$(BIN_DIR)

# VimbaCPP Dynamic library to link to (and also VimbaC)
VimbaCPP: $(BIN_DIR) 
	test -n "$(VIMBASDK_DIR)" # Make sure $$VIMBASDK_DIR is sets (should point to your sdk dir 'Vimba_2_1/')
	cp $(VIMBASDK_DIR)/VimbaC/DynamicLib/$(ARCH)_$(WORDSIZE)bit/libVimbaC.so $(BIN_DIR)/
	cp $(VIMBASDK_DIR)/VimbaCPP/DynamicLib/$(ARCH)_$(WORDSIZE)bit/libVimbaCPP.so $(BIN_DIR)/


SOURCE_DIR          = src
INCLUDE_DIRS        = -I$(SOURCE_DIR)

LIBS                = $(VIMBACPP_LIBS) \
                      $(OPENCV_LIBS) \
                      $(BOOST_LIBS) \
		              -lrt \
                      -pthread \

TEST_LIBS   	    = $(OPENCV_LIBS) \
                      -lopencv_imgcodecs \
                      -lboost_system \
					  -lpthread \
                      $(BOOST_TEST_LIBS)

CFLAGS              = -std=c++98 -Wall -Wextra -Werror \
                      $(COMMON_CFLAGS) \
                      $(VIMBACPP_CFLAGS) \
                      $(OPENCV_CFLAGS)

TEST_CFLAGS 	    = -Wall -Wextra -Werror \
		      $(OPENCV_CFLAGS)

OBJ_FILES           = $(OBJ_DIR)/DroneDetector.o \
                      $(OBJ_DIR)/ApiController.o \
                      $(OBJ_DIR)/FrameObserver.o \
                      $(OBJ_DIR)/program.o

TEST_OBJ_FILES	    = $(OBJ_DIR)/DroneDetector.o \
                      $(TEST_DIR)/main.o

DEPENDENCIES        = VimbaCPP

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(OBJ_DIR)
	g++ -c $(INCLUDE_DIRS) $(DEFINES) $(CFLAGS) -o $@ $<

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp 
	g++ -c  $(TEST_CFLAGS) -o $@ $<

$(BIN_PATH): $(DEPENDENCIES) $(OBJ_FILES) $(BIN_DIR)
	g++ $(ARCH_CFLAGS) -o $(BIN_PATH) $(OBJ_FILES) $(LIBS) -Wl,-rpath,'$$ORIGIN'

$(TEST_DIR)/test: $(TEST_OBJ_FILES)
	g++ $(ARCH_CFLAGS) -o $(TEST_DIR)/test $(TEST_OBJ_FILES) $(TEST_LIBS)

test: $(TEST_DIR)/test
	cd $(TEST_DIR) && ./test

clean:
	rm bin -rf
	rm obj -rf
	rm -f test/*.o

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)
