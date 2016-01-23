ADD_EXECUTABLE (TestRunner src/TestRunner.cpp)

TARGET_LINK_LIBRARIES(TestRunner cppunit dl ${TEST_LIBS})

ADD_CUSTOM_TARGET(test ${CMAKE_CURRENT_BINARY_DIR}/TestRunner DEPENDS TestRunner)
