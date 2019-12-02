#ifdef ENABLE_VIEW

#include <QDesktopWidget>
#include "view.h"
#include "offcirc.h"
#include "bisector.h"
#include "mwvd.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, m_scene{this} {
    ui.setupUi(this);

    ui.graphicsView->setScene(&m_scene);
    ui.graphicsView->installEventFilter(&m_navigation);
    ui.graphicsView->viewport()->installEventFilter(&m_navigation);
    ui.graphicsView->setRenderHint(QPainter::Antialiasing);
    ui.lineEditTime->setText(QString::number(m_time));
    ui.lineEditStepSize->setText(QString::number(std::sqrt(m_stepSize)));
    
    QObject::connect(ui.actionIncrTime, &QAction::triggered, this,
                     [this]() {
                         m_time += m_stepSize;
                         ui.lineEditTime->setText(QString::number(std::sqrt(m_time)));
                         emit timeChanged(m_time);
                     });
    QObject::connect(ui.actionDecrTime, &QAction::triggered, this, 
                     [this]() {
                         m_time -= m_stepSize;
                         ui.lineEditTime->setText(QString::number(std::sqrt(m_time)));
                         emit timeChanged(m_time);
                     });
    QObject::connect(ui.actionToggleVorDiag, &QAction::triggered, this,
                     [this]() {
                         m_bShowVorDiag = !m_bShowVorDiag;
                         emit toggleShowVorDiag(m_bShowVorDiag);
                     });
    QObject::connect(ui.actionPrevEv, &QAction::triggered, this,
                     [this]() {
                         emit prevEv(m_time);
                     });
    QObject::connect(ui.actionNextEv, &QAction::triggered, this,
                     [this]() {
                         emit nextEv(m_time);
                     });

    resize(1280, 1024);
    centerWidget();
}

void MainWindow::addItem(QGraphicsItem *item) {
    if (typeid(*item) == typeid(BisecGraphicsItem)) {
        const auto bgi = static_cast<BisecGraphicsItem*> (item);
        QObject::connect(this, &MainWindow::timeChanged,
                         bgi, &BisecGraphicsItem::onTimeChanged);
    } else if (typeid(*item) == typeid(OffCircGraphicsItem)) {
        const auto ogi = static_cast<OffCircGraphicsItem*> (item);
        QObject::connect(this, &MainWindow::timeChanged,
                         ogi, &OffCircGraphicsItem::onTimeChanged);
    } else if (typeid(*item) == typeid(VorDiagGraphicsItem)) {
        const auto vdgi = static_cast<VorDiagGraphicsItem*> (item);
        QObject::connect(this, &MainWindow::toggleShowVorDiag,
                         vdgi, &VorDiagGraphicsItem::onToggle);
        QObject::connect(vdgi, &VorDiagGraphicsItem::timeChanged,
                         this, &MainWindow::onTimeChanged);
        QObject::connect(this, &MainWindow::nextEv,
                         vdgi, &VorDiagGraphicsItem::onNextEv);
        QObject::connect(this, &MainWindow::prevEv,
                         vdgi, &VorDiagGraphicsItem::onPrevEv);
    }
    
    m_scene.addItem(item);
}

void MainWindow::onTimeChanged(double t) {
    m_time = t;
    ui.lineEditTime->setText(QString::number(std::sqrt(m_time)));
    emit timeChanged(m_time);
}

void MainWindow::centerWidget() {
    auto rect = geometry();
    rect.moveCenter(QApplication::desktop()->availableGeometry().center());
    setGeometry(rect);
}

#include "moc_view.cpp"

#endif
