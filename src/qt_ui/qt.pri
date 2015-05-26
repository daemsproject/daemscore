INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS  +=\  
        $$PWD/bitcoingui.h \    
        $$PWD/bitcoinunits.h \    
        $$PWD/clientmodel.h \    
        $$PWD/guiconstants.h \    
        $$PWD/guiutil.h \
        $$PWD/jsinterface.h \    
        $$PWD/mainview.h \
        $$PWD/networkstyle.h \ 
        $$PWD/notificator.h \
        $$PWD/splashscreen.h \
        $$PWD/userconfirmdialog.h \        
        $$PWD/utilitydialog.h \
        #$$PWD/walletmodel.h \
        $$PWD/webpage.h \
        $$PWD/winshutdownmonitor.h 

SOURCES  +=\  
  $$PWD/browser.cpp \  
  $$PWD/bitcoingui.cpp \
  $$PWD/bitcoinunits.cpp \  
  $$PWD/clientmodel.cpp \      
  $$PWD/guiutil.cpp \   
  $$PWD/jsinterface.cpp \    
  $$PWD/mainview.cpp \  
  $$PWD/networkstyle.cpp \    
  $$PWD/notificator.cpp \
  $$PWD/splashscreen.cpp \ 
  $$PWD/userconfirmdialog.cpp \                  
  $$PWD/utilitydialog.cpp \  
  #$$PWD/walletmodel.cpp \
  $$PWD/webpage.cpp \
  $$PWD/winshutdownmonitor.cpp 
#OTHER_FILES +=$$PWD/paymentrequest.proto
