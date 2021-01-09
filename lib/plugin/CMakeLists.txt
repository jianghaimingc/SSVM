# SPDX-License-Identifier: Apache-2.0

add_library(ssvmPlugin
  plugin.cpp
)

target_link_libraries(ssvmPlugin
  PUBLIC
  ssvmCommon
  ssvmLoaderFileMgr
)

target_include_directories(ssvmPlugin
  PUBLIC
  ${Boost_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/thirdparty
)
