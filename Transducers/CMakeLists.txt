SET (TRANSDUCERS_SOURCES 

)

SET (TRANSDUCERS_TEST
  test/data_structures/pushdown_state_map_test.cpp
  test/transducers/aggregation/symbol_buffer_test.cpp
  test/transducers/numeric/multiply_test.cpp
  test/transducers/pushdown/state_map_pushdown_transducer_test.cpp
  test/transducers/util/match_adapter_test.cpp
)

ADD_LIBRARY(TransducersTest SHARED ${TRANSDUCERS_TEST})
