project "protobuf"
  language    "C++"
  kind        "StaticLib"
  --warnings    "off"

files
{
-- sopy-paste from libprotobuf_lite_la_SOURCES variable of the makefile
-- this is from version 3.5.0
"src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc",
"src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc",
"src/google/protobuf/stubs/bytestream.cc",
"src/google/protobuf/stubs/bytestream.h",
"src/google/protobuf/stubs/common.cc",
"src/google/protobuf/stubs/hash.h",
"src/google/protobuf/stubs/int128.cc",
"src/google/protobuf/stubs/int128.h",
"src/google/protobuf/stubs/io_win32.cc",
"src/google/protobuf/stubs/io_win32.h",
"src/google/protobuf/stubs/map_util.h",
"src/google/protobuf/stubs/mathutil.h",
"src/google/protobuf/stubs/once.cc",
"src/google/protobuf/stubs/shared_ptr.h",
"src/google/protobuf/stubs/status.cc",
"src/google/protobuf/stubs/status.h",
"src/google/protobuf/stubs/status_macros.h",
"src/google/protobuf/stubs/statusor.cc",
"src/google/protobuf/stubs/statusor.h",
"src/google/protobuf/stubs/stringpiece.cc",
"src/google/protobuf/stubs/stringpiece.h",
"src/google/protobuf/stubs/stringprintf.cc",
"src/google/protobuf/stubs/stringprintf.h",
"src/google/protobuf/stubs/structurally_valid.cc",
"src/google/protobuf/stubs/strutil.cc",
"src/google/protobuf/stubs/strutil.h",
"src/google/protobuf/stubs/time.cc",
"src/google/protobuf/stubs/time.h",
"src/google/protobuf/arena.cc",
"src/google/protobuf/arenastring.cc",
"src/google/protobuf/extension_set.cc",
"src/google/protobuf/generated_message_util.cc",
"src/google/protobuf/generated_message_table_driven_lite.h",
"src/google/protobuf/generated_message_table_driven_lite.cc",
"src/google/protobuf/message_lite.cc",
"src/google/protobuf/repeated_field.cc",
"src/google/protobuf/wire_format_lite.cc",
"src/google/protobuf/io/coded_stream.cc",
"src/google/protobuf/io/coded_stream_inl.h",
"src/google/protobuf/io/zero_copy_stream.cc",
"src/google/protobuf/io/zero_copy_stream_impl_lite.cc",

-- copy-paste from libprotobuf_la_SOURCES
-- this is addition for full version

--"google/protobuf/any.pb.cc"
--"google/protobuf/api.pb.cc"
--"google/protobuf/stubs/mathlimits.cc"
--"google/protobuf/stubs/mathlimits.h"
--"google/protobuf/any.cc"
--"google/protobuf/descriptor.cc"
--"google/protobuf/descriptor_database.cc"
--"google/protobuf/descriptor.pb.cc"
--"google/protobuf/duration.pb.cc"
--"google/protobuf/dynamic_message.cc"
--"google/protobuf/empty.pb.cc"
--"google/protobuf/extension_set_heavy.cc"
--"google/protobuf/field_mask.pb.cc"
--"google/protobuf/generated_message_reflection.cc"
--"google/protobuf/generated_message_table_driven_lite.h"
--"google/protobuf/generated_message_table_driven.cc"
--"google/protobuf/map_field.cc"
--"google/protobuf/message.cc"
--"google/protobuf/reflection_internal.h"
--"google/protobuf/reflection_ops.cc"
--"google/protobuf/service.cc"
--"google/protobuf/source_context.pb.cc"
--"google/protobuf/struct.pb.cc"
--"google/protobuf/stubs/substitute.cc"
--"google/protobuf/stubs/substitute.h"
--"google/protobuf/text_format.cc"
--"google/protobuf/timestamp.pb.cc"
--"google/protobuf/type.pb.cc"
--"google/protobuf/unknown_field_set.cc"
--"google/protobuf/wire_format.cc"
--"google/protobuf/wrappers.pb.cc"
--"google/protobuf/io/gzip_stream.cc"
--"google/protobuf/io/printer.cc"
--"google/protobuf/io/strtod.cc"
--"google/protobuf/io/tokenizer.cc"
--"google/protobuf/io/zero_copy_stream_impl.cc"
--"google/protobuf/compiler/importer.cc"
--"google/protobuf/compiler/parser.cc"
--"google/protobuf/util/delimited_message_util.cc"
--"google/protobuf/util/field_comparator.cc"
--"google/protobuf/util/field_mask_util.cc"
--"google/protobuf/util/internal/constants.h"
--"google/protobuf/util/internal/datapiece.cc"
--"google/protobuf/util/internal/datapiece.h"
--"google/protobuf/util/internal/default_value_objectwriter.cc"
--"google/protobuf/util/internal/default_value_objectwriter.h"
--"google/protobuf/util/internal/error_listener.cc"
--"google/protobuf/util/internal/error_listener.h"
--"google/protobuf/util/internal/expecting_objectwriter.h"
--"google/protobuf/util/internal/field_mask_utility.cc"
--"google/protobuf/util/internal/field_mask_utility.h"
--"google/protobuf/util/internal/json_escaping.cc"
--"google/protobuf/util/internal/json_escaping.h"
--"google/protobuf/util/internal/json_objectwriter.cc"
--"google/protobuf/util/internal/json_objectwriter.h"
--"google/protobuf/util/internal/json_stream_parser.cc"
--"google/protobuf/util/internal/json_stream_parser.h"
--"google/protobuf/util/internal/location_tracker.h"
--"google/protobuf/util/internal/mock_error_listener.h"
--"google/protobuf/util/internal/object_location_tracker.h"
--"google/protobuf/util/internal/object_source.h"
--"google/protobuf/util/internal/object_writer.cc"
--"google/protobuf/util/internal/object_writer.h"
--"google/protobuf/util/internal/protostream_objectsource.cc"
--"google/protobuf/util/internal/protostream_objectsource.h"
--"google/protobuf/util/internal/protostream_objectwriter.cc"
--"google/protobuf/util/internal/protostream_objectwriter.h"
--"google/protobuf/util/internal/proto_writer.cc"
--"google/protobuf/util/internal/proto_writer.h"
--"google/protobuf/util/internal/structured_objectwriter.h"
--"google/protobuf/util/internal/type_info.cc"
--"google/protobuf/util/internal/type_info.h"
--"google/protobuf/util/internal/type_info_test_helper.cc"
--"google/protobuf/util/internal/type_info_test_helper.h"
--"google/protobuf/util/internal/utility.cc"
--"google/protobuf/util/internal/utility.h"
--"google/protobuf/util/json_util.cc"
--"google/protobuf/util/message_differencer.cc"
--"google/protobuf/util/time_util.cc"
--"google/protobuf/util/type_resolver_util.cc"
}

includedirs {
  "src",
}

defines{
  "LANG_CXX11",
  "HAVE_PTHREAD",
}



