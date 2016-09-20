find_path(CDS_INCLUDE_DIR cds/version.h)
find_library(CDS_LIBRARY NAMES cds libcds)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CDS DEFAULT_MSG CDS_LIBRARY CDS_INCLUDE_DIR)

mark_as_advanced(CDS_INCLUDE_DIR CDS_LIBRARY)
set(CDS_LIBRARIES ${CDS_LIBRARY})
set(CDS_INCLUDE_DIRS ${CDS_INCLUDE_DIR})
