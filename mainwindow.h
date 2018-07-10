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

#pragma once

#include <QMainWindow>


#if defined(Q_OS_WIN32) && defined(__MINGW32__)
#include <windef.h>
#include <winbase.h>
#endif


#include <ftd2xx.h>

#include "channelscountsfit.h"

#include "runinfo.h"
#include "typedefs.h"

namespace Ui {
class MainWindow;
}

class QFile;
class QTimer;
class QTreeWidgetItem;
class QListWidgetItem;
class QCloseEvent;
class QProgressDialog;
class QSettings;
class QDateTime;
class QStateMachine;
class QState;

class CommandThread;
class AcquireThread;
class ProcessThread;
class ProcessFileThread;
class DiagramTreeWidgetItem;
class RootCanvasDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    virtual ~MainWindow();

public slots:
     static void test_me();

protected:
    void closeEvent(QCloseEvent*);

signals:
    void updateDiagram();

private slots:
    void handle_root_events();
    void treeWidgetItemDoubleClicked( QTreeWidgetItem*, int);
    void treeWidgetItemClicked( QTreeWidgetItem*, int);
    void runDetailsSelectionTriggered(QAction*);
    void externalSignalReceived();
    void newBatchStateReceived(bool);
    void movementFinished();
    void commandThreadStarted();
    void commandThreadFinished();
    void acquireThreadStarted();
    void acquireThreadFinished();
    void processFileStarted();
    void processFileFinished();
    void processThreadStarted();
    void processThreadFinished();
    void acquireDeviceError();
    void commandDeviceError();
    void connectDevices();
    void disconnectDevices();
    void startRun();
    void stopRun();
    void saveRun();
    void openRun();
    void openFile(bool background_data);
    void openBackRun();
    void setRunSettings();
    void resetAlteraClicked();
    void setDelayChanged(int);
    void updateDiagrams(bool background_data = false);
    void resetDiagram(DiagramType type);
    void runTypeChanged(int);
    void triggersItemChanged(int);
    void motorItemChanged(int);
    void acquisitionTimingChanged(int);
    void backgroundValueChanged( int, int);
    void updateRunInfo();
    void fitChargeDiagram(DiagramType);
    void resetDiagramsClicked();
    void dataUpdateChanged(int);
    void detailsClear();
    void detailsSelectAll();
    void detailsSelectNone();
    void detailsItemSelectionChanged();
    void processBatchesClicked();
    void processData();

private:
    QString processTextFile( QFile* runfile, QList<QListWidgetItem*>& items);
    bool processRawFile( QFile* runfile, QList<QListWidgetItem*>& items);
    void batchDataReceived( const DataList& list, const QDateTime&);
    RunInfo batchCountsReceived(const CountsList& list);
    void saveSettings(QSettings* set);
    void loadSettings(QSettings* set);
    void deviceError( FT_HANDLE, FT_STATUS);


    void createTreeWidgetItems();
    void createRootHistograms();
    RootCanvasDialog* createCanvasDialog(DiagramTreeWidgetItem*);
    template<class T> RootCanvasDialog* createCanvas(DiagramTreeWidgetItem*);

    Ui::MainWindow* ui;
    QTimer* timer; // ROOT GUI update timer
    QTimer* timer_data; // data update timer
    QTimer* timer_opcua; // OPC UA update timer

    FT_HANDLE channel_a;
    FT_HANDLE channel_b;
    QFile* filerun;
    QFile* filetxt;
    QFile* filedat;

    CommandThread* command_thread;
    AcquireThread* acquire_thread;
    ProcessThread* process_thread;
    ProcessFileThread* profile_thread;
    QProgressDialog* progress_dialog;
    QList<QTreeWidgetItem*> items;
    QString rundir;
    Diagrams diagrams;
    QSettings* settings;
    bool flag_background;
    bool flag_write_run;
//    bool flag_batch_state; // if "true" then process data, if "false" - do not process them
    SharedFitParameters params;
    RunInfo runinfo;
    SystemCurrentStatus sys_status;
};
