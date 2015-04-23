######################################################################
# Automatically generated by qmake (2.01a) ? 10 24 00:36:17 2014
######################################################################

TARGET = iricPostbase
TEMPLATE = lib


DEFINES += POSTBASE_LIBRARY

include( ../../paths.pri )

QT += widgets xml

######################
# Internal libraries #
######################

# iricMisc

CONFIG(debug, debug|release) {
	LIBS += -L"../misc/debug"
} else {
	LIBS += -L"../misc/release"
}
LIBS += -liricMisc

# iricGuibase

CONFIG(debug, debug|release) {
	LIBS += -L"../guibase/debug"
} else {
	LIBS += -L"../guibase/release"
}
LIBS += -liricGuibase

# iricGuicore

CONFIG(debug, debug|release) {
	LIBS += -L"../guicore/debug"
} else {
	LIBS += -L"../guicore/release"
}
LIBS += -liricGuicore

######################
# External libraries #
######################

# VTK

LIBS += \
	-lvtkCommonCore-6.1 \
	-lvtkCommonDataModel-6.1 \
	-lvtkRenderingCore-6.1 \
	-lvtkRenderingFreetype-6.1

# Input
HEADERS += postallzoneselectingdialog.h \
           postbase_global.h \
           postparticlebasicpropertydialog.h \
           time/posttimedataitem.h \
           time/posttimeeditdialog.h \
           time/posttimesetting.h \
           title/posttitledataitem.h \
           title/posttitleeditdialog.h \
           title/posttitlesetting.h
FORMS += postallzoneselectingdialog.ui \
         postparticlebasicpropertydialog.ui \
         time/posttimeeditdialog.ui \
         title/posttitleeditdialog.ui
SOURCES += postallzoneselectingdialog.cpp \
           postparticlebasicpropertydialog.cpp \
           time/posttimedataitem.cpp \
           time/posttimeeditdialog.cpp \
           title/posttitledataitem.cpp \
           title/posttitleeditdialog.cpp
TRANSLATIONS += languages/iricPostbase_es_ES.ts \
                languages/iricPostbase_fr_FR.ts \
                languages/iricPostbase_id_ID.ts \
                languages/iricPostbase_ja_JP.ts \
                languages/iricPostbase_ko_KR.ts \
                languages/iricPostbase_ru_RU.ts \
                languages/iricPostbase_th_TH.ts \
                languages/iricPostbase_vi_VN.ts \
                languages/iricPostbase_zh_CN.ts