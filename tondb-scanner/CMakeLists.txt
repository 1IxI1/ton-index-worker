cmake_minimum_required(VERSION 3.16)

add_library(tondb-scanner STATIC
    src/InsertManager.cpp
    src/InsertManagerBase.cpp
    src/DbScanner.cpp
    src/DataParser.cpp
    src/TraceAssembler.cpp
    src/EventProcessor.cpp
    # src/EventProcessor2.cpp
    src/queue_state.cpp
    src/parse_token_data.cpp
    src/parse_contract_methods.cpp
    src/convert-utils.cpp
    src/tokens-tlb.cpp
    src/Statistics.cpp
    src/smc-interfaces/Tokens.cpp
    src/smc-interfaces/NftSale.cpp
    src/smc-interfaces/execute-smc.cpp
)

target_include_directories(tondb-scanner 
    PUBLIC src/
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/ton/crypto/src
)

target_compile_features(tondb-scanner PRIVATE cxx_std_20)
target_link_libraries(tondb-scanner overlay tdutils tdactor adnl tl_api dht
        catchain validatorsession validator-disk ton_validator validator-disk smc-envelope
        pqxx msgpack-cxx)

set(TLB_TOKENS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tokens-tlb.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/tokens-tlb.h
)

add_custom_command(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMAND tlbc -o tokens-tlb -n tokens::gen -z tlb/tokens.tlb
    COMMENT "Generate tokes tlb source files"
    OUTPUT ${TLB_TOKENS}
    DEPENDS tlbc src/tlb/tokens.tlb
)

add_custom_target(tlb_generate_tokens DEPENDS ${TLB_TOKENS})
add_dependencies(tondb-scanner tlb_generate_tokens)
