TARGET = iricTmsLoader
TEMPLATE = lib
QT       += core gui webkitwidgets network

DEFINES += TMSLOADER_LIBRARY

include( ../../paths.pri )

# Input
HEADERS += tmsloader.h \
           tmsloader_api.h \
           tmsloadertester.h \
           tmsrequest.h \
           tmsrequestbing.h \
           tmsrequestgooglemap.h \
           tmsrequestgsi.h \
           tmsrequestopenstreetmap.h \
           tmsrequestxyz.h \
           private/tmsloader_impl.h \
           private/tmsrequest_impl.h \
           private/tmsrequestbing_impl.h \
           private/tmsrequestgooglemap_impl.h \
           private/tmsrequestgsi_impl.h \
           private/tmsrequesthandler.h \
           private/tmsrequesthandlerbing.h \
           private/tmsrequesthandlergooglemap.h \
           private/tmsrequesthandleropenstreetmap.h \
           private/tmsrequesthandlerxyz.h \
           private/tmsrequestxyz_impl.h
SOURCES += tmsloader.cpp \
           tmsloadertester.cpp \
           tmsrequest.cpp \
           tmsrequestbing.cpp \
           tmsrequestgooglemap.cpp \
           tmsrequestgsi.cpp \
           tmsrequestopenstreetmap.cpp \
           tmsrequestxyz.cpp \
           private/tmsrequesthandler.cpp \
           private/tmsrequesthandlerbing.cpp \
           private/tmsrequesthandlergooglemap.cpp \
           private/tmsrequesthandleropenstreetmap.cpp \
           private/tmsrequesthandlerxyz.cpp
RESOURCES += tmsloader.qrc