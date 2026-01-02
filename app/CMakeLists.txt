add_executable(ofx2csv-app main.c)

target_link_libraries(ofx2csv-app ofx2csv)

install(TARGETS ofx2csv-app)
