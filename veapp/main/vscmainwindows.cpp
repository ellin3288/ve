

#include "vscmainwindows.h"

#include <QAction>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <qdebug.h>
#include <QDockWidget>
#include <QToolBar>
#include <QTabWidget>
#include <QListWidget>
#include <QMessageBox>

#include "vscdevicelist.h"
#include "vscview.h"
#include "vscplayback.h"
#include "vscvwidget.h"
#include "vscvideowall.h"
#include "vsccameraadd.h"
#include "vscvipcadd.h"
#include "factory.hpp"
#include "vscsearch.h"
#include "vsctoolbar.h"
#include "vscrecorderadd.h"
#include "vscsiteadd.h"
#include "vscsetting.h"
#include "vschddedit.h"
#include "vscloading.hpp"
#include "vscipcgroupconf.h"
#include "vscipcgroupmap.h"
#include "vscuserstatus.h"

extern Factory *gFactory;

Q_DECLARE_METATYPE(QDockWidget::DockWidgetFeatures)

VSCMainWindows::VSCMainWindows(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    QWidget *widget = new QWidget;
    setCentralWidget(widget);
    m_pToolBar = new VSCToolBar(this);
    //m_pEvent->hide();
    CreateActions();
    SetupMenuBar();
    SetupToolBar();
    CreateDockWindows();

    m_pMainArea = new QTabWidget(this);

    m_pMainArea->setTabsClosable(true);
    m_pMainArea->setMovable(true);
    m_pEventThread = VEventThread::CreateObject();
    m_pEventThread->start();
	
	VSCLoading *loading = new VSCLoading(NULL);
	loading->show();
	//loading->setGeometry(width()/2, height()/2, 64, 64);
	QCoreApplication::processEvents();
	
    m_pView = new VSCView(m_pMainArea, *m_pMainArea, "Main View");
    m_pView->setWindowFlags(Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint  );
    m_pMainArea->addTab(m_pView,QIcon(tr(":/view/resources/3x3.png")), tr("Main View"));
    connect(m_pEventThread, SIGNAL(EventNotify(int, int)), 
			m_pView, SLOT(DeviceEvent(int, int)));


    setCentralWidget(m_pMainArea);

    QString message = tr("VS Cloud Client");

	delete loading;
    SetupConnections();
}

VSCMainWindows::~VSCMainWindows()
{

}

void VSCMainWindows::SetupConnections()
{
    connect(m_pMainArea, SIGNAL(tabCloseRequested(int)), this, SLOT(MainCloseTab(int)));
    connect(m_pDeviceList, SIGNAL(SurveillanceClicked()), this, SLOT(AddSurveillance()));
    connect(m_pDeviceList, SIGNAL(CameraAddClicked()), this, SLOT(AddCamera()));
    connect(m_pDeviceList, SIGNAL(PlaybackClicked()), this, SLOT(AddPlayback()));
    connect(m_pDeviceList, SIGNAL(SearchClicked()), this, SLOT(Search()));
    connect(m_pDeviceList, SIGNAL(RecorderClicked()), this, SLOT(AddRecorder()));

    
    connect(m_pDeviceList, SIGNAL(CameraEditClicked(int)), this, SLOT(EditCamera(int)));
    connect(m_pDeviceList, SIGNAL(CameraDeleteClicked(int)), this, SLOT(DeleteCamera(int)));

    /* VIPC */
    connect(m_pDeviceList, SIGNAL(VIPCAddClicked()), this, SLOT(AddVIPC()));
    connect(m_pDeviceList, SIGNAL(VIPCEditClicked(int)), this, SLOT(EditVIPC(int)));
    connect(m_pDeviceList, SIGNAL(VIPCDeleteClicked(int)), this, SLOT(DeleteVIPC(int)));

   /* Camera Group */
    connect(m_pDeviceList, SIGNAL(VGroupAddClicked()), this, SLOT(AddVGroup()));
    connect(m_pDeviceList, SIGNAL(VGroupEditClicked(int)), this, SLOT(EditVGroup(int)));
    connect(m_pDeviceList, SIGNAL(VGroupDeleteClicked(int)), this, SLOT(DeleteVGroup(int)));
    connect(m_pDeviceList, SIGNAL(VGroupMapClicked()), this, SLOT(MapVGroup()));


    /* Disk edit */
    connect(m_pDeviceList, SIGNAL(DiskEditClicked()), this, SLOT(EditDisk()));

    /* View */
    connect(m_pDeviceList, SIGNAL(ViewDeleteClicked(int)), this, SLOT(DeleteView(int)));
	

    //connect(this, SIGNAL(CameraDeleted()), m_pDeviceList, SLOT(CameraTreeUpdated()));
    connect(m_pToolBar->ui.pbFullScreen, SIGNAL(clicked()), this, SLOT(SetFullScreen()));
    connect(m_pToolBar->ui.pbAbout, SIGNAL(clicked()), this, SLOT(about()));
    connect(m_pToolBar->ui.pbAlarm, SIGNAL(clicked()), this, SLOT(AddEvent()));
    connect(m_pToolBar->ui.pbSetting, SIGNAL(clicked()), this, SLOT(Setting()));
    connect(m_pToolBar->ui.pbUser, SIGNAL(clicked()), this, SLOT(UserStatus()));
    connect(m_pEventThread, SIGNAL(EventNotifyNoParam()), m_pToolBar, SLOT(NewAlarm()));

}

void VSCMainWindows::AddEvent()
{
    VEvent *pEvent = VEvent::CreateObject(m_pMainArea);
	
    m_pMainArea->addTab(pEvent, QIcon(tr(":/action/resources/alarmno.png")), tr("Alarm"));
    m_pMainArea->setCurrentWidget(pEvent);
}

void VSCMainWindows::AddSurveillance()
{
	char Name[256];
	int tabNum = m_pMainArea->count();
	int currentNum = -1;
	for (int i=0; i<tabNum; i++)
	{
		QWidget *qwidget = m_pMainArea->widget(i);
		VSCView *pView = dynamic_cast<VSCView* >(qwidget);
		if (pView)
		{
			currentNum++;
		}
	}
	sprintf(Name, "%s %d", "View", currentNum+1);
	VSCLoading *loading = new VSCLoading(NULL);
	//loading->setGeometry(width()/2, height()/2, 64, 64);
	QCoreApplication::processEvents();
	VSCView *pView = new VSCView(m_pMainArea,  *m_pMainArea, Name);
	pView->setWindowFlags(Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint  );
    	m_pMainArea->addTab(pView, QIcon(tr(":/view/resources/3x3.png")), tr("View %1").arg(currentNum+1));

	m_pMainArea->setCurrentWidget(pView);
	delete loading;
	connect(m_pEventThread, SIGNAL(EventNotify(int, VscEventType)), pView, SLOT(DeviceEvent(int, VscEventType)));
}

void VSCMainWindows::AddPlayback()
{
#if 0
    VSCSearchPb *pPb = new VSCSearchPb(m_pMainArea);
	
    m_pMainArea->addTab(pPb, QIcon(tr(":/action/resources/videosearch.png")), tr("Playback"));
    m_pMainArea->setCurrentWidget(pPb);
#endif
}

void VSCMainWindows::Setting()
{
    VSCSetting *pSetting = new VSCSetting(this, *m_pDeviceList);

    m_pMainArea->addTab(pSetting, QIcon(tr(":/action/resources/setting.png")), tr("Setting"));  
    m_pMainArea->setCurrentWidget(pSetting);
}

void VSCMainWindows::AddCamera()
{
    DeviceParam mParam;
    VSCCameraAdd *pCameraadd = new VSCCameraAdd(mParam, this);

    m_pMainArea->addTab(pCameraadd, QIcon(tr(":/device/resources/camera.png")), tr("Camera"));  
    m_pMainArea->setCurrentWidget(pCameraadd);
    //connect(pCameraadd, SIGNAL(CameraTreeUpdated()), m_pDeviceList, SLOT(CameraTreeUpdated()));
}

void VSCMainWindows::AddRecorder()
{
    DeviceParam mParam;
    VSCRecorderAdd *pRecorderadd = new VSCRecorderAdd(mParam, this);

    m_pMainArea->addTab(pRecorderadd, QIcon(tr(":/action/resources/computer.png")), tr("Recorder"));  
    m_pMainArea->setCurrentWidget(pRecorderadd);
    //connect(pCameraadd, SIGNAL(CameraTreeUpdated()), m_pDeviceList, SLOT(CameraTreeUpdated()));
}

void VSCMainWindows::AddSite()
{
    VSCVmsDataItem mParam;
    memset(&mParam, 0, sizeof(mParam));
    VSCVmsDataItemDefault(mParam);
    VSCSiteAdd *pSiteadd = new VSCSiteAdd(mParam, this);

    m_pMainArea->addTab(pSiteadd, QIcon(tr(":/action/resources/site.png")), tr("Site"));  
    m_pMainArea->setCurrentWidget(pSiteadd);
}

void VSCMainWindows::Search()
{
    if (VSCSearch::m_bStarted == TRUE)
    {
        QMessageBox msgBox(this);
        //Set text
        msgBox.setText(tr("Search is In Processing ..."));
            //Set predefined icon, icon is show on left side of text.
        msgBox.setIconPixmap(QPixmap(":/logo/resources/vsc32.png"));
        msgBox.setStandardButtons(QMessageBox::Ok);
            //Set focus of ok button
        msgBox.setDefaultButton(QMessageBox::Ok);

        int ret = msgBox.exec();

        return;
    }
    VSCSearch *pSearch = new VSCSearch(this);

    m_pMainArea->addTab(pSearch, QIcon(tr(":/action/resources/search.png")), tr("Search"));
    m_pMainArea->setCurrentWidget(pSearch);
	
    //connect(pSearch, SIGNAL(CameraTreeUpdated()), m_pDeviceList, SLOT(CameraTreeUpdated()));
}


void VSCMainWindows::EditDisk()
{
    VDC_DEBUG( "%s\n",__FUNCTION__);
    VSCHddEdit *pDiskEdit = new VSCHddEdit(this);

    m_pMainArea->addTab(pDiskEdit, QIcon(tr(":/device/resources/harddisk.png")), tr("Disk"));
    m_pMainArea->setCurrentWidget(pDiskEdit);
    //connect(pDiskEdit, SIGNAL(DiskTreeUpdated()), m_pDeviceList, SLOT(DiskTreeUpdated()));
}

void VSCMainWindows::EditCamera(int nId)
{
    VDC_DEBUG( "%s %d\n",__FUNCTION__, nId);
    DeviceParam mParam;
    gFactory->GetDeviceParamById(mParam, nId);
    VSCCameraAdd *pCameraadd = new VSCCameraAdd(mParam, this);

    m_pMainArea->addTab(pCameraadd, QIcon(tr(":/device/resources/camera.png")), tr("Camera"));
    m_pMainArea->setCurrentWidget(pCameraadd);
    //connect(pCameraadd, SIGNAL(CameraTreeUpdated()), m_pDeviceList, SLOT(CameraTreeUpdated()));
}
void VSCMainWindows::DeleteCamera(int nId)
{
    VDC_DEBUG( "%s %d\n",__FUNCTION__, nId);
    QMessageBox msgBox(this);
    //Set text
    msgBox.setText(tr("Delete the Camera ..."));
        //Set predefined icon, icon is show on left side of text.
    msgBox.setIconPixmap(QPixmap(":/logo/resources/vsc32.png"));

    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        //Set focus of ok button
    msgBox.setDefaultButton(QMessageBox::Ok);

        //execute message box. method exec() return the button value of cliecke button
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Ok:
       gFactory->DelDevice(nId);
       emit CameraDeleted();
       break;
    default:
       // should never be reached
       break;
    }

    return;
}

void VSCMainWindows::AddVIPC()
{
    VIPCDeviceParam mParam;
    VSCVIPCAdd *pVIPCadd = new VSCVIPCAdd(mParam, this);

    m_pMainArea->addTab(pVIPCadd, QIcon(tr(":/device/resources/virtualipc.png")), tr("Virutal IPC"));  
    m_pMainArea->setCurrentWidget(pVIPCadd);
}

void VSCMainWindows::EditVIPC(int nId)
{
    VDC_DEBUG( "%s %d\n",__FUNCTION__, nId);
    VIPCDeviceParam mParam;
    gFactory->GetVIPCParamById(mParam, nId);
    VSCVIPCAdd *pVIPCadd = new VSCVIPCAdd(mParam, this);

    m_pMainArea->addTab(pVIPCadd, QIcon(tr(":/device/resources/virtualipc.png")), tr("Virutal IPC"));  
    m_pMainArea->setCurrentWidget(pVIPCadd);
}
void VSCMainWindows::DeleteVIPC(int nId)
{
    VDC_DEBUG( "%s %d\n",__FUNCTION__, nId);
    QMessageBox msgBox(this);
    //Set text
    msgBox.setText(tr("Delete the Virtual IPC ..."));
        //Set predefined icon, icon is show on left side of text.
    msgBox.setIconPixmap(QPixmap(":/logo/resources/vsc32.png"));

    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        //Set focus of ok button
    msgBox.setDefaultButton(QMessageBox::Ok);

        //execute message box. method exec() return the button value of cliecke button
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Ok:
       gFactory->DelVIPC(nId);
       break;
    default:
       // should never be reached
       break;
    }

    return;
}

void VSCMainWindows::AddVGroup()
{
	VSCIPCGroupConf group;
	VSCVGroupDataItem groupItem;
	group.SetName("Group");
	group.show();
	group.exec();

	VSCIPCGroupConfType type = group.GetType();
	if (type == VSC_IPCGROUP_CONF_LAST)
	{
		return;
	}
	string strName = "Group";
	group.GetName(strName);
	strcpy(groupItem.Name, strName.c_str());
    	gFactory->AddVGroup(groupItem);
}

void VSCMainWindows::EditVGroup(int nId)
{
    VSCIPCGroupConf group;
	VSCVGroupDataItem groupItem;
	gFactory->GetVGroupById(groupItem, nId);
	group.SetName(groupItem.Name);
       group.show();
       group.exec();

	VSCIPCGroupConfType type = group.GetType();
	if (type == VSC_IPCGROUP_CONF_LAST)
	{
		return;
	}
	string strName = "Group";
	group.GetName(strName);
	strcpy(groupItem.Name, strName.c_str());
    	gFactory->AddVGroup(groupItem);
}
void VSCMainWindows::DeleteVGroup(int nId)
{
    VDC_DEBUG( "%s %d\n",__FUNCTION__, nId);
    QMessageBox msgBox(this);
    //Set text
    msgBox.setText(tr("Delete the  Group ..."));
        //Set predefined icon, icon is show on left side of text.
    msgBox.setIconPixmap(QPixmap(":/logo/resources/vsc32.png"));

    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        //Set focus of ok button
    msgBox.setDefaultButton(QMessageBox::Ok);

        //execute message box. method exec() return the button value of cliecke button
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Ok:
       gFactory->DelVGroup(nId);
       break;
    default:
       // should never be reached
       break;
    }

    return;
}


void VSCMainWindows::MapVGroup()
{
   VDC_DEBUG( "%s\n",__FUNCTION__);
    VSCVGroupWall *pGroup = new VSCVGroupWall(this);

    m_pMainArea->addTab(pGroup, QIcon(tr(":/device/resources/camgroup.png")), tr("Camera Group"));
    m_pMainArea->setCurrentWidget(pGroup);
}

void VSCMainWindows::DeleteView(int nId)
{
    VDC_DEBUG( "%s %d\n",__FUNCTION__, nId);
    QMessageBox msgBox(this);
    //Set text
    msgBox.setText(tr("Delete the View ..."));
        //Set predefined icon, icon is show on left side of text.
    msgBox.setIconPixmap(QPixmap(":/logo/resources/vsc32.png"));

    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        //Set focus of ok button
    msgBox.setDefaultButton(QMessageBox::Ok);

        //execute message box. method exec() return the button value of cliecke button
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Ok:
       gFactory->DelView(nId);
       break;
    default:
       // should never be reached
       break;
    }

    return;
}

void VSCMainWindows::CreateActions()
{
    pActSurveillance = new QAction(tr("&Surveillance"), this);
    pActSurveillance->setIcon(QIcon(":/action/resources/surveillance.png"));

    pActSearch = new QAction(tr("&Search"), this);
    pActSearch->setIcon(QIcon(":/action/resources/search.png"));


    pActDeviceList = new QAction(tr("&Devices List"), this);
    pActDeviceList->setIcon(QIcon(":/action/resources/devicelist.png"));

    pActDeviceAdd = new QAction(tr("&Add"), this);
    pActDeviceAdd->setIcon(QIcon(":/action/resources/list-add.png"));

    pActDeviceDel = new QAction(tr("&Delete"), this);
    pActDeviceDel->setIcon(QIcon(":/action/resources/list-remove.png"));

    pActDeviceConf = new QAction(tr("&Configurate"), this);
    pActDeviceConf->setIcon(QIcon(":/action/resources/configure.png"));

}

void VSCMainWindows::SetupToolBar()
{
    QToolBar *pToolBar = new QToolBar();
    pToolBar->addWidget(m_pToolBar);
    addToolBar(Qt::TopToolBarArea, pToolBar);

}
void VSCMainWindows::SetupMenuBar()
{

}
void VSCMainWindows::about()
{
   QMessageBox::about(this, tr("About VdcEye manager"),
            tr("The <b>VdcEye manager</b>. <br>"
            "<a href=\"https://github.com/xsmart\">https://github.com/xsmart</a>"
            "  <br><a href=\"http://www.vdceye.com/\">http://www.vdceye.com/</a>"));
}

void VSCMainWindows::UserStatus()
{
	VSCUserStatus userStatus;

	userStatus.show();
	userStatus.exec();
}

void VSCMainWindows::SetFullScreen()
{
    if(isFullScreen()) {
        this->setWindowState(Qt::WindowMaximized);
    } else {
        this->setWindowState(Qt::WindowFullScreen);
    }
}

void VSCMainWindows::CreateDockWindows()
{
    QDockWidget *pDockDevicelist = new QDockWidget(tr("Devices"), this);

    pDockDevicelist->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    m_pDeviceList = new VSCDeviceList(pDockDevicelist);
    pDockDevicelist->setWidget(m_pDeviceList);

    addDockWidget(Qt::LeftDockWidgetArea, pDockDevicelist);
}

void VSCMainWindows::MainCloseTab(int index)
{
    QWidget *wdgt = m_pMainArea->widget(index );
    m_pMainArea->setCurrentWidget(wdgt);
    /* Frist view can not be closed */
    if (wdgt == m_pView)
    {
        return;
    }
    m_pMainArea->removeTab(index);
    if (wdgt)
    {
        delete wdgt;
        wdgt = NULL;
    }
}
