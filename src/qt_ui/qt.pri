INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS  +=\  
        $$PWD/bitcoingui.h \    
        $$PWD/bitcoinunits.h \    
        $$PWD/guiconstants.h \    
        $$PWD/guiutil.h \
        $$PWD/jsinterface.h \    
        $$PWD/mainframe.h \
        $$PWD/mainview.h \
        $$PWD/networkstyle.h \ 
        $$PWD/notificator.h \        
        $$PWD/utilitydialog.h \
        #$$PWD/walletmodel.h \
        $$PWD/walletpage.h \
        $$PWD/winshutdownmonitor.h 

SOURCES  +=\  
  $$PWD/browser.cpp \  
  $$PWD/bitcoingui.cpp \
  $$PWD/bitcoinunits.cpp \    
  $$PWD/guiutil.cpp \   
  $$PWD/jsinterface.cpp \    
  $$PWD/mainframe.cpp \
  $$PWD/mainview.cpp \  
  $$PWD/networkstyle.cpp \    
  $$PWD/notificator.cpp \   
  $$PWD/utilitydialog.cpp \  
  #$$PWD/walletmodel.cpp \
  $$PWD/walletpage.cpp \
  $$PWD/winshutdownmonitor.cpp 
#OTHER_FILES +=$$PWD/paymentrequest.proto
