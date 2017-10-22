.PHONY: sdk

BIN_FILE            = tracker
BIN_DIR             = bin
OBJ_DIR             = obj
BIN_PATH            = $(BIN_DIR)/$(BIN_FILE)

ARCH                = x86
WORDSIZE            = 64

all: $(BIN_PATH)

BOOST_LIBS          = -L /usr/lib/boost/lib -lboost_system

OPENCV_CFLAGS       = $(shell pkg-config --cflags opencv)
OPENCV_LIBS	    = -lopencv_highgui -lopencv_imgproc -lopencv_core
#OPENCV_LIBS         = $(shell pkg-config --libs opencv | sed 's/-lopencv_dnn//' | sed 's/-lopencv_shape//') # Don't try to link these two libs


VIMBACPP_CFLAGS = -I$(VIMBASDK_DIR)
VIMBACPP_LIBS   = -L$(BIN_DIR) -lVimbaCPP -lVimbaC -Wl,-rpath-link,$(BIN_DIR)

# VimbaCPP Dynamic library to link to (and also VimbaC)
VimbaCPP: $(BIN_DIR) 
	test -n "$(VIMBASDK_DIR)" # Make sure $$VIMBASDK_DIR is sets (should point to your sdk dir 'Vimba_2_1/')
	cp $(VIMBASDK_DIR)/VimbaC/DynamicLib/$(ARCH)_$(WORDSIZE)bit/libVimbaC.so $(BIN_DIR)/
	cp $(VIMBASDK_DIR)/VimbaCPP/DynamicLib/$(ARCH)_$(WORDSIZE)bit/libVimbaCPP.so $(BIN_DIR)/


SOURCE_DIR          = src/
INCLUDE_DIRS        = -I$(SOURCE_DIR)

LIBS                = $(VIMBACPP_LIBS) \
                      $(OPENCV_LIBS) \
                      $(BOOST_LIBS) \
		      -lrt \
                      -pthread \

CFLAGS              = -Wall -Wextra -Werror \
                      $(COMMON_CFLAGS) \
                      $(VIMBACPP_CFLAGS) \
                      $(OPENCV_CFLAGS)

OBJ_FILES           = $(OBJ_DIR)/DroneDetector.o \
                      $(OBJ_DIR)/ApiController.o \
                      $(OBJ_DIR)/FrameObserver.o \
                      $(OBJ_DIR)/program.o

DEPENDENCIES        = VimbaCPP

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(OBJ_DIR)
	g++ -c $(INCLUDE_DIRS) $(DEFINES) $(CFLAGS) -o $@ $<

$(BIN_PATH): $(DEPENDENCIES) $(OBJ_FILES) $(BIN_DIR)
	g++ $(ARCH_CFLAGS) -o $(BIN_PATH) $(OBJ_FILES) $(LIBS) -Wl,-rpath,'$$ORIGIN'

clean:
	rm bin -rf
	rm obj -rf

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)
