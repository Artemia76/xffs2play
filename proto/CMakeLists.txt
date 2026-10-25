# Protobuf messages shared with the FFS2Play client (see docs/contracts/airdata-proto.md in ffs2-suite).
# Generated headers are reachable as "proto/aircraftstate.pb.h".

add_library(xffs2play_proto STATIC aircraftstate.proto)

set(_proto_gen_dir "${PROJECT_BINARY_DIR}/gen/proto")
file(MAKE_DIRECTORY "${_proto_gen_dir}")

protobuf_generate(TARGET xffs2play_proto
                  LANGUAGE cpp
                  IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
                  PROTOC_OUT_DIR "${_proto_gen_dir}")

target_include_directories(xffs2play_proto PUBLIC "${PROJECT_BINARY_DIR}/gen")
target_link_libraries(xffs2play_proto PUBLIC protobuf::libprotobuf)
