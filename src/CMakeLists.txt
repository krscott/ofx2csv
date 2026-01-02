add_library(ofx2csv ofx2csv.c)

target_include_directories(ofx2csv PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_sources(ofx2csv PRIVATE file.c)

install(TARGETS ofx2csv)
