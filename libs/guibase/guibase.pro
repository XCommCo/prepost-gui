######################################################################
# Automatically generated by qmake (2.01a) ? 10 23 18:49:25 2014
######################################################################

TARGET = iricGuibase
TEMPLATE = lib
INCLUDEPATH += ..

DEFINES += GUIBASE_LIBRARY

QT += widgets xml

include( ../../paths.pri )

######################
# Internal libraries #
######################

# iricMisc

win32 {
	CONFIG(debug, debug|release) {
		LIBS += -L"../misc/debug"
	} else {
		LIBS += -L"../misc/release"
	}
}
unix {
	LIBS += -L"../misc"
}
LIBS += -liricMisc

######################
# External libraries #
######################

# Qwt

win32 {
	CONFIG(debug, debug|release) {
		LIBS += -lqwtd
	}else {
		LIBS += -lqwt
		DEFINES += QT_NO_DEBUG_OUTPUT
		DEFINES += QT_NO_WARNING_OUTPUT
	}
}
unix {
	LIBS += -lqwt
}

# proj.4

win32 {
	LIBS += -lproj_i
}
unix {
	LIBS += -lproj
}

# VTK

LIBS += \
	-lvtkCommonCore-6.1 \
	-lvtkCommonDataModel-6.1 \
	-lvtkCommonExecutionModel-6.1 \
	-lvtkFiltersFlowPaths-6.1 \
	-lvtkInteractionStyle-6.1 \
	-lvtkInteractionWidgets-6.1 \
	-lvtkRenderingAnnotation-6.1 \
	-lvtkRenderingCore-6.1

# Input
HEADERS += colortool.h \
           comboboxtool.h \
           coordinatesystem.h \
           coordinatesystembuilder.h \
           coordinatesystemselectdialog.h \
           cursorchanger.h \
           graphicsmisc.h \
           guibase_global.h \
           irictoolbar.h \
           linestyleinformation.h \
           marginwidget.h \
           mousepositionlabel.h \
           objectbrowser.h \
           objectbrowserview.h \
           pointstyleinformation.h \
           qwtplotcustomcurve.h \
           qwtplotcustommarker.h \
           realnumberdisplaylabel.h \
           realnumbereditwidget.h \
           scalarbardialog.h \
           scalarbarsetting.h \
           scalarbarwidget.h \
           scalarsettingcontainer.h \
           sliderwithvalue.h \
           structuredgridregion.h \
           structuredgridregionselectwidget.h \
           transparencywidget.h \
           vtk2dinteractorstyle.h \
           vtkCustomStreamPoints.h \
           vtkdatasetattributestool.h \
           vtklegendboxrepresentation.h \
           vtklegendboxwidget.h \
           vtksubdividegrid.h \
           vtktextpropertysettingcontainer.h \
           vtktextpropertysettingdialog.h \
           waitdialog.h \
           xyaxisdisplaysettingdialog.h \
           colormap/colormapcustomsetting.h \
           colormap/colormapcustomsettingcolor.h \
           colormap/colormapcustomsettingdialog.h \
           colormap/colormapsettingwidget.h \
           executer/executeri.h \
           executer/executerwatcher.h \
           executer/processexecuter.h \
           executer/silentexecuterwatcher.h \
           executer/threadexecuter.h \
           executer/unziparchiveprocessexecuter.h \
           executer/waitdialogexecuterwatcher.h \
           executer/ziparchiveprocessexecuter.h \
           geometry/pointring.h \
           geometry/polygonregion.h \
           geometry/rect.h \
           private/coordinatesystem_impl.h \
           widget/asciionlylineedit.h \
           widget/asciionlytextedit.h \
           widget/centeredcheckbox.h \
           widget/cgnszoneselectdialog.h \
           widget/coloreditwidget.h \
           widget/contoursettingwidget.h \
           widget/coordinateeditwidget.h \
           widget/dirnameeditwidget.h \
           widget/doublespinboxwithhelp.h \
           widget/filenameeditwidget.h \
           widget/gridshapeeditdialog.h \
           widget/integernumberdisplaylabel.h \
           widget/integernumbereditwidget.h \
           widget/itemselectingdialog.h \
           widget/textediti.h \
           widget/private/itemselectingdialog_detail.h
FORMS += coordinatesystemselectdialog.ui \
         scalarbardialog.ui \
         scalarbarwidget.ui \
         structuredgridregionselectwidget.ui \
         transparencywidget.ui \
         vtktextpropertysettingdialog.ui \
         waitdialog.ui \
         xyaxisdisplaysettingdialog.ui \
         colormap/colormapcustomsettingdialog.ui \
         colormap/colormapsettingwidget.ui \
         widget/cgnszoneselectdialog.ui \
         widget/contoursettingwidget.ui \
         widget/dirnameeditwidget.ui \
         widget/doublespinboxwithhelp.ui \
         widget/filenameeditwidget.ui \
         widget/gridshapeeditdialog.ui \
         widget/itemselectingdialog.ui
SOURCES += colortool.cpp \
           comboboxtool.cpp \
           coordinatesystem.cpp \
           coordinatesystembuilder.cpp \
           coordinatesystemselectdialog.cpp \
           cursorchanger.cpp \
           graphicsmisc.cpp \
           irictoolbar.cpp \
           linestyleinformation.cpp \
           marginwidget.cpp \
           mousepositionlabel.cpp \
           objectbrowser.cpp \
           objectbrowserview.cpp \
           pointstyleinformation.cpp \
           qwtplotcustomcurve.cpp \
           qwtplotcustommarker.cpp \
           realnumberdisplaylabel.cpp \
           realnumbereditwidget.cpp \
           scalarbardialog.cpp \
           scalarbarsetting.cpp \
           scalarbarwidget.cpp \
           scalarsettingcontainer.cpp \
           sliderwithvalue.cpp \
           structuredgridregion.cpp \
           structuredgridregionselectwidget.cpp \
           transparencywidget.cpp \
           vtk2dinteractorstyle.cpp \
           vtkCustomStreamPoints.cxx \
           vtkdatasetattributestool.cpp \
           vtklegendboxrepresentation.cpp \
           vtklegendboxwidget.cpp \
           vtksubdividegrid.cpp \
           vtktextpropertysettingcontainer.cpp \
           vtktextpropertysettingdialog.cpp \
           waitdialog.cpp \
           xyaxisdisplaysettingdialog.cpp \
           colormap/colormapcustomsetting.cpp \
           colormap/colormapcustomsettingcolor.cpp \
           colormap/colormapcustomsettingdialog.cpp \
           colormap/colormapsettingwidget.cpp \
           executer/executerwatcher.cpp \
           executer/processexecuter.cpp \
           executer/silentexecuterwatcher.cpp \
           executer/threadexecuter.cpp \
           executer/unziparchiveprocessexecuter.cpp \
           executer/waitdialogexecuterwatcher.cpp \
           executer/ziparchiveprocessexecuter.cpp \
           geometry/pointring.cpp \
           geometry/polygonregion.cpp \
           geometry/rect.cpp \
           widget/asciionlylineedit.cpp \
           widget/asciionlytextedit.cpp \
           widget/centeredcheckbox.cpp \
           widget/cgnszoneselectdialog.cpp \
           widget/coloreditwidget.cpp \
           widget/contoursettingwidget.cpp \
           widget/coordinateeditwidget.cpp \
           widget/dirnameeditwidget.cpp \
           widget/doublespinboxwithhelp.cpp \
           widget/filenameeditwidget.cpp \
           widget/gridshapeeditdialog.cpp \
           widget/integernumberdisplaylabel.cpp \
           widget/integernumbereditwidget.cpp \
           widget/itemselectingdialog.cpp
RESOURCES += guibase.qrc
TRANSLATIONS += languages/iricGuibase_es_ES.ts \
                languages/iricGuibase_fr_FR.ts \
                languages/iricGuibase_id_ID.ts \
                languages/iricGuibase_ja_JP.ts \
                languages/iricGuibase_ko_KR.ts \
                languages/iricGuibase_ru_RU.ts \
                languages/iricGuibase_th_TH.ts \
                languages/iricGuibase_vi_VN.ts \
                languages/iricGuibase_zh_CN.ts
