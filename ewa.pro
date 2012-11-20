TEMPLATE            += app
QT                  += network webkit
TARGET              = ewa
DESTDIR             = ./
CONFIG              += debug_and_release
#QMAKE_CXXFLAGS_RELEASE -= g
win32{
                    RC_FILE = src/ui/ewa.rc
}
unix{
                    LIBS += -L/usr/lib -lX11
}
 
MAJOR               = 0
MINOR               = 9
VERSION             = $${MAJOR}.$${MINOR}

CONFIG(debug, debug|release){
                    mac: TARGET = $$join(TARGET,,,_debug)
                    win32|unix: TARGET = $$join(TARGET,,,d)
                    OUTDIR = ./tmp/debug
                    DEFINES += EWA_DBG
                    CONFIG += warn_on
}else{
                    OUTDIR = ./tmp/release
                    DEFINES -= EWA_DBG
                    CONFIG -= warn_on
}
     
OBJECTS_DIR         = $$OUTDIR/obj
UI_DIR              = $$OUTDIR/uic
RCC_DIR             = $$OUTDIR/rcc
MOC_DIR             = $$OUTDIR/moc

INCLUDEPATH         += . \
                    src \
                    src/app \
                    src/message \
                    src/network \
                    src/site \
                    src/site/editor \
                    src/site/model \
                    src/wizard \
                    src/statistic \
                    src/version

HEADERS             += src/app/ewa_aboutdialog.h \
                    #src/app/ewa_sexytabshower.h \
                    src/app/ewa_faderwidget.h \
                    src/app/ewa_application.h \
                    src/app/ewa_updateschecker.h \
                    src/app/ewa_mainwindow.h \
                    src/app/ewa_terminal.h \
                    src/app/ewa_timer.h \
                    src/app/ewa_trayiconmanager.h \
                    #src/app/ewa_webkitsettings.h \
                    src/app/ewa_uacombobox.h \
                    src/app/ewa_splashscreen.h \
                    src/app/ewa_object.h \
                    src/app/ewa_htmlselector.h \
                    src/app/ewa_htmlselectordlg.h \
                    src/app/ewa_sitethumbnailfilesystem.h \
                    src/app/ewa_sitethumbnailfilehandler.h \
                    src/app/ewa_settings.h \
                    src/app/ewa_xmessagebox.h \
                    src/version/ewa_versiondef.h \
                    src/message/ewa_textlabel.h \
                    src/message/ewa_sitewidgetlabel.h \
                    src/message/ewa_sysmsgsettings.h \
                    src/message/ewa_sitewidgetsettingsdlg.h \
                    src/message/ewa_sitewidget.h \
                    src/message/ewa_sitewidgettitlewidget.h \
                    src/message/ewa_colordialog.h \
                    src/message/ewa_colorchooser.h \
                    src/message/ewa_sitewidgetcolormanager.h \
                    src/message/ewa_downloaddlg.h \
                    src/network/ewa_cookiesjar.h \
                    src/network/ewa_networkaccessmanager.h \
                    src/network/ewa_siteroutepoint.h \
                    src/network/ewa_httppocketscollection.h \
                    src/network/ewa_webpage.h \
                    src/network/ewa_webview.h \
                    src/network/ewa_useractionscollection.h \
                    src/network/ewa_useraction.h \
                    src/network/ewa_useractionseditordlg.h \
                    src/network/ewa_faviconloader.h \
                    src/network/ewa_useractionsprocessor.h \
                    src/network/ewa_useractionsrecorder.h \
                    src/network/ewa_useractionsplayer.h \
                    src/site/editor/ewa_rxdlg.h \
                    src/site/editor/ewa_textsearch.h \
                    src/site/editor/ewa_siteeditordlg.h \
                    src/site/editor/ewa_simplesiteproperties.h \
                    src/site/editor/ewa_siteeditordraft.h \
                    src/site/model/ewa_sitedelegatebase.h \
                    src/site/model/ewa_siteactiondelegate.h \
                    src/site/model/ewa_sitefavicondelegate.h \
                    src/site/model/ewa_siteslistmodel.h \
                    src/site/model/ewa_siteslistmodeldelegate.h \
                    src/site/model/ewa_sitesmodelview.h \
                    src/site/model/ewa_siteusagedelegate.h \
                    src/site/ewa_sitedrawer.h \
                    src/site/ewa_sitehandle.h \
                    src/site/ewa_parsingrule.h \
                    src/site/ewa_parsingrulecollection.h \
                    src/site/ewa_siteslistmanager.h \
                    src/site/ewa_showpolicy.h \
                    src/statistic/ewa_sendstatisticdlg.h \
                    src/statistic/ewa_statisticmodelview.h \
                    src/statistic/ewa_buzzer.h \
                    src/statistic/ewa_feedbackdlg.h \
                    src/wizard/ewa_addsitewizard.h \
                    src/wizard/ewa_aswpage_greeting.h \
                    src/wizard/ewa_aswpage_inputurl.h \
                    src/wizard/ewa_sitehandle_wizardpage.h

SOURCES             += src/app/ewa_aboutdialog.cpp \
                    #src/app/ewa_sexytabshower.cpp \
                    src/app/ewa_faderwidget.cpp \
                    src/app/ewa_application.cpp \
                    src/app/ewa_updateschecker.cpp \
                    src/app/ewa_mainwindow.cpp \
                    src/app/ewa_terminal.cpp \
                    src/app/ewa_timer.cpp \
                    src/app/ewa_trayiconmanager.cpp \
                    #src/app/ewa_webkitsettings.cpp \
                    src/app/ewa_uacombobox.cpp \
                    src/app/ewa_splashscreen.cpp \
                    src/app/ewa_object.cpp \
                    src/app/ewa_htmlselector.cpp \
                    src/app/ewa_htmlselectordlg.cpp \
                    src/app/ewa_sitethumbnailfilesystem.cpp \
                    src/app/ewa_sitethumbnailfilehandler.cpp \
                    src/app/ewa_settings.cpp \
                    src/app/ewa_xmessagebox.cpp \
                    src/message/ewa_textlabel.cpp \
                    src/message/ewa_sitewidgetlabel.cpp \
                    src/message/ewa_sysmsgsettings.cpp \
                    src/message/ewa_sitewidgetsettingsdlg.cpp \
                    src/message/ewa_sitewidget.cpp \
                    src/message/ewa_sitewidgettitlewidget.cpp \
                    src/message/ewa_colordialog.cpp \
                    src/message/ewa_colorchooser.cpp \
                    src/message/ewa_sitewidgetcolormanager.cpp \
                    src/message/ewa_downloaddlg.cpp \
                    src/network/ewa_cookiesjar.cpp \
                    src/network/ewa_networkaccessmanager.cpp \
                    src/network/ewa_siteroutepoint.cpp \
                    src/network/ewa_httppocketscollection.cpp \
                    src/network/ewa_webpage.cpp \
                    src/network/ewa_webview.cpp \
                    src/network/ewa_useractionscollection.cpp \
                    src/network/ewa_useraction.cpp \
                    src/network/ewa_useractionseditordlg.cpp \
                    src/network/ewa_faviconloader.cpp \
                    src/network/ewa_useractionsprocessor.cpp \
                    src/network/ewa_useractionsrecorder.cpp \
                    src/network/ewa_useractionsplayer.cpp \
                    src/site/editor/ewa_rxdlg.cpp \
                    src/site/editor/ewa_textsearch.cpp \
                    src/site/editor/ewa_siteeditordlg.cpp \
                    src/site/editor/ewa_simplesiteproperties.cpp \
                    src/site/editor/ewa_siteeditordraft.cpp \
                    src/site/model/ewa_sitedelegatebase.cpp \
                    src/site/model/ewa_siteactiondelegate.cpp \
                    src/site/model/ewa_sitefavicondelegate.cpp \
                    src/site/model/ewa_siteslistmodel.cpp \
                    src/site/model/ewa_siteslistmodeldelegate.cpp \
                    src/site/model/ewa_sitesmodelview.cpp \
                    src/site/model/ewa_siteusagedelegate.cpp \
                    src/site/ewa_sitedrawer.cpp \
                    src/site/ewa_sitehandle.cpp \
                    src/site/ewa_parsingrule.cpp \
                    src/site/ewa_parsingrulecollection.cpp \
                    src/site/ewa_siteslistmanager.cpp \
                    src/site/ewa_showpolicy.cpp \
                    src/statistic/ewa_sendstatisticdlg.cpp \
                    src/statistic/ewa_statisticmodelview.cpp \
                    src/statistic/ewa_buzzer.cpp \
                    src/statistic/ewa_feedbackdlg.cpp \
                    src/wizard/ewa_addsitewizard.cpp \
                    src/wizard/ewa_aswpage_greeting.cpp \
                    src/wizard/ewa_aswpage_inputurl.cpp \
                    src/wizard/ewa_sitehandle_wizardpage.cpp \
                    src/main.cpp

FORMS               = src/ui/ewa_mainwindow.ui \
                    src/ui/ewa_siteslistmanager.ui \
                    src/ui/ewa_aswpage_greeting.ui \
                    src/ui/ewa_aswpage_inputurl.ui \
                    src/ui/ewa_colorchooser.ui \
                    src/ui/ewa_colordialog.ui \
                    src/ui/ewa_feedbackdlg.ui \
                    src/ui/ewa_sitehandle_wizardpage.ui \
                    src/ui/ewa_sysmsgsettings.ui \
                    src/ui/ewa_sitewidgetsettingsdlg.ui \
                    src/ui/ewa_siteeditordlg.ui \
                    src/ui/ewa_rxdlg.ui \
                    src/ui/ewa_textsearch.ui \
                    src/ui/ewa_about.ui \
                    src/ui/ewa_sitewidgettitlewidget.ui \
                    #src/ui/ewa_webkitsettings.ui \
                    src/ui/ewa_simplesiteproperties.ui \
                    src/ui/ewa_sitewidgetcolormanager.ui \
                    src/ui/ewa_useractionseditordlg.ui \
                    src/ui/ewa_autoregexpdlg.ui \
                    src/ui/ewa_download.ui \
                    src/ui/ewa_htmlselectordlg.ui \
                    src/ui/ewa_sendstatisticdlg.ui \
                    src/ui/ewa_siteeditordraft.ui \
                    src/ui/ewa_xmessagebox.ui \
                    src/ui/ewa_pointeditordlg.ui

mac{ 
    HEADERS += src/message/ewa_sitewidgetmacos.h
    SOURCES += src/message/ewa_sitewidgetmacos.cpp
}

win32{ 
    HEADERS += src/message/ewa_sitewidgetwindows.h
    SOURCES += src/message/ewa_sitewidgetwindows.cpp
}

unix{ 
    HEADERS += src/message/ewa_sitewidgetlinux.h
    SOURCES += src/message/ewa_sitewidgetlinux.cpp
}
                    
RESOURCES           = src/ui/ewa.qrc

TRANSLATIONS        = ./localizations/ewa_ru.ts ./localizations/ewa_en.ts

CODECFORTR          = UTF-8
CODECFORSOURCES     = UTF-8
