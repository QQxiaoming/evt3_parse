#include "mainwindow.h"
#include "config.h"

QString GIT_TAG =
#include "git_tag.inc"
;

MainWindow::MainWindow(QWidget *parent)
    : QGoodWindow{parent} {
    m_good_central_widget = new QGoodCentralWidget(this);
    m_central_widget = new QWidget(this);
    layout = new QVBoxLayout(m_central_widget);
    splitter = new QSplitter(Qt::Vertical,m_central_widget);
    widet_l = new EventSensorWidget(SEVER_PORT+0,100,100,m_central_widget);
    widet_r = new EventSensorWidget(SEVER_PORT+1,100,100,m_central_widget);
    splitter->setHandleWidth(1);
    splitter->addWidget(widet_l);
    splitter->addWidget(widet_r);
    splitter->setCollapsible(0,true);
    splitter->setCollapsible(1,true);
    splitter->setSizes(QList<int>() << 1 << 1);
    layout->addWidget(splitter);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    m_central_widget->setLayout(layout);
    m_central_widget->setContentsMargins(0, 0, 0, 0);
    m_central_widget->setWindowTitle("evt3_parse");
    this->resize(IMG_WIDTH/2,IMG_HEIGHT);
    m_menu_bar = new QMenuBar(this);
    QMenu * menuFile = m_menu_bar->addMenu("File");
    QAction * rec = menuFile->addAction("rec");
    rec->setCheckable(true);
    rec->setChecked(widet_l->getRec());
    QObject::connect(rec, &QAction::triggered, [=](){
        widet_l->setRec(rec->isChecked());
        widet_r->setRec(rec->isChecked());
    });
    QMenu * menuView = m_menu_bar->addMenu("View");
    QAction * actionViewL = menuView->addAction("Left");
    actionViewL->setCheckable(true);
    actionViewL->setChecked(true);
    QAction * actionViewR = menuView->addAction("Right");
    actionViewR->setCheckable(true);
    actionViewR->setChecked(true);
    QObject::connect(actionViewL, &QAction::triggered, [=](){
        widet_l->setVisible(actionViewL->isChecked());
        if(actionViewL->isChecked() && actionViewR->isChecked()) {
            this->resize(IMG_WIDTH/2,IMG_HEIGHT);
        } else {
            this->resize(IMG_WIDTH/2,IMG_HEIGHT/2);
        }
    });
    QObject::connect(actionViewR, &QAction::triggered, [=](){
        widet_r->setVisible(actionViewR->isChecked());
        if(actionViewL->isChecked() && actionViewR->isChecked()) {
            this->resize(IMG_WIDTH/2,IMG_HEIGHT);
        } else {
            this->resize(IMG_WIDTH/2,IMG_HEIGHT/2);
        }
    });
    QMenu * menuOpt = m_menu_bar->addMenu("Option");
    QAction * actionDiff = menuOpt->addAction("setDiff");
    QObject::connect(actionDiff, &QAction::triggered, [=](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(this, "setDiff", "setDiff", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l->setDiff(val|0x01A15000,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
            widet_r->setDiff(val|0x01A15000,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
        }
    });
    QAction * actionDiffOn = menuOpt->addAction("setDiffOn");
    QObject::connect(actionDiffOn, &QAction::triggered, [=](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(this, "setDiffOn", "setDiffOn", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l->setDiff(0xffffffff,val|0x01A16300,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
            widet_r->setDiff(0xffffffff,val|0x01A16300,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
        }
    });
    QAction * actionDiffOff = menuOpt->addAction("setDiffOff");
    QObject::connect(actionDiffOff, &QAction::triggered, [=](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(this, "setDiffOff", "setDiffOff", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l->setDiff(0xffffffff,0xffffffff,val|0x01A13700,0xffffffff,0xffffffff,0xffffffff);
            widet_r->setDiff(0xffffffff,0xffffffff,val|0x01A13700,0xffffffff,0xffffffff,0xffffffff);
        }
    });
    QAction * actionBiasFo = menuOpt->addAction("setBiasFo");
    QObject::connect(actionBiasFo, &QAction::triggered, [=](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(this, "setBiasFo", "setBiasFo", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l->setDiff(0xffffffff,0xffffffff,0xffffffff,val|0x03A1E800,0xffffffff,0xffffffff);
            widet_r->setDiff(0xffffffff,0xffffffff,0xffffffff,val|0x03A1E800,0xffffffff,0xffffffff);
        }
    });
    QAction * actionBiasHpf = menuOpt->addAction("setBiasHpf");
    QObject::connect(actionBiasHpf, &QAction::triggered, [=](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(this, "setBiasHpf", "setBiasHpf", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l->setDiff(0xffffffff,0xffffffff,0xffffffff,0xffffffff,val|0x03A1FF00,0xffffffff);
            widet_r->setDiff(0xffffffff,0xffffffff,0xffffffff,0xffffffff,val|0x03A1FF00,0xffffffff);
        }
    });
    QAction * actionBiasRefr = menuOpt->addAction("setBiasRefr");
    QObject::connect(actionBiasRefr, &QAction::triggered, [=](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(this, "setBiasRefr", "setBiasRefr", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l->setDiff(0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,val|0x03829600);
            widet_r->setDiff(0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,val|0x03829600);
        }
    });
    menuOpt->addSeparator();
    QAction * actionTrigTime = menuOpt->addAction("setTrigTime");
    QObject::connect(actionTrigTime, &QAction::triggered, [=](){
        static int val = 100;
        bool ok;
        val = QInputDialog::getInt(this, "setTrigTime", "setTrigTime", val, 0, 65536, 1, &ok);
        if(ok) {
            widet_l->setTime(val,-1);
            widet_r->setTime(val,-1);
        }
    });
    QAction * actionLutTime = menuOpt->addAction("setLutTime");
    QObject::connect(actionLutTime, &QAction::triggered, [=](){
        static int val = 100;
        bool ok;
        val = QInputDialog::getInt(this, "setLutTime", "setLutTime", val, 0, 65536, 1, &ok);
        if(ok) {
            widet_l->setTime(-1,val);
            widet_r->setTime(-1,val);
        }
    });
    QMenu * menuAbout = m_menu_bar->addMenu("About");
    QAction * actionAbout = menuAbout->addAction("About");
    QObject::connect(actionAbout, &QAction::triggered, [=](){
        QMessageBox::about(this, "About", QString("evt3_parse\n\nVersion: %0").arg(GIT_TAG));
    });
    QAction * actionAboutQt = menuAbout->addAction("About Qt");
    QObject::connect(actionAboutQt, &QAction::triggered, [=](){
        QMessageBox::aboutQt(this, "About Qt");
    });

#ifdef Q_OS_MAC
    //macOS uses global menu bar
    if(QApplication::testAttribute(Qt::AA_DontUseNativeMenuBar)) {
#else
    if(true) {
#endif

        //Set font of menu bar
        QFont font = m_menu_bar->font();
    #ifdef Q_OS_WIN
        font.setFamily("Segoe UI");
    #else
        font.setFamily(qApp->font().family());
    #endif
        m_menu_bar->setFont(font);

        QTimer::singleShot(0, this, [=]{
            const int title_bar_height = m_good_central_widget->titleBarHeight();
            m_menu_bar->setStyleSheet(QString("QMenuBar {height: %0px;}").arg(title_bar_height));
        });

        connect(m_good_central_widget,&QGoodCentralWidget::windowActiveChanged,this, [&](bool active){
            m_menu_bar->setEnabled(active);
        #ifdef Q_OS_MACOS
            fixWhenShowQuardCRTTabPreviewIssue();
        #endif
        });

        m_good_central_widget->setLeftTitleBarWidget(m_menu_bar);
        setNativeCaptionButtonsVisibleOnMac(false);
    }

    connect(qGoodStateHolder, &QGoodStateHolder::currentThemeChanged, this, [](){
        if (qGoodStateHolder->isCurrentThemeDark())
            QGoodWindow::setAppDarkTheme();
        else
            QGoodWindow::setAppLightTheme();
    });
    connect(this, &QGoodWindow::systemThemeChanged, this, [=]{
        qGoodStateHolder->setCurrentThemeDark(QGoodWindow::isSystemThemeDark());
    });
    qGoodStateHolder->setCurrentThemeDark(true);

    m_good_central_widget->setCentralWidget(m_central_widget);
    setCentralWidget(m_good_central_widget);

    m_good_central_widget->setTitleAlignment(Qt::AlignCenter);
}
