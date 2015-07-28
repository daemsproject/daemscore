// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mainview.h"

//#include "addressbookpage.h"
//#include "askpassphrasedialog.h"
#include "bitcoingui.h"
//#include "clientmodel.h"
#include "guiutil.h"
//#include "optionsmodel.h"
#include "webpage.h"
//#include "overviewpage.h"
///#include "receivecoinsdialog.h"
//#include "sendcoinsdialog.h"
//#include "signverifymessagedialog.h"
//#include "transactiontablemodel.h"
//#include "transactionview.h"
#include "walletmodel.h"
#include "jsinterface.h"
#include "ui_interface.h"
#include "ccc/filepackage.h"
#include "ccc/settings.h"
#include "json/json_spirit_reader_template.h"
#include "util.h"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
using namespace boost::filesystem;
MainView::MainView(QString languageIn,BitcoinGUI *parent,JsInterface *_js):
    QStackedWidget(parent),
        language(languageIn),
        jsInterface(_js)
    //clientModel(0)
    //walletModel(0)
{
    // Create tabs    
    //QUrl walletUrl= QUrl("file:///home/alan/projects/ccc/src/qt_ui/res/html/wallet_en.html"); 
    //QUrl walletUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/wallet_en.html"); 
    //QUrl browserUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/browser_en.html"); 
    //LogPrintf(QDir::currentPath().toUtf8() + "/res/html/wallet_en.html");
    
    //vWebPages.push_back(new WebPage(language,parent,jsInterface,walletUrl,1));    
    //vWebPages.push_back(new WebPage(language,this,jsInterface,browserUrl,2));  
    
    //overviewPage = new OverviewPage();

    //transactionsPage = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout();
    QHBoxLayout *hbox_buttons = new QHBoxLayout();
    //transactionView = new TransactionView(this);
    //vbox->addWidget(transactionView);
    QPushButton *exportButton = new QPushButton(tr("&Export"), this);
    exportButton->setToolTip(tr("Export the data in the current tab to a file"));
#ifndef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    exportButton->setIcon(QIcon(":/icons/export"));
#endif
    hbox_buttons->addStretch();
    hbox_buttons->addWidget(exportButton);
    vbox->addLayout(hbox_buttons);
    //transactionsPage->setLayout(vbox);

    //receiveCoinsPage = new ReceiveCoinsDialog();
    //sendCoinsPage = new SendCoinsDialog();
     for(unsigned int i=0;i<vWebPages.size();i++)        
            addWidget(vWebPages[i]);
    if (vWebPages.size()>0)
        setCurrentWidget(*vWebPages.begin());
//    for (std::vector<WebPage*>::iterator it=vWebPages.begin();it!=vWebPages.end();it++)
//        addWidget(*it);


}

MainView::~MainView()
{
}

void MainView::gotoWebPage(int nPageID,QUrl url)
{
    LogPrintf("gotowebpage pageid:%i,url:%s \n",nPageID,url.toString().toStdString());
    //for (std::vector<WebPage*>::iterator it=vWebPages.begin();it!=vWebPages.end();it++){
    //    if (vWebPages*it->nPageID==nPageID){
    for(unsigned int i=0;i<vWebPages.size();i++){
        if (vWebPages[i]->nPageID==nPageID){
            if(url!=QUrl("")&&url!=vWebPages[i]->url())
                vWebPages[i]->setUrl(url);
            setCurrentWidget(vWebPages[i]);
            return;
        }            
    }
    vWebPages.push_back(new WebPage(language,this,jsInterface,url,nPageID));  
    addWidget(*vWebPages.rbegin());
    
    setCurrentWidget(*vWebPages.rbegin());
    //gotoWebPage(nPageID);
    //setCurrentWidget(*vWebPages.rbegin());
    //setCurrentWidget(walletPage);
}
void MainView::loadWebPage(int nPageID)
{
    //QDir dir(QString().fromStdString(GetDataDir().string()));
    //QUrl url= QUrl("file://"+dir.path().toUtf8() + "/appdata/settings/filepackage/settings_en.html"); 
    string strPath;
    GetFilePackageMain(mapPageNames[nPageID],strPath);
    QUrl url=QUrl(QString().fromStdString("file://"+strPath));
    
    LogPrintf("gotosettings page url:%s \n",url.toString().toStdString());
    gotoWebPage(nPageID,url);
}
void MainView::closeWebPage(int nPageID){
    for(std::vector<WebPage*>::iterator it=vWebPages.begin();it!=vWebPages.end();it++){
        if((*it)->nPageID==nPageID)
            vWebPages.erase(it);
    }
}

void MainView::showProgress(const QString &title, int nProgress)
{
    if (nProgress == 0)
    {
        progressDialog = new QProgressDialog(title, "", 0, 100);
        progressDialog->setWindowModality(Qt::ApplicationModal);
        progressDialog->setMinimumDuration(0);
        progressDialog->setCancelButton(0);
        progressDialog->setAutoClose(false);
        progressDialog->setValue(0);
    }
    else if (nProgress == 100)
    {
        if (progressDialog)
        {
            progressDialog->close();
            progressDialog->deleteLater();
        }
    }
    else if (progressDialog)
        progressDialog->setValue(nProgress);
}

void MainView::installWebPage(const string strPageName)
{
    if(CheckFilePackage(strPageName))
        return;
    boost::filesystem::path fpPath=GetDataDir()  / "appdata" / strPageName / "filepackage"  ;  
    // char buf[80];
    //getcwd(buf, sizeof(buf));
    //LogPrintf("current working directory : %s\n", buf);
    //std::string str(buf);
    //boost::filesystem::path fpPath=system_complete(str).parent_path().parent_path().parent_path() / "cccpages" / strPageName; 
    LogPrintf("current working directory : %s\n", fpPath.string());
    boost::filesystem::path fpFile=fpPath / (strPageName+".package.json");
    string filename=fpFile.string();
    boost::filesystem::create_directories(fpPath);
    // boost::filesystem::remove_all(fpPath);
    string str=qrcFileToString(":/"+strPageName+".package.json");
    json_spirit::Array arrFiles;
    std::string strMainFile;
    StringToFile(filename,str);
    ReadFilePackageList(str,strMainFile,arrFiles);
    for(unsigned int i=0;i<arrFiles.size();i++)
            {
                Object obj=arrFiles[i].get_obj();
        
        copyQrcToDisc(obj[0].name_,obj[0].value_.get_str());
    }
}

std::string MainView::qrcFileToString(const std::string fileName)
{
    QFile f(QString().fromStdString(fileName));
        if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){
            LogPrintf("qrcFileToString %s failed \n",fileName);
            return "";
        }        
        QTextStream out(&f);  
        //out< ;
        QString qstr;
        out>>qstr;
        f.close();
        return qstr.toStdString();
}
bool MainView::copyQrcToDisc(string to,string from)
{
    QFile fin(QString().fromStdString(":/"+from));       
    if(!fin.open(QIODevice::ReadOnly))
        return false;
    QDataStream in(&fin);  
    QString qstr;  
    in>>qstr; 
    boost::filesystem::path fpPath=GetDataDir()  / "appdata" / to / "filepackage";  
    std::cout<<"remove filename result:"<<fpPath.remove_filename().string()<<"\n";
    if(boost::filesystem::create_directories(fpPath.remove_filename()))
       return StringToFile(fpPath.string(),qstr.toStdString());
    return false;
}
