# libboost
set(BOOST_VERSION "1.85.0")
set(BOOST_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libboost")
set(TAR_DECOMP_OPTS "--bzip2 -xf")

string(REPLACE "." "_" BOOST_VERSION_INFILE "${BOOST_VERSION}")

include(ExternalProject)
if(NOT ${BUILD_SHARED_LIBS})
  ExternalProject_Add(libboost
    URL https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_INFILE}.tar.bz2
    URL_HASH SHA256=7009fe1faa1697476bdc7027703a2badb84e849b7b0baad5086b087b971f8617
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_PREFIX}
    BUILD_COMMAND ./b2 install --with-regex --with-filesystem link=static
    INSTALL_COMMAND true)

  set(LIBBOOST_FILESYSTEM ${BOOST_PREFIX}/lib/libboost_filesystem.a CACHE INTERNAL "libboost filesystem")
  set(LIBBOOST_REGEX ${BOOST_PREFIX}/lib/libboost_regex.a CACHE INTERNAL "libboost regex")
else()
  if(${ENABLE_UNIT_TESTS})
    ExternalProject_Add(libboost
      URL https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_INFILE}.tar.bz2
      URL_HASH SHA256=7009fe1faa1697476bdc7027703a2badb84e849b7b0baad5086b087b971f8617
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_PREFIX}
      BUILD_COMMAND ./b2 install --with-regex --with-filesystem link=shared
      INSTALL_COMMAND ldconfig ${BOOST_PREFIX}/lib)

    set(LIBBOOST_FILESYSTEM ${BOOST_PREFIX}/lib/libboost_filesystem.so CACHE INTERNAL "libboost filesystem")
    set(LIBBOOST_REGEX ${BOOST_PREFIX}/lib/libboost_regex.so CACHE INTERNAL "libboost regex")
  else()
    ExternalProject_Add(libboost
      URL https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_INFILE}.tar.bz2
      URL_HASH SHA256=7009fe1faa1697476bdc7027703a2badb84e849b7b0baad5086b087b971f8617
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND true
      BUILD_COMMAND cp -r boost ${BOOST_PREFIX}/include
      INSTALL_COMMAND true)
  endif(${ENABLE_UNIT_TESTS})
endif(NOT ${BUILD_SHARED_LIBS})

set(LIBBOOST_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libboost/include PARENT_SCOPE)
