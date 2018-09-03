/*
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <TString.h>
#include <TFile.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <TH1I.h>
#include <TH1F.h>
#include <TH2I.h>
#include <TF1.h>
#include <TLegend.h>

#include <QActionGroup>
#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
#include <QProgressDialog>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QMessageBox>
#include <QTextStream>
#include <QThreadPool>
#include <QDebug>

#include <cstdio>

#include <typeinfo>

#if defined(Q_OS_WIN32) && defined(__MINGW32__)
#include <windef.h>
#include <winbase.h>
#endif

#include <ftd2xx.h>

#include "acquisitionthread.h"
#include "commandthread.h"
#include "writeprocess.h"
#include "rootcanvasdialog.h"
#include "settingsdialog.h"
#include "opcuaclientdialog.h"

#include "backgroundvaluedelegate.h"
#include "signalvaluedelegate.h"

#include "diagramtreewidgetaction.h"
#include "diagramtreewidgetitem.h"
#include "rundetailslistwidgetitem.h"
#include "diagramparameters.h"
#include "channelscountsfit.h"
#include "channelschargefit.h"

#include "opcuaclient.h"

#include "ui_mainwindow.h"
#include "mainwindow.h"

namespace {

struct Hist1Parameters hist1params[] = {
    { HIST_CHANNEL1, "C1", "Channel 1", 400, 0., 4095. },
    { HIST_CHANNEL2, "C2", "Channel 2", 400, 0., 4095. },
    { HIST_CHANNEL3, "C3", "Channel 3", 400, 0., 4095. },
    { HIST_CHANNEL4, "C4", "Channel 4", 400, 0., 4095. },
    { HIST_FITALL, "F", "Fitted counts", 200, 0., 400. },
    { HIST_FIT_CHANNEL1, "FC1", "Fitted channel 1", 200, 0., 400. },
    { HIST_FIT_CHANNEL2, "FC2", "Fitted channel 2", 200, 0., 400. },
    { HIST_FIT_CHANNEL3, "FC3", "Fitted channel 3", 200, 0., 400. },
    { HIST_FIT_CHANNEL4, "FC4", "Fitted channel 4", 200, 0., 400. },
    { HIST_FIT_MEAN, "FMEAN", "Fitted mean", 200, 0., 400. },
    { HIST_FIT_MEDIAN, "FMED", "Fitted median", 200, 0., 400. },
    { HIST_RANK1, "FR1", "Fitted rank 1", 200, 0., 400. },
    { HIST_RANK2, "FR2", "Fitted rank 2", 200, 0., 400. },
    { HIST_RANK3, "FR3", "Fitted rank 3", 200, 0., 400. },
    { HIST_RANK4, "FR4", "Fitted rank 4", 200, 0., 400. },
    { HIST_Z, "Z", "Charge destribution", 200, 0.5, 10.5 },
    { HIST_Z2, "Z2", "Charge^{2} destribution", 200, 0.5, 100.5 },
    { NONE, nullptr, nullptr, 0, 0.0, 0.0 }
};

struct Hist1Parameters& c1hp = hist1params[0];
struct Hist1Parameters& c2hp = hist1params[1];
struct Hist1Parameters& c3hp = hist1params[2];
struct Hist1Parameters& c4hp = hist1params[3];
struct Hist1Parameters& fhp = hist1params[4];
struct Hist1Parameters& zhp = hist1params[15];
struct Hist1Parameters& z2hp = hist1params[16];

struct Hist2Parameters hist2params[] = {
    { HIST_CHANNEL12, "C12", "Channel correlation 1-2", 200, 0., 400., 200, 0., 400. },
    { HIST_CHANNEL23, "C23", "Channel correlation 2-3", 200, 0., 400., 200, 0., 400. },
    { HIST_CHANNEL34, "C34", "Channel correlation 3-4", 200, 0., 400., 200, 0., 400. },
    { HIST_CHANNEL14, "C14", "Channel correlation 1-4", 200, 0., 400., 200, 0., 400. },
    { HIST_CHANNEL13, "C13", "Channel correlation 1-3", 200, 0., 400., 200, 0., 400. },
    { HIST_CHANNEL24, "C24", "Channel correlation 2-4", 200, 0., 400., 200, 0., 400. },
    { HIST_Z12, "Z12", "Charge correlation 1-2", 200, 0.5, 10.5, 200, 0.5, 10.5 },
    { HIST_Z23, "Z23", "Charge correlation 2-3", 200, 0.5, 10.5, 200, 0.5, 10.5 },
    { HIST_Z34, "Z34", "Charge correlation 3-4", 200, 0.5, 10.5, 200, 0.5, 10.5 },
    { HIST_Z14, "Z14", "Charge correlation 1-4", 200, 0.5, 10.5, 200, 0.5, 10.5 },
    { HIST_Z13, "Z13", "Charge correlation 1-3", 200, 0.5, 10.5, 200, 0.5, 10.5 },
    { HIST_Z24, "Z24", "Charge correlation 2-4", 200, 0.5, 10.5, 200, 0.5, 10.5 },
    { NONE, nullptr, nullptr, 0, 0.0, 0.0, 0, 0.0, 0.0 }
};
/*
struct Hist2Parameters& c12hp = hist2params[0];
struct Hist2Parameters& c23hp = hist2params[1];
struct Hist2Parameters& c34hp = hist2params[2];
struct Hist2Parameters& c14hp = hist2params[3];
struct Hist2Parameters& c13hp = hist2params[4];
struct Hist2Parameters& c24hp = hist2params[5];
struct Hist2Parameters& z12hp = hist2params[6];
struct Hist2Parameters& z23hp = hist2params[7];
struct Hist2Parameters& z34hp = hist2params[8];
struct Hist2Parameters& z14hp = hist2params[9];
struct Hist2Parameters& z13hp = hist2params[10];
struct Hist2Parameters& z24hp = hist2params[11];
*/
void
local_reverse(char* s)
{
    for ( size_t i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/// transform value "n" to string "s"
void
local_itoa( int n, char* s, size_t digits = 3)
{
    size_t i = 0;
    for ( ; i < digits; ) { // generate digits in reverse order
        s[i++] = n % 10 + '0'; // get next digit
        n /= 10; // delete it
    };

    s[i] = '\0';
    local_reverse(s);
}

// charge colors
const Color_t ccolors[] = { kBlack, kRed, kBlue, kCyan, kOrange, kMagenta + 10, kViolet };

// number of Gaus parameters
const int gparams = 3;

const char* description_channel_a = "FT2232H_MM A";
const char* description_channel_b = "FT2232H_MM B";

const size_t towrite = COMMAND_SIZE;

} // namespace

MainWindow::MainWindow(QWidget *parent)
    :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(new QTimer(this)),
    timer_data(new QTimer(this)),
    timer_opcua(new QTimer(this)),
    timer_heartbeat(new QTimer(this)),
    channel_a(nullptr),
    channel_b(nullptr),
    filerun(nullptr),
    filetxt(nullptr),
    filedat(nullptr),
    command_thread(new CommandThread(this)),
    acquire_thread(new AcquireThread(this)),
    process_thread(new ProcessThread(this)),
    profile_thread(new ProcessFileThread(this)),
    progress_dialog(new QProgressDialog( tr("Processing file..."), \
        tr("Abort Process"), 0, 0, this)),
    settings(new QSettings( "BeamComposition", "configure")),
    flag_background(false),
    flag_write_run(true),
    sys_status(STATUS_DEVICE_DISCONNECTED),
    opcua_client(new OpcUaClient(this)),
    opcua_dialog(nullptr)
{
    ui->setupUi(this);

    loadSettings(settings);

    createTreeWidgetItems();
    createRootHistograms();

    // Root diagrams
    DiagramTreeWidgetAction action(ui->treeWidget);
    action.getDiagrams(diagrams);
    profile_thread->setDiagrams(diagrams);

    // Background Values
    BackgroundValueDelegate* delegate = new BackgroundValueDelegate();
    ui->runInfoTableWidget->setItemDelegate(delegate);

    // Fitting initiation
    params = FitParameters::instance(settings);

    // background
    const SignalArray& back = params->background();
    for ( int i = 0; i < CHANNELS; ++i) {
        const SignalPair& p = back[i];
        QString str = SignalValueDelegate::form_text(p);
        QTableWidgetItem* item = new QTableWidgetItem(str);
        ui->runInfoTableWidget->setItem( i + 1, 0, item);
    }

    // charge information
    for ( int i = 0; i < CARBON_Z; ++i) {
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->runInfoTableWidget->setItem( i + 6, 0, item);
    }

    // triggers information
    for ( int i = 0; i < 2; ++i) {
        QTableWidgetItem* item = new QTableWidgetItem();
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->runInfoTableWidget->setItem( i + 13, 0, item);
    }

    QActionGroup* selectionGroup = new QActionGroup(this);
    selectionGroup->addAction(ui->actionDetailsSelectionSingle);
    selectionGroup->addAction(ui->actionDetailsSelectionMultiple);
    selectionGroup->addAction(ui->actionDetailsSelectionContiguous);
    selectionGroup->addAction(ui->actionDetailsSelectionExtended);
    ui->actionDetailsSelectionMultiple->setChecked(true);

    connect( ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect( ui->actionOpenRun, SIGNAL(triggered()), this, SLOT(openRun()));
    connect( ui->actionOpenBackRun, SIGNAL(triggered()), this, SLOT(openBackRun()));
    connect( ui->actionSaveRun, SIGNAL(triggered()), this, SLOT(saveRun()));
    connect( ui->actionSettings, SIGNAL(triggered()), this, SLOT(setRunSettings()));
    connect( ui->actionOpcUaClient, SIGNAL(triggered(bool)), this, SLOT(opcUaClientDialog(bool)));
    connect( ui->actionDetailsClearAll, SIGNAL(triggered()), this, SLOT(detailsClear()));
    connect( ui->actionDetailsSelectAll, SIGNAL(triggered()), this, SLOT(detailsSelectAll()));
    connect( ui->actionDetailsSelectNone, SIGNAL(triggered()), this, SLOT(detailsSelectNone()));
    connect( selectionGroup, SIGNAL(triggered(QAction*)), this, SLOT(runDetailsSelectionTriggered(QAction*)));

    connect( ui->startRunButton, SIGNAL(clicked()), this, SLOT(startRun()));
    connect( ui->stopRunButton, SIGNAL(clicked()), this, SLOT(stopRun()));
    connect( ui->connectButton, SIGNAL(clicked()), this, SLOT(connectDevices()));
    connect( ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectDevices()));
    connect( ui->resetAlteraPushButton, SIGNAL(clicked()), this, SLOT(resetAlteraClicked()));
    connect( ui->delaySpinBox, SIGNAL(valueChanged(int)), this, SLOT(setDelayChanged(int)));
    connect( ui->triggersComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(triggersItemChanged(int)));
    connect( ui->motorComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(motorItemChanged(int)));
    connect( ui->acquisitionTimeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(acquisitionTimingChanged(int)));
    connect( ui->delayTimeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(acquisitionTimingChanged(int)));

    connect( ui->resetDiagramsPushButton, SIGNAL(clicked()), this, SLOT(resetDiagramsClicked()));
    connect( ui->processPushButton, SIGNAL(clicked()), this, SLOT(processBatchesClicked()));

    connect( ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(treeWidgetItemDoubleClicked( QTreeWidgetItem*, int)));
    connect( ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
             this, SLOT(treeWidgetItemClicked( QTreeWidgetItem*, int)));
    connect( ui->runInfoTableWidget, SIGNAL(cellChanged(int,int)),
             this, SLOT(backgroundValueChanged(int, int)));
    connect( ui->runDetailsListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(detailsItemSelectionChanged()));

    connect( ui->runTypeButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(runTypeChanged(int)));
    connect( ui->updateDataButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(dataUpdateChanged(int)));

    connect( timer, SIGNAL(timeout()), this, SLOT(handle_root_events()));
    connect( timer_opcua, SIGNAL(timeout()), opcua_client, SLOT(iterate()));
    connect( timer_heartbeat, SIGNAL(timeout()), this, SLOT(onOpcUaTimeout()));

    connect( command_thread, SIGNAL(started()), this, SLOT(commandThreadStarted()));
    connect( command_thread, SIGNAL(finished()), this, SLOT(commandThreadFinished()));
    connect( command_thread, SIGNAL(signalMovementFinished()), this, SLOT(movementFinished()));
    connect( command_thread, SIGNAL(signalDeviceError()), this, SLOT(commandDeviceError()));

    connect( process_thread, SIGNAL(started()), this, SLOT(processThreadStarted()));
    connect( process_thread, SIGNAL(finished()), this, SLOT(processThreadFinished()));
    connect( acquire_thread, SIGNAL(started()), this, SLOT(acquireThreadStarted()));
    connect( acquire_thread, SIGNAL(finished()), this, SLOT(acquireThreadFinished()));
    connect( acquire_thread, SIGNAL(signalDeviceError()), this, SLOT(acquireDeviceError()));

    connect( profile_thread, SIGNAL(started()), this, SLOT(processFileStarted()));
    connect( profile_thread, SIGNAL(finished()), this, SLOT(processFileFinished()));
    connect( profile_thread, SIGNAL(progress(int)), progress_dialog, SLOT(setValue(int)));
    connect( progress_dialog, SIGNAL(canceled()), profile_thread, SLOT(stop()));

    connect( this, SIGNAL(signalBeamSpectrumChanged(RunInfo::BeamSpectrumArray,RunInfo::BeamSpectrumArray,QDateTime)),
        opcua_client, SLOT(writeBeamSpectrumValue(RunInfo::BeamSpectrumArray,RunInfo::BeamSpectrumArray,QDateTime)));

    connect( opcua_client, SIGNAL(externalCommandChanged( int, QDateTime)),
        this, SLOT(externalSignalReceived( int, QDateTime)));
    connect( opcua_client, SIGNAL(connected()), this, SLOT(onOpcUaClientConnected()));
    connect( opcua_client, SIGNAL(connected()), timer_heartbeat, SLOT(start()));
    connect( opcua_client, SIGNAL(disconnected()), timer_heartbeat, SLOT(stop()));

    ui->runDetailsListWidget->addAction(ui->actionDetailsSelectAll);
    ui->runDetailsListWidget->addAction(ui->actionDetailsSelectNone);

    QAction* act0 = new QAction(this);
    act0->setSeparator(true);
    act0->setText(tr("Selection"));
    ui->runDetailsListWidget->addAction(act0);

    ui->runDetailsListWidget->addAction(ui->actionDetailsSelectionSingle);
    ui->runDetailsListWidget->addAction(ui->actionDetailsSelectionMultiple);
    ui->runDetailsListWidget->addAction(ui->actionDetailsSelectionContiguous);
    ui->runDetailsListWidget->addAction(ui->actionDetailsSelectionExtended);

    QAction* act1 = new QAction(this);
    act1->setSeparator(true);
    ui->runDetailsListWidget->addAction(act1);
    ui->runDetailsListWidget->addAction(ui->actionDetailsClearAll);
    ui->runDetailsListWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    progress_dialog->setWindowModality(Qt::WindowModal);
    progress_dialog->setWindowTitle(tr("Progress"));

    timer->start(20);

    int update_period = settings->value( "update-timeout", 3).toInt() * 1000;
    timer_data->setInterval(update_period);

    update_period = settings->value( "opcua-update-timeout", 2).toInt() * 1000;    
    timer_opcua->setInterval(update_period);
    timer_heartbeat->setInterval(update_period);
    timer_opcua->start();

    bool opcua_at_startup = settings->value( "opcua-connect-startup", false).toBool();
    if (opcua_at_startup) {
        QTimer::singleShot( 0, this, SLOT(opcUaClientDialog()));
    }

    ui->treeWidget->expandAll();
}

MainWindow::~MainWindow()
{
    timer_data->stop();
    delete timer_data;

    timer->stop();
    delete timer;

    command_thread->stop();
    command_thread->wait();
    delete command_thread;

    profile_thread->stop();
    profile_thread->wait();
    delete profile_thread;

    acquire_thread->stop();
    process_thread->stop();

    acquire_thread->wait();
    process_thread->wait();

    delete acquire_thread;
    delete process_thread;

    if (filerun) {
        filerun->flush();
        filerun->close();
        delete filerun;
    }

    if (filetxt) {
        filetxt->flush();
        filetxt->close();
        delete filetxt;
    }

    if (filedat) {
        filedat->flush();
        filedat->close();
        delete filedat;
    }
/*
    delete progress_dialog;
    delete opcua_client;
    if (opcua_dialog)
        delete opcua_dialog;
*/
    delete settings;
    delete ui;
}

void
MainWindow::handle_root_events()
{
    //call the inner loop of ROOT
    gSystem->ProcessEvents();
}

void
MainWindow::treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int)
{
    DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(item);

    if (ditem) {
        DiagramType type = ditem->diagramType();
        if (type != NONE) {
            RootCanvasDialog* dialog = ditem->canvasDialog();
            if (!dialog) {
//                qDebug() << "GUI: TreeWidgetItem double clicked created";
                dialog = createCanvasDialog(ditem);
            }

            if (dialog && dialog->isHidden()) {
//                qDebug() << "GUI: TreeWidgetItem double clicked exists";
//                qDebug() << "GUI: TreeWidgetItem double clicked hidden";
                dialog->show();
                dialog->updateDiagram();
            }
            dialog->raise();
            dialog->activateWindow();
        }
    }
}

void
MainWindow::treeWidgetItemClicked(QTreeWidgetItem* item, int)
{
    DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(item);

    if (ditem) {
        DiagramType type = ditem->diagramType();

        if (type != NONE) {
            RootCanvasDialog* dialog = ditem->canvasDialog();
            if (dialog && !dialog->isHidden())
                dialog->raise();
        }
    }
}

RootCanvasDialog*
MainWindow::createCanvasDialog(DiagramTreeWidgetItem* item)
{
    QTreeWidgetItem* parent = item->parent();
    int parent_row = ui->treeWidget->indexOfTopLevelItem(parent);

    RootCanvasDialog* dialog = nullptr;

    switch (parent_row) {
    case 0: // 1-D diagrams
        dialog = createCanvas<TH1>(item);
        break;
    case 1: // 2-D diagrams
        dialog = createCanvas<TH2>(item);
        break;
    case -1: // root index
    default:
        break;
    }

    return dialog;
}

template<class T>
RootCanvasDialog*
MainWindow::createCanvas(DiagramTreeWidgetItem* item)
{
    DiagramType type = item->diagramType();

    RootCanvasDialog* dialog = new RootCanvasDialog( this, type);
    connect( this, SIGNAL(updateDiagram()), dialog, SLOT(updateDiagram()));
    connect( dialog, SIGNAL(resetDiagram(DiagramType)), this, SLOT(resetDiagram(DiagramType)));
    connect( dialog, SIGNAL(fitDiagram(DiagramType)), this, SLOT(fitChargeDiagram(DiagramType)));

    item->setCanvasDialog(dialog);
    if (typeid(T) == typeid(TH1)) {
        if (type == HIST_CHANNELS) {
            dialog->drawChannels<TH1>(diagrams.channels);
        }
        else {
            TH1* h = item->getTH1();
            dialog->drawDiagram<TH1>(h);
        }
    }
    else if (typeid(T) == typeid(TH2)) {
        TH2* h = item->getTH2();
        dialog->drawDiagram<TH2>(h);
    }

    QString title = item->text(0);
    dialog->setWindowTitle(title);

    return dialog;
}

void
MainWindow::createTreeWidgetItems()
{
    QTreeWidgetItem* item1D = new QTreeWidgetItem();
    item1D->setText( 0, tr("1-D"));

    QTreeWidgetItem* itemCh = new DiagramTreeWidgetItem(HIST_CHANNELS);
    itemCh->setText( 0, tr("Channels"));

    QTreeWidgetItem* itemC1 = new DiagramTreeWidgetItem(HIST_CHANNEL1);
    itemC1->setText( 0, tr("Channel-1"));
    QTreeWidgetItem* itemC2 = new DiagramTreeWidgetItem(HIST_CHANNEL2);
    itemC2->setText( 0, tr("Channel-2"));
    QTreeWidgetItem* itemC3 = new DiagramTreeWidgetItem(HIST_CHANNEL3);
    itemC3->setText( 0, tr("Channel-3"));
    QTreeWidgetItem* itemC4 = new DiagramTreeWidgetItem(HIST_CHANNEL4);
    itemC4->setText( 0, tr("Channel-4"));

    QTreeWidgetItem* itemFitted = new DiagramTreeWidgetItem(HIST_FITALL);
    itemFitted->setText( 0, tr("Fitted channels"));

    QTreeWidgetItem* itemF1 = new DiagramTreeWidgetItem(HIST_FIT_CHANNEL1);
    itemF1->setText( 0, tr("Fitted channel-1"));

    QTreeWidgetItem* itemF2 = new DiagramTreeWidgetItem(HIST_FIT_CHANNEL2);
    itemF2->setText( 0, tr("Fitted channel-2"));

    QTreeWidgetItem* itemF3 = new DiagramTreeWidgetItem(HIST_FIT_CHANNEL3);
    itemF3->setText( 0, tr("Fitted channel-3"));

    QTreeWidgetItem* itemF4 = new DiagramTreeWidgetItem(HIST_FIT_CHANNEL4);
    itemF4->setText( 0, tr("Fitted channel-4"));

    QTreeWidgetItem* itemFmean = new DiagramTreeWidgetItem(HIST_FIT_MEAN);
    itemFmean->setText( 0, tr("Fitted mean"));

    QTreeWidgetItem* itemFmed = new DiagramTreeWidgetItem(HIST_FIT_MEDIAN);
    itemFmed->setText( 0, tr("Fitted median"));

    QTreeWidgetItem* itemR1 = new DiagramTreeWidgetItem(HIST_RANK1);
    itemR1->setText( 0, tr("Rank-1"));
    QTreeWidgetItem* itemR2 = new DiagramTreeWidgetItem(HIST_RANK2);
    itemR2->setText( 0, tr("Rank-2"));
    QTreeWidgetItem* itemR3 = new DiagramTreeWidgetItem(HIST_RANK3);
    itemR3->setText( 0, tr("Rank-3"));
    QTreeWidgetItem* itemR4 = new DiagramTreeWidgetItem(HIST_RANK4);
    itemR4->setText( 0, tr("Rank-4"));

    QTreeWidgetItem* itemZ = new DiagramTreeWidgetItem(HIST_Z);
    itemZ->setText( 0, tr("Z"));

    QTreeWidgetItem* itemZ2 = new DiagramTreeWidgetItem(HIST_Z2);
    itemZ2->setText( 0, tr("Z^2"));

    item1D->addChild(itemCh);
    item1D->addChild(itemC1);
    item1D->addChild(itemC2);
    item1D->addChild(itemC3);
    item1D->addChild(itemC4);
    item1D->addChild(itemFitted);
    item1D->addChild(itemF1);
    item1D->addChild(itemF2);
    item1D->addChild(itemF3);
    item1D->addChild(itemF4);
    item1D->addChild(itemFmean);
    item1D->addChild(itemFmed);
    item1D->addChild(itemR1);
    item1D->addChild(itemR2);
    item1D->addChild(itemR3);
    item1D->addChild(itemR4);
    item1D->addChild(itemZ);
    item1D->addChild(itemZ2);

    items.append(itemCh);
    items.append(itemC1);
    items.append(itemC2);
    items.append(itemC3);
    items.append(itemC4);
    items.append(itemFitted);
    items.append(itemF1);
    items.append(itemF2);
    items.append(itemF3);
    items.append(itemF4);
    items.append(itemFmean);
    items.append(itemFmed);
    items.append(itemR1);
    items.append(itemR2);
    items.append(itemR3);
    items.append(itemR4);
    items.append(itemZ);
    items.append(itemZ2);

    QTreeWidgetItem* item2D = new QTreeWidgetItem();
    item2D->setText( 0, tr("2-D"));

    QTreeWidgetItem* itemC12 = new DiagramTreeWidgetItem(HIST_CHANNEL12);
    itemC12->setText( 0, tr("Channel-1-2"));
    QTreeWidgetItem* itemC23 = new DiagramTreeWidgetItem(HIST_CHANNEL23);
    itemC23->setText( 0, tr("Channel-2-3"));
    QTreeWidgetItem* itemC34 = new DiagramTreeWidgetItem(HIST_CHANNEL34);
    itemC34->setText( 0, tr("Channel-3-4"));
    QTreeWidgetItem* itemC14 = new DiagramTreeWidgetItem(HIST_CHANNEL14);
    itemC14->setText( 0, tr("Channel-1-4"));
    QTreeWidgetItem* itemC13 = new DiagramTreeWidgetItem(HIST_CHANNEL13);
    itemC13->setText( 0, tr("Channel-1-3"));
    QTreeWidgetItem* itemC24 = new DiagramTreeWidgetItem(HIST_CHANNEL24);
    itemC24->setText( 0, tr("Channel-2-4"));

    QTreeWidgetItem* itemZ12 = new DiagramTreeWidgetItem(HIST_Z12);
    itemZ12->setText( 0, tr("Charge-1-2"));
    QTreeWidgetItem* itemZ23 = new DiagramTreeWidgetItem(HIST_Z23);
    itemZ23->setText( 0, tr("Charge-2-3"));
    QTreeWidgetItem* itemZ34 = new DiagramTreeWidgetItem(HIST_Z34);
    itemZ34->setText( 0, tr("Charge-3-4"));
    QTreeWidgetItem* itemZ14 = new DiagramTreeWidgetItem(HIST_Z14);
    itemZ14->setText( 0, tr("Charge-1-4"));
    QTreeWidgetItem* itemZ13 = new DiagramTreeWidgetItem(HIST_Z13);
    itemZ13->setText( 0, tr("Charge-1-3"));
    QTreeWidgetItem* itemZ24 = new DiagramTreeWidgetItem(HIST_Z24);
    itemZ24->setText( 0, tr("Charge-2-4"));

    item2D->addChild(itemC12);
    item2D->addChild(itemC23);
    item2D->addChild(itemC34);
    item2D->addChild(itemC14);
    item2D->addChild(itemC13);
    item2D->addChild(itemC24);
    item2D->addChild(itemZ12);
    item2D->addChild(itemZ23);
    item2D->addChild(itemZ34);
    item2D->addChild(itemZ14);
    item2D->addChild(itemZ13);
    item2D->addChild(itemZ24);

    items.append(itemC12);
    items.append(itemC23);
    items.append(itemC34);
    items.append(itemC14);
    items.append(itemC13);
    items.append(itemC24);
    items.append(itemZ12);
    items.append(itemZ23);
    items.append(itemZ34);
    items.append(itemZ14);
    items.append(itemZ13);
    items.append(itemZ24);

    ui->treeWidget->addTopLevelItem(item1D);
    ui->treeWidget->addTopLevelItem(item2D);
}

void
MainWindow::triggersItemChanged(int value)
{
    char buf[5] = "T000";
    buf[3] += value;

    command_thread->writeCommand( buf, towrite);
    QString message = (value) ? tr("Triggers activated") : tr("Triggers diactivated");
    statusBar()->showMessage( message, 1000);
}

void
MainWindow::motorItemChanged(int value)
{
    char buf[5] = "M000";
    buf[1] += value;
    int steps = ui->scanningStepSpinBox->value();
    local_itoa( steps, buf + 2, 2);

    command_thread->writeCommand( buf, towrite);
    QString message;
    switch (value) {
    case 0:
        message = QString(tr("Motor stopped"));
        break;
    case 1:
        message = QString(tr("Hide away"));
        break;
    case 2:
    default:
        message = QString(tr("Move out"));
        break;
    }
    statusBar()->showMessage( message, 1000);
}

void
MainWindow::createRootHistograms()
{
    QMap< DiagramType, DiagramTuple > root_diagrams;

    int i = 0;
    while (hist1params[i].type != NONE) {
        TH1* h1 = nullptr;
        if (hist1params[i].type == HIST_FITALL) {
            h1 = new TH1F( hist1params[i].name, hist1params[i].title,
                hist1params[i].bins, hist1params[i].min, hist1params[i].max);
        }
        else {
            h1 = new TH1I( hist1params[i].name, hist1params[i].title,
                hist1params[i].bins, hist1params[i].min, hist1params[i].max);
        }
        h1->SetFillColor(kViolet + 2);
        h1->SetFillStyle(3001);

        TH2* h2 = nullptr;
        root_diagrams.insert( hist1params[i].type, std::make_tuple( h1, h2));

        ++i;
    }

    i = 0;
    while (hist2params[i].type != NONE) {
        TH2I* h2 = new TH2I( hist2params[i].name, hist2params[i].title,
            hist2params[i].xbins, hist2params[i].xmin, hist2params[i].xmax,
            hist2params[i].ybins, hist2params[i].ymin, hist2params[i].ymax);
        h2->SetStats(kFALSE);

        TH1* h1 = nullptr;
        root_diagrams.insert( hist2params[i].type, std::make_tuple( h1, h2));

        ++i;
    }

#if defined(_MSC_VER) && (_MSC_VER < 1900)
    for ( QList<QTreeWidgetItem*>::iterator it = items.begin(); it != items.end(); ++it) {
        QTreeWidgetItem* item = *it;
#elif defined(__GNUG__) && (__cplusplus >= 201103L)
    for ( QTreeWidgetItem* item : items) {
#endif
        DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(item);
        if (ditem) {
            DiagramType type = ditem->diagramType();
            ditem->setDiagramTuple(root_diagrams[type]);
        }
    }
}

void
MainWindow::commandThreadStarted()
{
    qDebug() << "GUI: Command thread started";
}

void
MainWindow::commandThreadFinished()
{
    qDebug() << "GUI: Command thread finished";
}

void
MainWindow::acquireThreadStarted()
{
    qDebug() << "GUI: Data acquisition started";
}

void
MainWindow::acquireThreadFinished()
{
    qDebug() << "GUI: Data acquisition finished";
}

void
MainWindow::processThreadStarted()
{
    qDebug() << "GUI: Data processing started";

    int n = ui->runNumberSpinBox->value();

    QDateTime dt = QDateTime::currentDateTime();
    QString dt_string = dt.toString("ddMMyyyy_hhmmss");
    QString namerun, nametxt, nameraw;

    if (flag_background) {
        namerun = QString("Run%1_back_%2.dat").arg( int(n), int(4), int(10), QLatin1Char('0')).arg(dt_string);
        nametxt = QString("Run%1_back_%2.txt").arg( int(n), int(4), int(10), QLatin1Char('0')).arg(dt_string);
        nameraw = QString("Run%1_back_%2.raw").arg( int(n), int(4), int(10), QLatin1Char('0')).arg(dt_string);
    }
    else {
        namerun = QString("Run%1_data_%2.dat").arg( int(n), int(4), int(10), QLatin1Char('0')).arg(dt_string);
        nametxt = QString("Run%1_data_%2.txt").arg( int(n), int(4), int(10), QLatin1Char('0')).arg(dt_string);
        nameraw = QString("Run%1_data_%2.raw").arg( int(n), int(4), int(10), QLatin1Char('0')).arg(dt_string);
    }

    QDir* dir = new QDir(rundir);
    QString filenamedat = dir->filePath(namerun);
    QString filenametxt = dir->filePath(nametxt);
    QString filenameraw = dir->filePath(nameraw);
    delete dir;

    // clear list of received data
    ui->runDetailsListWidget->clear();

    if (flag_write_run) {
        filerun = new QFile(filenamedat);
        filetxt = new QFile(filenametxt);
        filedat = new QFile(filenameraw);
        filerun->open(QFile::WriteOnly);
        filetxt->open(QFile::WriteOnly);
        filedat->open(QFile::WriteOnly);

        // write pedestals flag
        QDataStream out(filedat);
        out << quint8(flag_background);
    }

    if (filetxt && filetxt->isOpen()) {
        // write data file name and pedestals flag
        QTextStream out(filetxt);
        QString text = QString("Run%1").arg( int(n), int(4), int(10), QLatin1Char('0'));
        out << text << " " << int(flag_background) << endl;
    }

    // clear diagrams and update canvas

    runinfo.clear();
    updateRunInfo();

    ui->actionOpenRun->setEnabled(false);
    ui->actionSaveRun->setEnabled(false);
    ui->actionSettings->setEnabled(false);

    ui->startRunButton->setEnabled(false);
    ui->stopRunButton->setEnabled(true);
    ui->runNumberSpinBox->setEnabled(false);
    ui->runTypeGroupBox->setEnabled(false);
}

void
MainWindow::processThreadFinished()
{
    qDebug() << "GUI: Data processing finished";

    QThreadPool *threadPool = QThreadPool::globalInstance();
    threadPool->waitForDone();

    if (flag_write_run) {
        // name of file for batch processes
        profile_thread->setFile( filerun->fileName(), flag_background);

        filerun->flush();
        filerun->close();
        delete filerun;
        filerun = nullptr;

        filetxt->flush();
        filetxt->close();
        delete filetxt;
        filetxt = nullptr;

        filedat->flush();
        filedat->close();
        delete filedat;
        filedat = nullptr;
    }

    // if it was a background run, then save background results
    QAbstractButton* button = ui->runTypeButtonGroup->checkedButton();
    QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);
    if (rbutton == ui->fixedRunRadioButton) {
//        updateDiagrams();
    }
    else if (rbutton == ui->backgroundRunRadioButton) {
        // save mean, RMS
        const SignalArray& back = params->background();

        for ( int i = 0; i < CHANNELS; ++i) {
            const SignalPair& pair = back[i];
            QTableWidgetItem* item = ui->runInfoTableWidget->item( i + 1, 0);
            QString str = SignalValueDelegate::form_text(pair);
            item->setText(str);
        }
        statusBar()->showMessage( tr("Background data saved"), 2000);
    }

    ui->actionOpenRun->setEnabled(true);
    ui->actionSaveRun->setEnabled(true);
    ui->actionSettings->setEnabled(true);

    ui->startRunButton->setEnabled(true);
    ui->stopRunButton->setEnabled(false);
    ui->runNumberSpinBox->setEnabled(true);
    int run = ui->runNumberSpinBox->value();
    if (run != ui->runNumberSpinBox->maximum())
        ui->runNumberSpinBox->setValue(++run);

    ui->runTypeGroupBox->setEnabled(true);
}

void
MainWindow::processFileStarted()
{
    qDebug() << "GUI: File processing started";
    progress_dialog->show();

//    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void
MainWindow::processFileFinished()
{
    qDebug() << "GUI: File processing finished";
    if (!progress_dialog->isHidden()) {
        progress_dialog->hide();
    }

    // if it was a background file, then save background results
    if (profile_thread->isBackground()) {
        const SignalArray& back = params->background();

        for ( int i = 0; i < CHANNELS; ++i) {
            const SignalPair& pair = back[i];
            QTableWidgetItem* item = ui->runInfoTableWidget->item( i + 1, 0);
            QString str = SignalValueDelegate::form_text(pair);
            item->setText(str);
        }

        statusBar()->showMessage( tr("Background data loaded"), 2000);

        // recalculate channels calibration with new background
        params->recalculate();
    }

    int run_items = ui->runDetailsListWidget->count();
    for ( int i = 0; i < run_items; ++i) {
        QListWidgetItem* item = ui->runDetailsListWidget->item(i);
        RunDetailsListWidgetItem* ritem = dynamic_cast<RunDetailsListWidgetItem*>(item);
        ritem->update_text();
    }

    runinfo = profile_thread->runInfo();
    updateRunInfo();

    emit updateDiagram();

    QApplication::restoreOverrideCursor();
}

void
MainWindow::startRun()
{
    if (acquire_thread->isRunning()) {
        QMessageBox::warning( this, tr("Error"),
            tr("Acquisition thread is still running."), QMessageBox::Ok | QMessageBox::Default);
        return;
    }

    if (process_thread->isRunning()) {
        QMessageBox::warning( this, tr("Error"),
            tr("Processing thread is still running."), QMessageBox::Ok | QMessageBox::Default);
        return;
    }

    if (ui->dataUpdateAutoRadioButton->isChecked())
        timer_data->start();

    acquire_thread->start();
    process_thread->start();
}

void
MainWindow::stopRun()
{
    if (ui->dataUpdateAutoRadioButton->isChecked())
        timer_data->stop();

    acquire_thread->stop();
    process_thread->stop();
}

void
MainWindow::saveRun()
{
    // Save ROOT file
    QString fileName = QFileDialog::getSaveFileName( this,
        tr("Save ROOT file"), rundir, tr("ROOT Files (*.root)"));

    if (fileName.isEmpty())
        return;

#if QT_VERSION >= 0x050000
    std::string std_fileName = fileName.toStdString();
    TFile* rootfile = new TFile( std_fileName.c_str(), "CREATE");
#elif (QT_VERSION >= 0x040000 && QT_VERSION < 0x050000)
    TFile* rootfile = new TFile( fileName.toAscii(), "CREATE");
#endif

    if (rootfile->IsOpen()) {
        QTreeWidgetItemIterator iter(ui->treeWidget);
        while (*iter) {
            DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(*iter);

            if (ditem) {
                TH1* h1 = ditem->getTH1();
                TH2* h2 = ditem->getTH2();
                if (h1) h1->Write();
                if (h2) h2->Write();
            }
            ++iter;
        }
        rootfile->Close();
    }
    delete rootfile;
}

void
MainWindow::acquisitionTimingChanged(int value)
{
    QComboBox* combobox = qobject_cast<QComboBox*>(sender());

    int acquisition_time = 5; // 600 ms
    int delay_time = 2; // 100 ms

    if (combobox == ui->acquisitionTimeComboBox) {
        acquisition_time = value;
        delay_time = ui->delayTimeComboBox->currentIndex();
    }
    else if (combobox == ui->delayTimeComboBox) {
        acquisition_time = ui->acquisitionTimeComboBox->currentIndex();
        delay_time = value;
    }

    char buf[5] = "A100";
    buf[2] = delay_time + '0';

    if (acquisition_time >= 0 && acquisition_time <= 9)
        buf[3] = acquisition_time + '0';
    else if (acquisition_time >= 10 && acquisition_time <= 15)
        buf[3] = (acquisition_time - 10) + 'A';
    else
        buf[3] = '5';

    command_thread->writeCommand( buf, towrite);
    statusBar()->showMessage( tr("Extraction signal update"), 1000);
}

void
MainWindow::runTypeChanged(int id)
{
    QAbstractButton* button = ui->runTypeButtonGroup->button(id);
    QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

    flag_background = false;

    if (rbutton == ui->backgroundRunRadioButton) {
        flag_background = true;
        ui->triggersComboBox->setEnabled(false);
        ui->triggersComboBox->setCurrentIndex(4); // "T004"
        process_thread->setBackground(true);
    }
    else if (rbutton == ui->fixedRunRadioButton) {
        ui->triggersComboBox->setEnabled(true);
        ui->triggersComboBox->setCurrentIndex(0);
        process_thread->setBackground(false);
    }
}

void
MainWindow::dataUpdateChanged(int id)
{
    QAbstractButton* button = ui->updateDataButtonGroup->button(id);
    QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

    int state = 0;
    int delay_time = 0;
    int acquisition_time = 0;

    if (rbutton == ui->dataUpdateStartRadioButton) {
        qDebug() << "Extraction signal update";
//        flag_batch_state = false;
        disconnect( timer_data, SIGNAL(timeout()), this, SLOT(processData()));
        connect( command_thread, SIGNAL(signalExternalSignal()), this, SLOT(externalSignalReceived()));
        connect( command_thread, SIGNAL(signalNewBatchState(bool)), this, SLOT(newBatchStateReceived(bool)));
        delay_time = ui->delayTimeComboBox->currentIndex();
        acquisition_time = ui->acquisitionTimeComboBox->currentIndex();
        state = 1;
    }
    else if (rbutton == ui->dataUpdateAutoRadioButton) {
        qDebug() << "Automatic timeout update";
//        flag_batch_state = true;
        connect( timer_data, SIGNAL(timeout()), this, SLOT(processData()));
        disconnect( command_thread, SIGNAL(signalExternalSignal()), this, SLOT(externalSignalReceived()));
        disconnect( command_thread, SIGNAL(signalNewBatchState(bool)), this, SLOT(newBatchStateReceived(bool)));
    }

    ui->acquisitionTimeComboBox->setEnabled(state);
    ui->delayTimeComboBox->setEnabled(state);

    char buf[5] = "A000";
    buf[1] = state + '0';
    buf[2] = delay_time + '0';

    if (acquisition_time >= 0 && acquisition_time <= 9)
        buf[3] = acquisition_time + '0';
    else if (acquisition_time >= 10 && acquisition_time <= 15)
        buf[3] = (acquisition_time - 10) + 'A';
    else
        buf[3] = '5';

    command_thread->writeCommand( buf, towrite);
    if (state)
        statusBar()->showMessage( tr("Extraction signal update"), 1000);
    else
        statusBar()->showMessage( tr("Automatic timeout update"), 1000);
}

void
MainWindow::setRunSettings()
{
    SettingsDialog* dialog = new SettingsDialog(this);
    dialog->setSettingsParameters( settings, hist1params, hist2params);
    dialog->setWindowModality(Qt::WindowModal);
    int res = dialog->exec();
    if (res == QDialog::Accepted) {
        rundir = settings->value( "run-directory", "/home").toString();
        flag_write_run = settings->value( "write-run", true).toBool();
        int update_period = settings->value( "update-timeout", 3).toInt() * 1000;
        timer_data->setInterval(update_period);

        updateDiagrams();
    }

    delete dialog;
}

void
MainWindow::openRun()
{
    openFile(false);
}

void
MainWindow::openBackRun()
{
    openFile(true);
}

void
MainWindow::openFile(bool background_data)
{
    flag_background = background_data;

    if (process_thread->isRunning()) {
        QMessageBox::warning( this, tr("Error"), \
            tr("Processing thread is still running."), \
            QMessageBox::Ok | QMessageBox::Default);
        return;
    }
/*
    QString fileName = QFileDialog::getOpenFileName( this,
        tr("Open File"), rundir, tr("Run Files *.dat (*.dat);;Run Files *.txt (*.txt)"));

    if (fileName.isEmpty())
        return;
*/

    QFileDialog* dialog = new QFileDialog( this, tr("Open File"));
    QStringList fileNames;
    QString fileName;
    QString filter;

    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::ExistingFile);

#if QT_VERSION >= 0x050000
    QStringList filters;
    filters << tr("Run Files *.txt (*.txt)") \
            << tr("Run Files *.dat (*.dat)") \
            << tr("Run Files *.raw (*.raw)");

    dialog->setNameFilters(filters);
#elif (QT_VERSION >= 0x040000 && QT_VERSION < 0x050000)
    dialog->setFilter(tr("Run Files *.txt (*.txt);;Run Files *.dat (*.dat);;Run Files *.raw (*.raw)"));
#endif

    dialog->setDirectory(rundir);

    if(dialog->exec()) {
        fileNames = dialog->selectedFiles();

        if(!fileNames.isEmpty()) {
            fileName = fileNames[0];
#if QT_VERSION >= 0x050000
            filter = dialog->selectedNameFilter();
#elif (QT_VERSION >= 0x040000 && QT_VERSION < 0x050000)
            filter = dialog->selectedFilter();
#endif
        }
    }
    delete dialog;
/*
    if (fileNames.size() == 1) {
        if(!fileName.isEmpty())
            loadFile( fileName, filter);
    }
    else if (fileNames.size() > 1) {
        loadFiles( fileNames, filter);
    }
*/
    if (fileName.isEmpty())
        return;

    qDebug() << fileName;
    if (filter == tr("Run Files *.dat (*.dat)")) {
        // Open raw data run file
        QFile* runfile = new QFile(fileName);
        runfile->open(QFile::ReadOnly);
        if (runfile->isOpen()) {
            ui->runDetailsListWidget->clear();

            // read and process data
            qint64 datasize = runfile->size();
            runfile->close();

            // clear diagrams
            QTreeWidgetItemIterator iter(ui->treeWidget);
            while (*iter) {
                DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(*iter);
                if (ditem) {
                    TH1* h1 = ditem->getTH1();
                    TH2* h2 = ditem->getTH2();
                    if (h1) h1->Reset();
                    if (h2) h2->Reset();
                }
                ++iter;
            }

            progress_dialog->setRange( 0, datasize);

            profile_thread->setFile( fileName, flag_background);
//          updateDiagrams(background_data);
            profile_thread->start();
        }
        delete runfile;
    }
    else if (filter == tr("Run Files *.txt (*.txt)")) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        ui->runDetailsListWidget->clear();

        QList<QListWidgetItem*> details_items;
        QFile* runfile = new QFile(fileName);
        QString runnumber;
        runfile->open(QFile::ReadOnly);
        if (runfile->isOpen()) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            runnumber = processTextFile( runfile, details_items);
            QApplication::restoreOverrideCursor();
        }

        QFileInfo info( QDir(rundir), runnumber + ".dat");
        if (info.exists()) {
            QString fname = info.absoluteFilePath();

            qDebug() << "file OK: " << fname;

            // clear diagrams
            QTreeWidgetItemIterator iter(ui->treeWidget);
            while (*iter) {
                DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(*iter);
                if (ditem) {
                    TH1* h1 = ditem->getTH1();
                    TH2* h2 = ditem->getTH2();
                    if (h1) h1->Reset();
                    if (h2) h2->Reset();
                }
                ++iter;
            }

            progress_dialog->setRange( 0, details_items.size());

            profile_thread->setBatches( fname, details_items, flag_background);
//          updateDiagrams(background_data);
            profile_thread->start();
        }
        else {
            QMessageBox::warning( this, tr("Error"),
                tr("Data file is absent!"),
                QMessageBox::Ok | QMessageBox::Default);
        }
        delete runfile;
    }
    else if (filter == tr("Run Files *.raw (*.raw)")) {
//        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        ui->runDetailsListWidget->clear();

        QList<QListWidgetItem*> details_items;
        QFile* runfile = new QFile(fileName);

        runfile->open(QFile::ReadOnly);
        if (runfile->isOpen()) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            flag_background = processRawFile( runfile, details_items);
            QApplication::restoreOverrideCursor();
        }

        delete runfile;

        // clear diagrams
        QTreeWidgetItemIterator iter(ui->treeWidget);
        while (*iter) {
            DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(*iter);
            if (ditem) {
                TH1* h1 = ditem->getTH1();
                TH2* h2 = ditem->getTH2();
                if (h1) h1->Reset();
                if (h2) h2->Reset();
            }
            ++iter;
        }

        progress_dialog->setRange( 0, details_items.size());

        profile_thread->setBatches( fileName, details_items, flag_background);
        profile_thread->start();
    }
    else {
        QMessageBox::warning( this, tr("Error"),
            tr("Wrong file filter!"),
            QMessageBox::Ok | QMessageBox::Default);
    }
}

void
MainWindow::connectDevices()
{
#ifdef Q_OS_LINUX
    FT_SetVIDPID( 0x0403, 0x6010);
#endif

    char* name = const_cast<char*>(description_channel_a);

    FT_STATUS ftStatus = FT_OpenEx( name, FT_OPEN_BY_DESCRIPTION, &channel_a);
    if (!FT_SUCCESS(ftStatus)) {
        QMessageBox::warning( this, tr("Unable to open the FT2232H device"), \
            tr("Error during connection of FT2232H Channel A. This can fail if the ftdi_sio\n" \
               "driver is loaded, use lsmod to check this and rmmod ftdi_sio\n" \
               "to remove also rmmod usbserial."));
        statusBar()->showMessage( tr("Channel A connection canceled"), 2000);
        return;
    }
    ftStatus = FT_SetTimeouts( channel_a, 8000, 8000);
    if (!FT_SUCCESS(ftStatus)) {
        deviceError( channel_a, ftStatus);
        return;
    }

    name = const_cast<char*>(description_channel_b);

    ftStatus = FT_OpenEx( name, FT_OPEN_BY_DESCRIPTION, &channel_b);
    if (!FT_SUCCESS(ftStatus)) {
        QMessageBox::warning( this, tr("Unable to open the FT2232H device"), \
            tr("Error during connection of FT2232H Channel B. This can fail if the ftdi_sio\n" \
               "driver is loaded, use lsmod to check this and rmmod ftdi_sio\n" \
               "to remove also rmmod usbserial."));
        statusBar()->showMessage( tr("Channel B connection canceled"), 2000);
        return;
    }
    ftStatus = FT_SetTimeouts( channel_b, 8000, 8000);
    if (!FT_SUCCESS(ftStatus)) {
        deviceError( channel_b, ftStatus);
        return;
    }
    command_thread->setDeviceHandle(channel_a);
    acquire_thread->setDeviceHandle(channel_b);

    command_thread->start();

    ui->connectButton->setEnabled(false);
    ui->disconnectButton->setEnabled(true);
    ui->dataUpdateGroupBox->setEnabled(true);
    ui->detectorsPositionGroupBox->setEnabled(true);
    ui->acquisitionGroupBox->setEnabled(true);
    ui->runTypeGroupBox->setEnabled(true);
    ui->scanningRunRadioButton->setEnabled(false);
    ui->startRunButton->setEnabled(true);
    ui->stopRunButton->setEnabled(false);

    // stop motor
    int mindex = ui->motorComboBox->currentIndex();
    motorItemChanged(mindex);

    // stop internal trigger
    int cindex = ui->triggersComboBox->currentIndex();
    triggersItemChanged(cindex);

    // reset ALTERA
    resetAlteraClicked();

    // set delay
    int delay = ui->delaySpinBox->value();
    setDelayChanged(delay);

    // set automatic update
    ui->dataUpdateAutoRadioButton->setChecked(true);
    int button_id = ui->updateDataButtonGroup->id(ui->dataUpdateAutoRadioButton);
    dataUpdateChanged(button_id);

    sys_status = STATUS_IDLE;
    emit signalSystemCurrentStatus(sys_status);
}

void
MainWindow::disconnectDevices()
{
    bool process_state = process_thread->isRunning();
    bool acquire_state = acquire_thread->isRunning();
    if (process_state && acquire_state)
        stopRun();

    if (command_thread->isRunning())
        command_thread->stop();

    FT_STATUS ftStatus = FT_Close(channel_a);
    if (!FT_SUCCESS(ftStatus)) {
        std::cerr << "FT2232H Channel A close error." << std::endl;
    }
    ftStatus = FT_Close(channel_b);
    if (!FT_SUCCESS(ftStatus)) {
        std::cerr << "FT2232H Channel B close error." << std::endl;
    }

    if (timer_data->isActive())
        timer_data->stop();

    disconnect( timer_data, SIGNAL(timeout()), this, SLOT(processData()));
    disconnect( command_thread, SIGNAL(signalExternalSignal()), this, SLOT(externalSignalReceived()));
    disconnect( command_thread, SIGNAL(signalNewBatchState(bool)), this, SLOT(newBatchStateReceived(bool)));

    ui->connectButton->setEnabled(true);
    ui->disconnectButton->setEnabled(false);
    ui->dataUpdateGroupBox->setEnabled(false);
    ui->detectorsPositionGroupBox->setEnabled(false);
    ui->acquisitionGroupBox->setEnabled(false);
    ui->runTypeGroupBox->setEnabled(false);
    ui->startRunButton->setEnabled(false);
    ui->stopRunButton->setEnabled(false);

    sys_status = STATUS_DEVICE_DISCONNECTED;
    emit signalSystemCurrentStatus(sys_status);
}

void
MainWindow::commandDeviceError()
{
    FT_STATUS ftState = command_thread->deviceStatus();
    deviceError( channel_a, ftState);
    statusBar()->showMessage(tr("FT2232H Channel A error"));
}

void
MainWindow::acquireDeviceError()
{
    FT_STATUS ftState = acquire_thread->deviceStatus();
    deviceError( channel_b, ftState);
    statusBar()->showMessage(tr("FT2232H Channel B error"));

    stopRun();
}

void
MainWindow::externalSignalReceived( int value, const QDateTime& timestamp)
{
    qDebug() << "GUI: External signal received: " << value << " timestamp: " << timestamp.toString();
}

/// if rising edge (state is high), then process data
/// else ignore data
void
MainWindow::newBatchStateReceived(bool state)
{
    qDebug() << "GUI: Batch signal state -- " << state;
    if (process_thread->isRunning() && state) {
//        statusBar()->showMessage( tr("New batch signal"), 1000);
        processData();
//        QTimer::singleShot( 2000, this, SLOT(processData()));
    }
    else if (process_thread->isRunning() && !state) {
        // get any processed data (just to delete any of them)
        CountsList countslist;
        DataList datalist;
        process_thread->getProcessedData( datalist, countslist);
    }
}

void
MainWindow::movementFinished()
{
    statusBar()->showMessage( tr("Movement finished"), 1000);

    QMessageBox msgBox(this);
    ui->motorComboBox->setCurrentIndex(0);
    msgBox.setText(tr("Movement has been finished"));
    msgBox.exec();
}

QString
MainWindow::processTextFile( QFile* runfile, QList<QListWidgetItem *>& items)
{
    QTextStream in(runfile);
    QString line;
    QString run_number, datestring, timestring;
    int background_data, batchoffset, bytes, events;

    in >> run_number >> background_data;
    flag_background = bool(background_data);

//    qDebug() << run_number << " " << background_data;

    int offset = 0;
    int batchnumber = 1;
    while (!in.atEnd()) {
        line = in.readLine();
        if (!line.isEmpty()) {
            QTextStream sline( &line, QIODevice::ReadOnly);
            sline >> datestring >> timestring >> batchoffset >> bytes >> events;
            QDate date = QDate::fromString( datestring, "dd.MM.yyyy");
            QTime time = QTime::fromString( timestring, "hh:mm:ss");
//            qDebug() << datestring << " " << timestring << " " << bytes << " " << events;
            QDateTime dtime( date, time);
            QListWidgetItem *item = new RunDetailsListWidgetItem( dtime,
                batchnumber, bytes, events, 0, offset, ui->runDetailsListWidget);
            offset += bytes;
            batchnumber++;
            items.append(item);
        }
    }
    return run_number;
}

bool
MainWindow::processRawFile( QFile* runfile, QList<QListWidgetItem*>& items)
{
    QDataStream in(runfile);
    in.setByteOrder(QDataStream::LittleEndian);


    quint8 background_data;
    quint64 datetime;
    quint32 bytes_size;

    in >> background_data;

    qint64 offset = 0;
    int batchnumber = 1;
    while (!in.atEnd()) {
        in >> datetime >> bytes_size;
        QDateTime dtime;
        dtime.setTime_t(datetime);
        offset = runfile->pos();

        QListWidgetItem *item = new RunDetailsListWidgetItem( dtime,
            batchnumber, bytes_size, 0, 0, offset, ui->runDetailsListWidget);
        batchnumber++;
        items.append(item);
        in.skipRawData(bytes_size);
    }
    return bool(background_data);
}

void
MainWindow::processData()
{
    if (ui->dataUpdateAutoRadioButton->isChecked()) {
        qDebug() << "GUI: data timer is stopped";
        timer_data->stop();
    }

    CountsList countslist;
    DataList datalist;
    process_thread->getProcessedData( datalist, countslist);

    size_t listsize = countslist.size();
    statusBar()->showMessage( tr("Events received: %1").arg(listsize), 1000);

    QDateTime datetime = QDateTime::currentDateTime();

    // process data
    RunInfo batch_info;
    if (listsize)
        batch_info = batchCountsReceived(countslist);

    if (filerun && filedat)
        batchDataReceived( datalist, datetime);

    int batch_counts = ui->runDetailsListWidget->count();

    size_t batch_data_offset = 0;

    if (batch_counts) {
        QListWidgetItem* item = ui->runDetailsListWidget->item(batch_counts - 1);
        RunDetailsListWidgetItem* ritem = dynamic_cast<RunDetailsListWidgetItem*>(item);
        if (ritem)
           batch_data_offset = ritem->batch_offset() + ritem->batch_bytes();
    }
/*
    RunDetailsListWidgetItem* item = new RunDetailsListWidgetItem( dtime, \
        (batch_counts + 1), datalist.size(), countslist.size(), \
        batch_data_offset, ui->runDetailsListWidget);
*/
    RunDetailsListWidgetItem* item = new RunDetailsListWidgetItem( datetime, \
        (batch_counts + 1), datalist.size(),
        batch_info.counted(), batch_info.processed(), \
        batch_data_offset, ui->runDetailsListWidget);

    if (filetxt && filetxt->isOpen()) {
//        qDebug() << item->batch_offset() << " " << item->batch_bytes() << " " << item->batch_events();
        QTextStream out(filetxt);
        out << item->file_string() << endl;
    }
    updateRunInfo();

    RunInfo::BeamSpectrumArray batch_array = batch_info.averageComposition();
    RunInfo::BeamSpectrumArray mean_array = runinfo.averageComposition();

    emit signalBeamSpectrumChanged( batch_array, mean_array, datetime);

    if (ui->dataUpdateAutoRadioButton->isChecked()) {
        qDebug() << "GUI: data timer is started";
        timer_data->start();
    }
}

void
MainWindow::deviceError( FT_HANDLE dev, FT_STATUS ftStatus)
{
    disconnectDevices();

    QString msg;
    switch (ftStatus) {
    case FT_INVALID_HANDLE:
        msg = tr("Invalid handle");
        break;
    case FT_DEVICE_NOT_FOUND:
        msg = tr("Device not found");
        break;
    case FT_DEVICE_NOT_OPENED:
        msg = tr("Device not opened");
        break;
    case FT_IO_ERROR:
        msg = tr("IO error");
        break;
    case FT_INSUFFICIENT_RESOURCES:
        msg = tr("Insufficient resources");
        break;
    case FT_INVALID_PARAMETER:
        msg = tr("Invalid parameter");
        break;
    case FT_INVALID_BAUD_RATE:
        msg = tr("Invalid baud rate");
        break;
    case FT_DEVICE_NOT_OPENED_FOR_ERASE:
        msg = tr("Device not opened for erase");
        break;
    case FT_DEVICE_NOT_OPENED_FOR_WRITE:
        msg = tr("Device not opened for write");
        break;
    case FT_FAILED_TO_WRITE_DEVICE:
        msg = tr("Failed to write device");
        break;
    case FT_INVALID_ARGS:
        msg = tr("Invalid arguments");
        break;
    case FT_NOT_SUPPORTED:
        msg = tr("Device not supported");
        break;
    case FT_OTHER_ERROR:
    default:
        msg = tr("Other error");
        break;
    }

    QString channel;
    if (dev == channel_a)
        channel = tr("FT2232H Channel A error.");
    else if (dev == channel_b)
        channel = tr("FT2232H Channel B error.");
    else
        channel = tr("Unknown device error.");

    QMessageBox::warning( this, tr("Error message"),
        tr("%1\n\n%2").arg(channel).arg(msg),
        QMessageBox::Ok | QMessageBox::Default);
}

RunInfo
MainWindow::batchCountsReceived(const CountsList &list)
{
    SharedFitParameters params = FitParameters::instance();
    RunInfo info = params->fit( list, diagrams, process_thread->isBackground());
    runinfo += info;
    emit updateDiagram();
    return info;
}

void
MainWindow::batchDataReceived( const DataList& datalist, const QDateTime& dt)
{
    time_t dtime = dt.toTime_t();

    WriteDataProcess* writedata = new WriteDataProcess( filerun, datalist);
    writedata->setAutoDelete(true);
    WriteDataTimeProcess* writedatatime = new WriteDataTimeProcess( filedat, dtime, datalist);
    writedatatime->setAutoDelete(true);

    QThreadPool* threadPool = QThreadPool::globalInstance();
    threadPool->start(writedata);
    threadPool->start(writedatatime);
}

void
MainWindow::opcUaClientDialog(bool state)
{
    QString opcua_path = settings->value( "opcua-server-path", "opc.tcp://localhost").toString();
    int opcua_port = settings->value( "opcua-server-port", 4840).toInt();

    QString server_path = QString("%1:%2").arg(opcua_path).arg(opcua_port);

    if (!opcua_dialog) {
        opcua_dialog = new OpcUaClientDialog( server_path, opcua_client, state, this);
        connect( this, SIGNAL(signalBeamSpectrumChanged(RunInfo::BeamSpectrumArray,RunInfo::BeamSpectrumArray,QDateTime)),
            opcua_dialog, SLOT(setBreamSpectrumValue(RunInfo::BeamSpectrumArray,RunInfo::BeamSpectrumArray,QDateTime)));
        connect( opcua_client, SIGNAL(externalCommandChanged( int, QDateTime)),
            opcua_dialog, SLOT(setExtCommandValue( int, QDateTime)));
    }
    if (opcua_dialog)
        opcua_dialog->show();
}
/*
void
MainWindow::opcUaStartUp()
{
    QString opcua_path = settings->value( "opcua-server-path", "opc.tcp://localhost").toString();
    int opcua_port = settings->value( "opcua-server-port", 4840).toInt();

    QString server_path = QString("%1:%2").arg(opcua_path).arg(opcua_port);

    if (!opcua_dialog) {
        opcua_dialog = new OpcUaClientDialog( server_path, opcua_client, true, this);
        connect( this, SIGNAL(signalBeamSpectrumChanged(RunInfo::BeamSpectrumArray,RunInfo::BeamSpectrumArray,QDateTime)),
            opcua_dialog, SLOT(setBreamSpectrumValue(RunInfo::BeamSpectrumArray,RunInfo::BeamSpectrumArray,QDateTime)));
        connect( opcua_client, SIGNAL(externalCommandChanged( int, QDateTime)),
            opcua_dialog, SLOT(externalSignalReceived( int, QDateTime)));
    }
    if (opcua_dialog)
        opcua_dialog->show();
}
*/
void
MainWindow::saveSettings(QSettings* set)
{
    int i = 0;
    while (hist1params[i].type != NONE) {
        set->beginGroup(QString(hist1params[i].name));
        set->setValue( "min", hist1params[i].min);
        set->setValue( "max", hist1params[i].max);
        set->setValue( "bins", hist1params[i].bins);
        set->endGroup();
        ++i;
    }
    i = 0;
    while (hist2params[i].type != NONE) {
        set->beginGroup(QString(hist2params[i].name));
        set->setValue( "xmin", hist2params[i].xmin);
        set->setValue( "xmax", hist2params[i].xmax);
        set->setValue( "xbins", hist2params[i].xbins);
        set->setValue( "ymin", hist2params[i].ymin);
        set->setValue( "ymax", hist2params[i].ymax);
        set->setValue( "ybins", hist2params[i].ybins);
        set->endGroup();
        ++i;
    }
    set->setValue( "current-run", ui->runNumberSpinBox->value());

    set->setValue( "run-directory", rundir);
    set->setValue( "delay", ui->delaySpinBox->value());

    params->save(set);

    QSize wsize = this->size();
    set->setValue( "main-window-size", wsize);
}

void
MainWindow::loadSettings(QSettings* set)
{
    int i = 0;
    while (hist1params[i].type != NONE) {
        set->beginGroup(QString(hist1params[i].name));
        hist1params[i].min = set->value( "min", hist1params[i].min).toDouble();
        hist1params[i].max = set->value( "max", hist1params[i].max).toDouble();
        hist1params[i].bins = set->value( "bins", hist1params[i].bins).toInt();
        set->endGroup();
        ++i;
    }
    i = 0;
    while (hist2params[i].type != NONE) {
        set->beginGroup(QString(hist2params[i].name));
        hist2params[i].xmin = set->value( "xmin", hist2params[i].xmin).toDouble();
        hist2params[i].xmax = set->value( "xmax", hist2params[i].xmax).toDouble();
        hist2params[i].xbins = set->value( "xbins", hist2params[i].xbins).toInt();
        hist2params[i].ymin = set->value( "ymin", hist2params[i].ymin).toDouble();
        hist2params[i].ymax = set->value( "ymax", hist2params[i].ymax).toDouble();
        hist2params[i].ybins = set->value( "ybins", hist2params[i].ybins).toInt();
        set->endGroup();
        ++i;
    }
    int value = set->value( "current-run", 0).toInt();
    ui->runNumberSpinBox->setValue(value);

    rundir = set->value( "run-directory", "/home").toString();
    int delay = set->value( "delay", 0).toInt();
    ui->delaySpinBox->setValue(delay);

    flag_write_run = settings->value( "write-run", true).toBool();

    QSize wsize = set->value( "main-window-size", QSize( 500, 600)).toSize();
    resize(wsize);
}

void
MainWindow::closeEvent(QCloseEvent* event)
{
    int res = QMessageBox::Yes;
    if (acquire_thread->isRunning()) {
        res = QMessageBox::warning( this, tr("Close program"), \
            tr("Acquisition is still active.\nDo you want to quit program?"), \
            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    }

    if (res == QMessageBox::Yes) {
        if (opcua_client && opcua_client->isConnected()) {
            int value = 0;
            QDateTime now = QDateTime::currentDateTime();
            bool res = opcua_client->writeHeartBeatValue( value, now);
            std::cout << "OPC UA HeartBeat result: " << res << ", value: " << value << std::endl;
        }
        saveSettings(settings);
        event->accept();
    }
    else
        event->ignore();
}

void
MainWindow::resetAlteraClicked()
{
    char buf[5] = "R000";
    command_thread->writeCommand( buf, towrite);
    statusBar()->showMessage( tr("reset ALTERA"), 1000);
}

void
MainWindow::setDelayChanged(int delay)
{
    char buf[5] = "DXXX";
    local_itoa( delay, buf + 1);

    command_thread->writeCommand( buf, towrite);
    statusBar()->showMessage( tr("Delay %1").arg(delay), 1000);
}

void
MainWindow::updateDiagrams(bool background_data)
{
    Diagrams& d = diagrams;

    if (background_data) {
        for ( int i = 0; i < CHANNELS; ++i) {
            TH1* hist = d.channels[i];
            hist->SetBins( 500, 0., 4095.);
        }
    }
    else {
        TH1* h1 = d.channels[0];
        h1->SetBins( c1hp.bins, c1hp.min, c1hp.max);

        TH1* h2 = d.channels[1];
        h2->SetBins( c2hp.bins, c2hp.min, c2hp.max);

        TH1* h3 = d.channels[2];
        h3->SetBins( c3hp.bins, c3hp.min, c3hp.max);

        TH1* h4 = d.channels[3];
        h4->SetBins( c4hp.bins, c4hp.min, c4hp.max);

        d.fitall->SetBins( fhp.bins, fhp.min, fhp.max);
        d.fit_mean->SetBins( fhp.bins, fhp.min, fhp.max);
        d.fit_median->SetBins( fhp.bins, fhp.min, fhp.max);

        for ( int i = 0; i < CHANNELS; ++i) {
            TH1* rank = d.rank[i];
            TH1* fit = d.fit[i];
            rank->SetBins( fhp.bins, fhp.min, fhp.max);
            fit->SetBins( fhp.bins, fhp.min, fhp.max);
        }

        d.z->SetBins( zhp.bins, zhp.min, zhp.max);
        d.z2->SetBins( z2hp.bins, z2hp.min, z2hp.max);

        d.c12->SetBins( fhp.bins, fhp.min, fhp.max, fhp.bins, fhp.min, fhp.max);
        d.c23->SetBins( fhp.bins, fhp.min, fhp.max, fhp.bins, fhp.min, fhp.max);
        d.c34->SetBins( fhp.bins, fhp.min, fhp.max, fhp.bins, fhp.min, fhp.max);
        d.c13->SetBins( fhp.bins, fhp.min, fhp.max, fhp.bins, fhp.min, fhp.max);
        d.c14->SetBins( fhp.bins, fhp.min, fhp.max, fhp.bins, fhp.min, fhp.max);
        d.c24->SetBins( fhp.bins, fhp.min, fhp.max, fhp.bins, fhp.min, fhp.max);
        d.z12->SetBins( zhp.bins, zhp.min, zhp.max, zhp.bins, zhp.min, zhp.max);
        d.z23->SetBins( zhp.bins, zhp.min, zhp.max, zhp.bins, zhp.min, zhp.max);
        d.z34->SetBins( zhp.bins, zhp.min, zhp.max, zhp.bins, zhp.min, zhp.max);
        d.z13->SetBins( zhp.bins, zhp.min, zhp.max, zhp.bins, zhp.min, zhp.max);
        d.z14->SetBins( zhp.bins, zhp.min, zhp.max, zhp.bins, zhp.min, zhp.max);
        d.z24->SetBins( zhp.bins, zhp.min, zhp.max, zhp.bins, zhp.min, zhp.max);

    }

    emit updateDiagram();
}

void
MainWindow::resetDiagram(DiagramType type)
{
    DiagramTreeWidgetAction action(ui->treeWidget);
    action.resetDiagram(type);
}

void
MainWindow::backgroundValueChanged( int r, int c)
{
    // only rows with background info
    if (r > 0 && r <= CHANNELS) {
        QTableWidgetItem* item = ui->runInfoTableWidget->item( r, c);
        SignalPair p = SignalValueDelegate::parse_text(item->text());

        SignalArray& back = params->background();
        back[r - 1] = p;
    }
}

void
MainWindow::updateRunInfo()
{
    for ( int i = 0; i < CARBON_Z; ++i) {
        double charge_z = runinfo.averageComposition(i);
        QTableWidgetItem* item = ui->runInfoTableWidget->item( i + CARBON_Z, 0);
        if (charge_z >= 0.) {
            charge_z *= 100.;
            item->setText(QString("%1").arg( charge_z,  3, 'f', 1));
        }
    }

    // triggers information
    QTableWidgetItem* counted = ui->runInfoTableWidget->item( 13, 0);
    QTableWidgetItem* processed = ui->runInfoTableWidget->item( 14, 0);
    if (runinfo.counted()) {
#if defined(_MSC_VER)
        double percent = floor(1000. * runinfo.processed() / runinfo.counted()) / 10.;
#elif defined(__GNUC__)
        double percent = round(1000. * runinfo.processed() / runinfo.counted()) / 10.;
#endif
        counted->setText(QString("%1").arg(runinfo.counted()));
        processed->setText(QString("%1 (%2 %)").arg(runinfo.processed()).arg( percent, 0, 'g', 4));
    }
    else {
        counted->setText(QString("%1").arg(runinfo.counted()));
        processed->setText(QString("%1").arg(runinfo.processed()));
    }
}

void
MainWindow::fitChargeDiagram(DiagramType)
{
    // parent widget is a charge root canvas dialog
    QWidget* widget = qobject_cast<QWidget*>(sender());
    RootCanvasDialog* dialog = new RootCanvasDialog(widget);

    TH1* charge = dynamic_cast<TH1*>(diagrams.z->Clone("FitZ"));

    // draw charge hist in the canvas pad
    TPad* pad = dialog->getPad();
    pad->cd();

    int zmin = settings->value( "min-fit-charge", 1).toInt();
    int zmax = settings->value( "max-fit-charge", CARBON_Z).toInt();

    CalibrationFitting::BeamCompositionFit fbeam( charge, zmin, zmax, 0.3);
    TF1* fit = fbeam.fit( fbeam, charge, zmin, zmax, 0.5); // don't delete

    double fitint = fit->Integral( 0.0, 7.5); // fit integral
    double dataint = charge->GetEntries() * charge->GetBinWidth(1); // data integral
    double datapercent = floor(1000.0 * fitint / dataint) / 10.0;

    charge->Draw("lpe");

    Double_t* params = fbeam.fit_parameters();
    fit->GetParameters(params);

    TLegend* legend = new TLegend( 0.75, 0.15, 0.99, 0.55);
    legend->SetTextFont(70);
    legend->SetTextSize(0.03f);
    legend->AddEntry( charge, "Data", "lpe");
    legend->AddEntry( fit, TString::Format( "Total fit : %2.1f %%", datapercent));

//    const bool* fit_charge = CalibrationFitting::BeamCompositionFit::charge_in_fit();

    std::map< int, TF1* > charges;
    for ( int i = zmin; i <= zmax; i++) {
//        if (!(fit_charge[i - 1]))
//            continue;
        double p[gparams];
        size_t pos = gparams * (i - zmin);
        for ( int j = 0; j < gparams; ++j)
            p[j] = params[pos + j];

        TF1* ft = fbeam.fit_charge( i, 0.7); // don't delete
        charges.insert(std::make_pair( i, ft));
        ft->SetParameters(p);
        ft->SetLineColor(ccolors[i]);
        ft->SetFillColor(ccolors[i]);
        ft->SetFillStyle(3001);
        ft->SetLineWidth(1);
        ft->SetNpx(400);
        ft->Draw("same");

        double parint = ft->Integral( double(i) - .5, double(i) + .5);
        double percent = 100.0 * parint / dataint;//floor(10000.0 * parint / dataint) / 100.0;

        legend->AddEntry( ft, TString::Format( "Z = %d : %2.2f %%", i, percent), "l");
    }
    fit->Draw("same");
    legend->Draw();
//    charge->SetAxisRange( 0.5, 7.5);

    dialog->setModal(true);
    dialog->exec();

    for ( std::map< int, TF1* >::iterator iter = charges.begin(); iter != charges.end(); ++iter)
        delete (*iter).second;

    delete charge;
    delete [] params;
    delete legend;
    delete fit;
    delete dialog;
}

void
MainWindow::resetDiagramsClicked()
{
    int res = QMessageBox::warning( this, tr("Reset diagrams"),
        tr("Diagrams data will be lost. Do you want to proceed?"),
        QMessageBox::Ok, QMessageBox::Cancel | QMessageBox::Default);

    if (res == QMessageBox::Ok) {
        QTreeWidgetItemIterator iter(ui->treeWidget);
        while (*iter) {
            DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(*iter);
            if (ditem) {
                TH1* h1 = ditem->getTH1();
                TH2* h2 = ditem->getTH2();
                if (h1) h1->Reset();
                if (h2) h2->Reset();
            }
            ++iter;
        }
        emit updateDiagram();
    }
}

void
MainWindow::detailsClear()
{
    ui->runDetailsListWidget->clear();
}

void
MainWindow::detailsSelectAll()
{
    ui->runDetailsListWidget->selectAll();
}

void
MainWindow::detailsSelectNone()
{
    ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->runDetailsListWidget->setCurrentRow(-1);
    if (ui->actionDetailsSelectionSingle->isChecked())
        ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    else if (ui->actionDetailsSelectionMultiple->isChecked())
        ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    else if (ui->actionDetailsSelectionExtended->isChecked())
        ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else if (ui->actionDetailsSelectionContiguous->isChecked())
        ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
}

void
MainWindow::runDetailsSelectionTriggered(QAction* action)
{
    if (action == ui->actionDetailsSelectionSingle)
        ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    else if (action == ui->actionDetailsSelectionMultiple)
        ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    else if (action == ui->actionDetailsSelectionExtended)
        ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else if (action == ui->actionDetailsSelectionContiguous)
        ui->runDetailsListWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
}

void
MainWindow::processBatchesClicked()
{
    QList<QListWidgetItem*> details_items = ui->runDetailsListWidget->selectedItems();

    // clear diagrams
    QTreeWidgetItemIterator iter(ui->treeWidget);
    while (*iter) {
        DiagramTreeWidgetItem* ditem = dynamic_cast<DiagramTreeWidgetItem*>(*iter);
        if (ditem) {
            TH1* h1 = ditem->getTH1();
            TH2* h2 = ditem->getTH2();
            if (h1) h1->Reset();
            if (h2) h2->Reset();
        }
        ++iter;
    }
    progress_dialog->setRange( 0, details_items.size());

    profile_thread->processBatches( details_items, flag_background);
    profile_thread->start();
}

void
MainWindow::detailsItemSelectionChanged()
{
    QList<QListWidgetItem*> items = ui->runDetailsListWidget->selectedItems();
    bool no_data_processing = !process_thread->isRunning();
    bool no_file_processing = !profile_thread->isRunning();

    if (no_data_processing && no_file_processing && items.size()) {
        ui->processPushButton->setEnabled(true);
    }
    else {
        ui->processPushButton->setEnabled(false);
    }
}

void
MainWindow::onOpcUaClientConnected()
{
    ui->statusBar->showMessage( "OPC UA server connection successfull", 1000);
}

void
MainWindow::onOpcUaTimeout()
{
    if (opcua_client && opcua_client->isConnected()) {
        int value = 1;
        QDateTime now = QDateTime::currentDateTime();
        bool res = opcua_client->writeHeartBeatValue( value, now);
        if (opcua_dialog && res) opcua_dialog->setHeatBeatValue( value, now);
//        std::cout << "OPC UA HeartBeat result: " << res << ", value: " << value << std::endl;
    }
}

void
MainWindow::onOpcUaClientDisconnected()
{
}
