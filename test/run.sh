g++ \
    ../src/DroneDetector.h ../src/DroneDetector.cpp \
    Test.cpp \
    -lboost_unit_test_framework \
    -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc\
     -o test \
&& ./test

