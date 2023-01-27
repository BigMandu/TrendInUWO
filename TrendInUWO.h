#pragma once

#include <QtWidgets/QMainWindow>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSignalMapper>
#include <QModelIndex>
#include <QStandardItem>
#include "ui_TrendInUWO.h"

class TrendInUWO : public QMainWindow
{
    Q_OBJECT

public:
    TrendInUWO(QWidget *parent = nullptr);
    ~TrendInUWO();


private:
    Ui::TrendInUWOClass ui;
    int NumOfColumns;
    QDateTime ExcuteProgramTime;
    QString FilePath;
    QJsonObject rootObj;
    
    quint64 PassedTime;
    void CalcDiffTime();
    

    QDateTime calcTrendOccurTime(QString LastOccurTime, int TrendTerm);


    void SetToolbarWidget();

    ////////////////////////
    /******* data *********/
    ////////////////////////

    QString LastSavingTime;
    QJsonArray TrendInfoArr;
    QJsonArray TradingAreaInfoArr;

    //setting ui (dynamically)
    void SettingGrid();

    //fileopen && Read data
    void OpenJSON();

    //savedata
    void WriteDataToJSON();



    QJsonDocument loadJson(QString filepath);
    void saveJson(QJsonDocument doc, QString filepath);

public slots:
    void clickbutton(class QTableView* Table);

    void ChangeValueofTable(QTableView* Table, int areaID);//QModelIndex startindex, QModelIndex endindex);
    void LastOccurTimeWasChanged(QModelIndex StartColumn, QModelIndex EndColumn);

signals:
    

    //void LastOccurTimeWasChanged(class QModelIndex StartColumn, QModelIndex EndColumn);
    //emit bool dataChanged(int StartColumn, int EndColumn);
};
