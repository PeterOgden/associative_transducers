SET (TRANSDUCERS_SOURCES 

)

SET (TRANSDUCERS_TEST
  test/transducers/aggregation/symbol_buffer_test.cpp
  test/transducers/numeric/multiply_test.cpp
)

ADD_LIBRARY(TransducersTest SHARED ${TRANSDUCERS_TEST})
