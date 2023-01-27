#include "TrendInUWO.h"
#include <qlabel.h>
#include <qtableview.h>
#include <QFileDialog>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPushButton>
#include <QSignalMapper>
#include <QStandardItemModel>
#include <QLCDNumber>
#include <QToolBar>

//https://forum.qt.io/topic/75432/difference-between-two-qdatetime-in-milliseconds/7
// 
//https://www.bogotobogo.com/Qt/Qt5_QTableView_QItemDelegate_ModelView_MVC.php

//https://stackoverflow.com/questions/4696285/update-cell-in-qtableview-with-new-value  <<좋음

TrendInUWO::TrendInUWO(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    NumOfColumns = 4;
    SetToolbarWidget();
    
    
    OpenJSON();
    SettingGrid();

}

void TrendInUWO::SetToolbarWidget()
{
    //setting clock
    QLCDNumber* Clock = new QLCDNumber();
    Clock->setStyleSheet("background: white; color: #000000");
    Clock->setSegmentStyle(QLCDNumber::Flat);
    Clock->setFixedSize(100, 30);
    Clock->setDigitCount(5); //strclock.toInt());
    Clock->display(QTime::currentTime().toString(QString("hh:mm")));


    //Add widget at Left side
    
    //Add Spacer
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui.toolBar->addWidget(spacer);

    //Add widget at Right side
    ui.toolBar->addWidget(Clock);
}


void TrendInUWO::clickbutton(QTableView* Table)
{
    if (Table)
    {
        if (Table->isHidden())
        {
            Table->setHidden(false);
        }
        else
        {
            Table->setHidden(true);
        }
    }
}


//get difftime between program start time and  last saving time.//프로그램 실행시간과 마지막 프로그램 종료시간 차이
void TrendInUWO::CalcDiffTime()
{    
    ExcuteProgramTime = QDateTime::currentDateTime();
    QDateTime LST = QDateTime::fromString(LastSavingTime,Qt::ISODate);
    
    //QDateTime LSTdate = QDateTime::fromString(LastSavingTime,Qt::TextDate);
    //QString LSTstr = LST.toString(Qt::TextDate);
    //QDateTime FixLST = QDateTime::fromString(LSTstr, Qt::TextDate);
    
    quint64 diff = qAbs(LST.date().daysTo(ExcuteProgramTime.date()));
    diff *= static_cast<quint64>(24); //to hour. //시간으로
    qDebug() << diff;
    PassedTime = diff;
    
    /*quint64 Timediff = qAbs(FixLST.secsTo(ExcuteProgramTime));
    qDebug() << Timediff;*/
}


//get difftime between last saved time and difftime(value from calcdifftime func) of specific trend type. //저장된 마지막 유행 발생 시간에서 CalcDiffTime 계산.
QDateTime TrendInUWO::calcTrendOccurTime(QString LastOccurTime, int TrendTerm)
{
    QDateTime returnTime = QDateTime::fromString(LastOccurTime, "hh:mm");

    //get PassedTerm count //종료시간에서 currentdate까지 흐른 시간을 trendTerm으로 나눠 지난Term의 개수를 구함. 
    qint64 PassedTerm = PassedTime/TrendTerm;
    
    //multiply PassedTerm by TrendTerm to recover LastOccurTime  //지난텀의 개수와 트렌드텀을 곱해 LastOccurTime을 (흐른 시간만큼)구해 초로 더해줌.
    returnTime = returnTime.addSecs(PassedTerm*TrendTerm*3600);
    //returnTime.addSecs(PassedTerm*TrendTerm*3600);

    return returnTime;
}


QJsonDocument TrendInUWO::loadJson(QString filepath)
{
    QJsonDocument Doc;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
		qDebug() << "TrendInUWO::loadJson // Failed to open file";
	    //return Doc;
    }
    
	QString data = file.readAll();
	file.close();
	Doc = QJsonDocument::fromJson(data.toUtf8());

    return Doc;
}

void TrendInUWO::saveJson(QJsonDocument doc, QString filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly))// | QIODevice::Text | QIODevice::Truncate))
    {
		qDebug() << "TrendInUWO::saveJson // Failed to open file";
		return;
    }
    file.write(doc.toJson());
    file.close();
}


//if change value of table, save to a Json file
void TrendInUWO::ChangeValueofTable(QTableView* Table, int areaID)//QStandardItem* item)
{
    qDebug() << "Value has changed";

	//get model
	QStandardItemModel* model = (QStandardItemModel*)Table->model();
    int areaIDforArr = areaID - 1;
	     
	QJsonDocument Doc = loadJson(FilePath);

	rootObj = Doc.object();
    QJsonObject InfoObj = rootObj["info"].toObject();
    TradingAreaInfoArr = rootObj["TradingAreaInfo"].toArray();

    //change value at 'info->LastSaveTime'
    InfoObj.insert("LastSaveTime",QJsonValue::fromVariant(QDateTime::currentDateTime().toString(Qt::ISODate)));
    rootObj.insert("info",InfoObj);


    QJsonObject TAIObj = TradingAreaInfoArr[areaIDforArr].toObject();   
    

    if (TAIObj["AreaId"].toInt() == areaID)
    {
        QJsonArray TrendstatusArr = TAIObj.value("TrendStatus").toArray();

        for (int i = 0; i < TrendstatusArr.size(); ++i)
        {
            QString timestr = model->data(model->index(i, 1)).toString();

            QJsonObject ArrelementObj =TrendstatusArr[i].toObject();
            ArrelementObj["LastTrendTime"] = timestr;
            TrendstatusArr[i] = ArrelementObj;
        }

        TAIObj["TrendStatus"] = TrendstatusArr;
        //TradingAreaInfoArr[areaIDforArr] = TAIObj;
        //TradingAreaInfoArr.insert(areaIDforArr,TAIObj);
        //rootObj["TradingAreaInfo"] = TAIObj; //it works, but delete all other TradingAreaInfo Array..
        //rootObj.insert("TradingAreaInfo",TAIObj);


        //QJsonValueRef를 이용한 방식 (안됨)
        {
         
		//TrendStatus의 ref값을 얻어온다.
		//QJsonValueRef AreaTradingArrRef = TAIObj.find("TrendStatus").value();

        //ref를 TrendStatus의 형식(Arr type)에 맞게 변환한다.
        //QJsonArray areaTrendStatusArr = AreaTradingArrRef.toArray();

        //test
        /*
        QJsonArray::iterator Arrayiter = areaTrendStatusArr.begin();
        QJsonValueRef elefirstvalref = Arrayiter[0];

        QJsonObject eleoneobj = elefirstvalref.toObject();
        eleoneobj["LastTrendTime"] = "11:11";

        elefirstvalref = eleoneobj;
        AreaTradingArrRef = areaTrendStatusArr;
        */

        /*int index = 0;
        for (auto iter = areaTrendStatusArr.begin(); iter < areaTrendStatusArr.end(); ++iter)
        {
			if (TrendInfoArr[index].toObject()["trendID"].toInt() == areaTrendStatusArr[index].toObject()["trendid"].toInt())
			{
				qDebug() << "can change value";
			}
            
            QString timestr = model->data(model->index(index, 1)).toString();
			QTime Time = QTime::fromString(timestr, "hh:mm");
			QString pretimefix = Time.toString("hh:mm");

            QJsonValueRef iterobj = *iter;
            QJsonObject ele = iterobj.toObject();
                        
            ele["LastTrendTime"] = timestr;
            AreaTradingArrRef = ele;

            ++index;
        }*/
        }

        //not work.        
        /*
        for (int trendcnt = 0; trendcnt < TrendInfoArr.size(); ++trendcnt)
        {
            if (TrendInfoArr[trendcnt].toObject()["trendID"].toInt() == areaTrendStatusArr[trendcnt].toObject()["trendid"].toInt())
            {
                qDebug() << "can change value";
            }

            QString timestr = model->data(model->index(trendcnt, 1)).toString();
            QTime Time = QTime::fromString(timestr,"hh:mm");
            QString pretimefix = Time.toString("hh:mm");


            QJsonObject TrArrayele = areaTrendStatusArr[trendcnt].toObject();

            TrArrayele.insert("LastTrendTime", pretimefix); //TrArrayele["LastTrendTime"] = timestr;

            //areaTrendStatusArr.insert(,TrArrayele);
            TAIObj.insert("TrendStatus",areaTrendStatusArr);
            
        }
        

        TradingAreaInfoArr.insert(TAIObj);
        
        rootObj.insert("TradingAreaInfo",TradingAreaInfoArr);        
        */
        
		Doc.setObject(rootObj);
		saveJson(Doc, FilePath);

        SettingGrid();
    }

    
    
}

void TrendInUWO::LastOccurTimeWasChanged(class QModelIndex StartColumn, QModelIndex EndColumn)
{
    qDebug() <<"Value has changed";
}


void TrendInUWO::OpenJSON()
{
    
    FilePath = QFileDialog::getOpenFileName(this,tr("Open File"),QDir::currentPath(),tr("JSON file (*.json)"));

   
    QJsonDocument Doc = loadJson(FilePath);
    rootObj = Doc.object();

    QJsonObject InfoObj = rootObj["info"].toObject();

    //마지막 JSON파일 저장 시간. (프로그램 종료시 저장되는 값)
    LastSavingTime = InfoObj["LastSaveTime"].toString();   
    

    //대유행 정보
    //대유행 종류마다 trendID, trendName, trendTerm를 할당.
    TrendInfoArr = InfoObj["TrendInfo"].toArray();

    //교역권 정보
    //교역권 마다 AreaId, AreaName, TrendStatus[trendid, LTT (LastTrendTime)] 를 할당해뒀음
    TradingAreaInfoArr = rootObj["TradingAreaInfo"].toArray();

    CalcDiffTime();
}



void TrendInUWO::WriteDataToJSON()
{
    
}

//귀찮으니까 얘를 걍 계속 refresh하자
//dynamically set gridlayout based on json file
void TrendInUWO::SettingGrid()
{
    //remove all children of gridlayout, before setting gridlayout.
    //qDeleteAll(ui.gridLayout->findChildren<QLayout*>());

    //iter as much as trading area //교역권 종류만큼 반복
    for (int i = 0; i < TradingAreaInfoArr.size(); ++i)
    {
        int row = i/NumOfColumns;
        int column = i%NumOfColumns;

        QVBoxLayout* VBox = new QVBoxLayout(this);
        QHBoxLayout* Headerborder = new QHBoxLayout(this);
        
        QLabel* label = new QLabel();
        QPushButton* togglebtn = new QPushButton(QString(">"),this);
        QTableView* table = new QTableView(this);
        int AreaID = i+1;

        Headerborder->addWidget(togglebtn,0,Qt::AlignLeft);
        Headerborder->addWidget(label,10,Qt::AlignLeft);

        VBox->addLayout(Headerborder);
        VBox->addWidget(table); 

        //Create model for tableview //모델 생성
        QStandardItemModel* model = new QStandardItemModel(TrendInfoArr.size(),6,this);
        
        //korean ver
        /*
        model->setHeaderData(0, Qt::Horizontal, QObject::tr("유행종류"));
        model->setHeaderData(1, Qt::Horizontal, QObject::tr("최근발생시간"));
        model->setHeaderData(2, Qt::Horizontal, QObject::tr("1타임발생예측"));
        model->setHeaderData(3, Qt::Horizontal, QObject::tr("2타임발생예측"));
        model->setHeaderData(4, Qt::Horizontal, QObject::tr("3타임발생예측"));
        model->setHeaderData(5, Qt::Horizontal, QObject::tr("4타임발생예측"));
        */
        //eng ver
		model->setHeaderData(0, Qt::Horizontal, QObject::tr("kind of trend"));
		model->setHeaderData(1, Qt::Horizontal, QObject::tr("Last occur time"));
		model->setHeaderData(2, Qt::Horizontal, QObject::tr("Predict next 1 time"));
		model->setHeaderData(3, Qt::Horizontal, QObject::tr("Predict next 2 time"));
		model->setHeaderData(4, Qt::Horizontal, QObject::tr("Predict next 3 time"));
		model->setHeaderData(5, Qt::Horizontal, QObject::tr("Predict next 4 time"));


        QJsonObject ele = TradingAreaInfoArr[i].toObject();
        label->setText(ele["AreaName"].toString());

        //iter as much as type of trend. //유행 종류의 개수 만큼 반복
        for (int trendcnt = 0; trendcnt < TrendInfoArr.size(); ++trendcnt)
        {
            /*** Info obj -> TrendInfo Array **/
            QJsonObject TrendInfo = TrendInfoArr[trendcnt].toObject();

            //trendName // 유행이름
            QString trendName = TrendInfo["trendName"].toString();
            //trendTerm //유행주기
            qint64 trendTerm = TrendInfo["trendTerm"].toInt();


            /**** TradingAreaInfo Array ************/
            QJsonArray trendstatusArr = ele["TrendStatus"].toArray();
            QJsonObject TrArrayele = trendstatusArr[trendcnt].toObject();
            
            //trend id //유행id   
            int AreaTrendID = TrArrayele["trendid"].toInt();

            //get the saved last occur time. //저장된 해당 유행의 최근 발생시간.
            QString savedLastOccurTime = TrArrayele["LastTrendTime"].toString();

            //calc last occur time.  //계산된 해당 유행의 최근 발생시간. (current date와 계산함)
            QDateTime CalculatedLastOccurTime = calcTrendOccurTime(savedLastOccurTime,trendTerm);
            
            //just change expression //표현식을 바꿔주기 위함.
            QString expressionTime = CalculatedLastOccurTime.toString("hh:mm");
            QTime Time = QTime::fromString(expressionTime,"hh:mm");


            model->setData(model->index(trendcnt,0),trendName);
            model->setData(model->index(trendcnt,1), Time);
            model->setData(model->index(trendcnt,2), Time.addSecs(trendTerm*3600));
            model->setData(model->index(trendcnt,3), Time.addSecs(trendTerm*2*3600));// Qt::DisplayRole);
            model->setData(model->index(trendcnt,4), Time.addSecs(trendTerm*3*3600));
            model->setData(model->index(trendcnt,5), Time.addSecs(trendTerm*4*3600));
        }

       
       /* when data change(through table view) , changevalueofTable func will called.
       */
       connect(model,&QStandardItemModel::dataChanged,[this,table,AreaID] {ChangeValueofTable(table,AreaID);});
       
        /*togglebtn delegate.bind button clicked eventand clickbutton func.
        * clickbutton func will called when clicked togglebtn.
        */            
        connect(togglebtn,&QPushButton::clicked, [this,table] {clickbutton(table); });

        
        table->setModel(model);
        ui.gridLayout->addLayout(VBox,row,column,Qt::AlignTop);
        //ui.gridLayout->addWidget(VBox,0,0);
        //ui.gridLayout->addWidget(VBox);

    }
}

TrendInUWO::~TrendInUWO()
{}
