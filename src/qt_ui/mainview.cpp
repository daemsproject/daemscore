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
    //boost::filesystem::path fpPath=GetDataDir()  / "appdata" / strPageName;  
     char buf[80];
    getcwd(buf, sizeof(buf));
    LogPrintf("current working directory : %s\n", buf);
    std::string str(buf);
    boost::filesystem::path fpPath=system_complete(str) / strPageName;  
    boost::filesystem::path fpFile=fpPath / (strPageName+".ffl");
    boost::filesystem::create_directories(fpPath);
    string filename=fpFile.string();
    json_spirit::Array arrFiles;
    std::string strMainFile;
    if(boost::filesystem::exists(fpFile))
    {
        bool fIntegrite=true;
        std::string str;        
        if(FileToString(fpFile.string(),str)&&readFileList(str,strMainFile,arrFiles))
        {
            for(unsigned int i=0;i<arrFiles.size();i++)
            {
                Object obj=arrFiles[i].get_obj();
                
                boost::filesystem::path fpFile2=fpPath / obj[0].name_;
                if(!boost::filesystem::exists(fpFile2)) 
                {
                    fIntegrite=false;
                    break;
                }
            }
            if (fIntegrite)
                return;
        }
         boost::filesystem::remove_all(fpPath);
    }          
    str=qrcFileToString(":/"+strPageName+".ffl");
    StringToFile(filename,str);
    readFileList(str,strMainFile,arrFiles);
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
    boost::filesystem::path fpPath=GetDataDir()  / "appdata" / to;  
    std::cout<<"remove filename result:"<<fpPath.remove_filename().string()<<"\n";
    if(boost::filesystem::create_directories(fpPath.remove_filename()))
       return StringToFile(fpPath.string(),qstr.toStdString());
    return false;
}
bool MainView::readFileList(const std::string strFileList,std::string& strMainFile,json_spirit::Array& arrFiles)
{
        json_spirit::Value fileData;
        if (!json_spirit::read_string(strFileList,fileData)){
            ///LogPrintf("readFileList %s: fail2 \n",strPageName);
            return false;
        }    
        if(fileData.type() != json_spirit::obj_type)
        {
          // LogPrintf("readFileList %s:  fail3 \n",strPageName);
            return false;
        }
        json_spirit::Object obj= fileData.get_obj();
        json_spirit::Value val = find_value(obj, "files");
        if (val.type()!=obj_type)
        {
           // LogPrintf("readFileList %s:  fail4 \n",strPageName);
            return false;
        }
        arrFiles = val.get_array();
        val = find_value(obj, "mainfile");
        if (val.type()!=str_type)
        {
            //LogPrintf("readFileList %s:  fail5 \n",strPageName);
            return false;
        }
        strMainFile = val.get_str();
        
}