add_executable(ofx2csv-app main.c)
set_target_properties(ofx2csv-app PROPERTIES OUTPUT_NAME "ofx2csv")

target_link_libraries(ofx2csv-app ofx2csv)

install(TARGETS ofx2csv-app)
