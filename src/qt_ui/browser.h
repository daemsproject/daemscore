

#ifndef FAI_BROWSER_H
#define	FAI_BROWSER_H
/** Class encapsulating Bitcoin Core startup and shutdown.
 * Allows running startup and shutdown in a different thread from the UI thread.
 */
#include <QApplication>
#include <boost/thread.hpp>
#include <QThread>
#include <QTimer>
#include "clientmodel.h"
#include "walletmodel.h"
#include "bitcoingui.h"
#include "networkstyle.h"

class BitcoinCore: public QObject
{
    Q_OBJECT
public:
    explicit BitcoinCore();

public slots:
    void initialize();
    void shutdown();

signals:
    void initializeResult(int retval);
    void shutdownResult(int retval);
    void runawayException(const QString &message);

private:
    boost::thread_group threadGroup;

    /// Pass fatal exception message to UI thread
    void handleRunawayException(std::exception *e);
};

/** Main Bitcoin application object */
class BitcoinApplication: public QApplication
{
    Q_OBJECT
public:
    explicit BitcoinApplication(int &argc, char **argv);
    ~BitcoinApplication();

    static BitcoinApplication *instance();
    
    /// Create options model
    //void createOptionsModel();
    /// Create main window
    void createWindow(const NetworkStyle *networkStyle,QString languange);
    /// Create splash screen
    void createSplashScreen(const NetworkStyle *networkStyle);

    /// Request core initialization
    void requestInitialize();
    /// Request core shutdown
    void requestShutdown();
    BitcoinGUI *getWindow(){return window;}
    /// Get process return value
    int getReturnValue() { return returnValue; }

    /// Get window identifier of QMainWindow (BitcoinGUI)
    WId getMainWinId() const;
    void loadSettings();
public slots:
    void initializeResult(int retval);
    void shutdownResult(int retval);
    /// Handle runaway exceptions. Shows a message box with the problem and quits the program.
    void handleRunawayException(const QString &message);

signals:
    void requestedInitialize();
    void requestedShutdown();
    void stopThread();
    void splashFinished(QWidget *window);

private:
    QThread *coreThread;
    //OptionsModel *optionsModel;
    ClientModel *clientModel;
    BitcoinGUI *window;
    QTimer *pollShutdownTimer;
#ifdef ENABLE_WALLET    
    WalletModel *walletModel;
#endif
    int returnValue;

    void startThread();
};


#endif	/* FAI_BROWSER_H */

