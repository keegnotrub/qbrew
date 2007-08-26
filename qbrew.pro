# Copyright 2007 David Johnson
# This project file is free software; the author gives unlimited
# permission to copy, distribute and modify it.

TARGET = qbrew
TEMPLATE = app
CONFIG += qt warn_on
QT += xml

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build
UI_DIR = build

INCLUDEPATH += src
VPATH = src

win32 {
    RC_FILE = win\icon.rc
    CONFIG -= console

    target.path = release
    INSTALLS += target

    data.path = release
    data.files = data/* pics/splash.png win/qbrew.ico README LICENSE
    INSTALLS += data

    doc.path = release/doc
    doc.files = docs/book/*.html docs/primer/*.html
    INSTALLS += doc

    trans.path = translations
    trans.files = translations/*.qm
    INSTALLS += trans
}

macx {
    CONFIG += x86 ppc
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk

    DEFINES += HAVE_ROUND
    LIBS += -dead-strip
    QMAKE_POST_LINK=strip qbrew.app/Contents/MacOS/qbrew

    # install into app bundle
    data.path = qbrew.app/Contents/Resources
    data.files = data/* pics/splash.png mac/application.icns mac/document.icns
    INSTALLS += data

    misc.path = qbrew.app/Contents
    misc.files = mac/Info.plist mac/PkgInfo
    INSTALLS += misc

    doc.path = qbrew.app/Contents/Resources/en.lproj
    doc.files = docs/book/*.html docs/primer/*.html
    INSTALLS += doc

    trans.path = qbrew.app/Contents/Resources/translations
    trans.files = translations/*.qm
    INSTALLS += trans
}

unix:!macx {
    configure {
        DEFINES += $$(HAVEDEFS)
        target.path = $$(BINDIR)
        data.path = $$(DATADIR)
        doc.path = $$(DOCDIR)
        trans.path = $$(DATADIR)/translations
    } else {
        target.path = /usr/local/bin
        data.path = /usr/local/share/qbrew
        doc.path = /usr/local/share/doc/qbrew
        trans.path = /usr/local/share/qbrew/translations
    }

    data.files = data/* pics/splash.png
    doc.files = docs/book/*.html docs/primer/*.html README LICENSE
    trans.files = translations/*.qm
    INSTALLS += target data doc trans
}

RESOURCES = qbrew.qrc

HEADERS = alcoholtool.h \
	qbrew.h \
	configstate.h \
	configure.h \
	data.h \
	databasetool.h \
	grain.h \
	graindelegate.h \
	grainmodel.h \
	helpviewer.h \
	hop.h \
	hopdelegate.h \
	hopmodel.h \
	hydrometertool.h \
        ingredientview.h \
	misc.h \
	miscdelegate.h \
	miscmodel.h \
	notepage.h \
	previewdialog.h \
	quantity.h \
	recipe.h \
	resource.h \
	style.h \
	styledelegate.h \
	stylemodel.h \
	textprinter.h \
	view.h

SOURCES = alcoholtool.cpp \
	qbrew.cpp \
	configure.cpp \
	data.cpp \
	databasetool.cpp \
	export.cpp \
	grain.cpp \
	graindelegate.cpp \
	grainmodel.cpp \
	helpviewer.cpp \
	hop.cpp \
	hopdelegate.cpp \
	hopmodel.cpp \
	hydrometertool.cpp \
        ingredientview.cpp \
	main.cpp \
	misc.cpp \
	miscdelegate.cpp \
	miscmodel.cpp \
	notepage.cpp \
	previewdialog.cpp \
	quantity.cpp \
	recipe.cpp \
	style.cpp \
	styledelegate.cpp \
	stylemodel.cpp \
	textprinter.cpp \
	view.cpp

FORMS = alcoholtool.ui \
	calcconfig.ui \
	databasetool.ui \
	generalconfig.ui \
	helpviewer.ui \
	hydrometertool.ui \
        ingredientpage.ui \
	mainwindow.ui \
	noteview.ui \
	recipeconfig.ui \
	view.ui

TRANSLATIONS = translations/qbrew_de.ts