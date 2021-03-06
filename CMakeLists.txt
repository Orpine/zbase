cmake_minimum_required(VERSION 3.3)
project(zbase)
find_package(BISON REQUIRED)
set(BisionOutput yy.tab.cpp)
add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/yy.tab.cpp
        COMMAND ${BISON_EXECUTABLE} -d --output=${CMAKE_SOURCE_DIR}/yy.tab.cpp ${CMAKE_SOURCE_DIR}/yy.y
)
find_package(FLEX REQUIRED)
add_custom_command(
        OUTPUT ${CMAKE_SOURCE_DIR}/lex.yy.cpp
        COMMAND ${FLEX_EXECUTABLE} --outfile=${CMAKE_SOURCE_DIR}/lex.yy.cpp ${CMAKE_SOURCE_DIR}/lex.l
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES PF.h zbase.h PF_Manager.cpp PF_BufferManager.cpp PF_BufferManager.h PF_HashTable.cpp PF_HashTable.h PF_PageHandle.cpp PF_FileHandle.cpp QL.h SM.h SM.cpp RM.h IX.h IX.cpp RM.cpp parser.h common.h zbase.cpp parser.cpp B_Tree.cpp B_Tree.h QL_Manager.cpp QL_Scan.cpp QL_Scan.h Printer.cpp CatRecord.cpp)
set(PARSER_FILES lex.yy.cpp yy.tab.cpp)

set(PF_TEST_FILES PF.h PF_Manager.cpp PF_BufferManager.cpp PF_BufferManager.h PF_HashTable.cpp PF_HashTable.h PF_PageHandle.cpp PF_FileHandle.cpp zbase.h)

set(SM_TEST_FILES IX.h IX.cpp RM.cpp RM.h SM.cpp SM.h B_Tree.cpp B_Tree.h SM_test.cpp CatRecord.cpp QL.h QL_Manager.cpp QL_Scan.cpp QL_Scan.h Printer.cpp)

set(IX_TEST_FILES IX.h IX.cpp  B_Tree.cpp B_Tree.h IX_test.cpp RM.cpp RM.h)

set(RM_TEST_FILES RM.cpp RM.h RM_test.cpp)


add_executable(zbase ${SOURCE_FILES} ${PARSER_FILES})
add_executable(pf_test ${PF_TEST_FILES})
add_executable(sm_test ${SM_TEST_FILES} ${PF_TEST_FILES})
add_executable(ix_test ${IX_TEST_FILES} ${PF_TEST_FILES})
add_executable(rm_test ${RM_TEST_FILES} ${PF_TEST_FILES})
