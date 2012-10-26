project(dolphin-plugin-owncloud)

find_package(KDE4 REQUIRED)
find_package(LibKonq REQUIRED)
include(KDE4Defaults)

include_directories(${QT_INCLUDES} ${LIBKONQ_INCLUDE_DIR})

set(dolphin-plugin-owncloud_SRCS dolphin-plugin-owncloud.cpp)
kde4_add_plugin(dolphin-plugin-owncloud ${dolphin-plugin-owncloud_SRCS})
target_link_libraries(dolphin-plugin-owncloud ${KDE4_KIO_LIBS} ${LIBKONQ_LIBRARY})

install(FILES dolphin-plugin-owncloud.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS dolphin-plugin-owncloud DESTINATION ${PLUGIN_INSTALL_DIR})
