/*  Ekos Scheduler Module
    Copyright (C) 2015 Daniel Leu <daniel_mihai.leu@cti.pub.ro>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 */


#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QMainWindow>
#include "ui_scheduler.h"
#include <QtDBus/QtDBus>
#include "scheduler.h"
#include "kstars.h"
#include "schedulerjob.h"
#include "skyobjects/ksmoon.h"
#include "QProgressIndicator.h"
#include "ui_scheduler.h"

namespace Ekos {


/**
 * @brief The Scheduler class Will orchestrate the main functionality of the scheduler
 * @author Daniel Leu
 */
class Scheduler : public QWidget, public Ui::Scheduler
{
    Q_OBJECT

public:
    enum StateChoice{IDLE, STARTING_EKOS, EKOS_STARTED, CONNECTING, CONNECTED,  READY, FINISHED, SHUTDOWN,
                     PARK_TELESCOPE, WARM_CCD, CLOSE_DOME, ABORTED};
     Scheduler();
    ~Scheduler();
     /**
      * @brief checkWeather checks the weather conditions (only cloud status at the moment). In development
      * @return
      */
     int checkWeather();
     /**
      * @brief startEkos DBus call for starting ekos
      */
     void startEkos();
     /**
      * @brief updateJobInfo Updates the state cell of the current job
      * @param o the current job that is being evaluated
      */
     void updateJobInfo(Schedulerjob *o);
     void appendLogText(const QString &);
     QString getLogText() { return logText.join("\n"); }

     /**
      * @brief startSlew DBus call for initiating slew
      */
     void startSlew();
     /**
      * @brief startFocusing DBus call for feeding ekos the specified settings and initiating focus operation
      */
     void startFocusing();
     /**
      * @brief startAstrometry initiation of the capture and solve operation. We change the job state
      * after solver is started
      */
     void startAstrometry();
     /**
      * @brief startGuiding After ekos is fed the calibration options, we start the guiging process
      */
     void startGuiding();
     /**
      * @brief startCapture The current job file name is solved to an url which is fed to ekos. We then start the capture process
      */
     void startCapture();
     /**
      * @brief getNextAction Checking for the next appropiate action regarding the current state of the scheduler  and execute it
      */
     void getNextAction();
     /**
      * @brief connectDevices After ekos is started, we connect devices
      */
     void connectDevices();
     void parkTelescope();
     void warmCCD();
     void closeDome();
     /**
      * @brief stopindi Stoping the indi services
      */
     void stopindi();
     /**
      * @brief stopGuiding After guiding is done we need to stop the process
      */
     void stopGuiding();
     void clearLog();
     /**
      * @brief setGOTOMode set the GOTO mode for the solver
      * @param mode 1 for SlewToTarget, 2 for Nothing
      */
     void setGOTOMode(int mode);
     /**
      * @brief startSolving start the solving process for the FITS job
      */
     void startSolving();
     /**
      * @brief getResults After solver is completed, we get the object coordinates and construct the schedulerJob object
      */
     void getResults();
     /**
      * @brief processFITS I use this as an intermediary to start the solving process of the FITS objects that are currenty in the list
      * @param value
      */
     void processFITS(Schedulerjob *value);
     /**
      * @brief getNextFITSAction Similar process to the one used on regular objects. This one is used in case of FITS selection method
      */
     void getNextFITSAction();
     /**
      * @brief terminateFITSJob After a FITS object is solved, we check if another FITS object exists. If not, we end the solving process.
      * @param value the current FITS job
      */
     void terminateFITSJob(Schedulerjob *value);

     Schedulerjob *getCurrentjob() const;
     void setCurrentjob(Schedulerjob *value);
     /**
      * @brief terminateJob After a job is completed, we check if we have another one pending. If not, we start the shutdown sequence
      * @param value the current job
      */
     void terminateJob(Schedulerjob *value);
     /**
      * @brief executeJob After the best job is selected, we call this in order to start the process that will execute the job.
      * checkJobStatus slot will be connected in order to fgiure the exact state of the current job each second
      * @param value
      */
     void executeJob(Schedulerjob *value);

     StateChoice getState() const;
     void setState(const StateChoice &value);

public slots:
     /**
      * @brief selectSlot Normal selection method. Toggles the find dialog.
      */
     void selectSlot();
     /**
      * @brief addToTableSlot The schedulerJob object is being constructed and added to the table
      */
     void addToTableSlot();
     /**
      * @brief removeTableSlot Removing the object from the table and from the list
      */
     void removeTableSlot();
     /**
      * @brief setSequenceSlot File select functionality for the sequence file
      */
     void setSequenceSlot();
     /**
      * @brief startSlot We connect evaluateJobs() and start the scheduler for the given object list.
      * we make sure that there are objects in the list.
      */
     void startSlot();
     /**
      * @brief saveSlot saves the current configuration of the scheduler sequence in a .xml file
      */
     void saveSlot();
     /**
      * @brief processObjectInfo parses the scheduler scml file and creates the corresponding Schedulerjob object
      * @param root
      * @param ob this will be created with the information parsed from the document
      */
     void processObjectInfo(XMLEle *root, Schedulerjob *ob);
     /**
      * @brief loadSlot will take the information currently present in the scheduler queue and save it in a xml file for further use
      */
     void loadSlot();
     /**
      * @brief evaluateJobs evaluates the current state of each objects and gives each one a score based on the constraints.
      * Given that score, the scheduler will decide which is the best job that needs to be executed.
      */
     void evaluateJobs();
     /**
      * @brief checkJobStatus This will run each second until it is diconnected. Thus, it will decide the state of the
      * scheduler at the present moment making sure all the pending operations are resolved.
      */
     void checkJobStatus();
     /**
      * @brief selectFITSSlot FITS selection method. Toggles the file dialog.
      */
     void selectFITSSlot();
     /**
      * @brief solveFITSSlot Checks for any pending FITS objects that need to be solved
      */
     void solveFITSSlot();
     /**
      * @brief solveFITSAction if a FITS job is detected, processFITS() is called and the solving process is started
      */
     void solveFITSAction();
     /**
      * @brief checkFITSStatus Checks the scheduler state each second, making sure all the operations are completed succesfully
      */
     void checkFITSStatus();

signals:
        void newLog();

private:
    QDBusConnection bus = QDBusConnection::sessionBus();
    //DBus interfaces
    QDBusInterface *focusinterface;
    QDBusInterface *ekosinterface;
    QDBusInterface *captureinterface;
    QDBusInterface *mountinterface;
    QDBusInterface *aligninterface;
    QDBusInterface *guideinterface;
    //Scheduler current state
    StateChoice state;
    QProgressIndicator *pi;
    Ekos::Scheduler *ui;
    KSMoon Moon;
    SkyPoint *moon;
    int tableCountRow;
    int tableCountCol;
    //This will determine when all the objects are processed
    int iterations ;
    int isFITSSelected;
    //The list of pending objects
    QVector<Schedulerjob> objects;
    SkyObject *o;
    QStringList logText;
    //The current job that is evaluated
    Schedulerjob *currentjob;
    //The current FITS job that is in the process of solving
    Schedulerjob *currentFITSjob;
    bool isStarted;
};
}

#endif // SCHEDULER_H