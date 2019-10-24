project(Assets)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( Assets Assets Assets.cpp )
target_include_directories( Assets PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( Assets ${CMAKE_SOURCE_DIR}/../ricardian )
