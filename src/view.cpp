#ifdef ENABLE_VIEW

#include <QDesktopWidget>
#include "view.h"
#include "offcirc.h"
#include "bisector.h"
#include "mwvd.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow{parent} {
    ui.setupUi(this);
    m_scene = new QGraphicsScene{this};
    
    ui.graphicsView->setScene(m_scene);
    ui.graphicsView->installEventFilter(&m_navigation);
    ui.graphicsView->viewport()->installEventFilter(&m_navigation);
    ui.graphicsView->setRenderHint(QPainter::Antialiasing);
    ui.lineEditTime->setText(QString::number(m_time));
    
    QObject::connect(ui.actionIncrTime, &QAction::triggered, this,
                     [this]() {
                         m_time += m_stepSize;
                         double displayTime = (m_time - 1.e-10) < 0. ? 0. : m_time;
                         ui.lineEditTime->setText(QString::number(displayTime));
                         emit timeChanged(m_time);
                         emit hideEvPnt();
                     });
    QObject::connect(ui.actionDecrTime, &QAction::triggered, this,
                     [this]() {
                         if (m_time - m_stepSize > 0.) {
                             m_time -= m_stepSize;
                             double displayTime = (m_time - 1.e-10) < 0. ? 0. : m_time;
                             ui.lineEditTime->setText(QString::number(displayTime));
                             emit timeChanged(m_time);
                             emit hideEvPnt();
                         }
                     });
    QObject::connect(ui.actionToggleVorDiag, &QAction::triggered, this,
                     [this]() {
                         m_bShowVorDiag = !m_bShowVorDiag;
                         emit toggleShowVorDiag(m_bShowVorDiag);
                     });
    QObject::connect(ui.actionToggleWavefront, &QAction::triggered, this,
                     [this]() {
                         m_bShowWavefront = !m_bShowWavefront;
                         emit toggleShowWavefront(m_bShowWavefront);
                     });
    QObject::connect(ui.actionToggleSites, &QAction::triggered, this,
                     [this]() {
                         m_bShowSites = !m_bShowSites;
                         emit toggleShowSites(m_bShowSites);
                     });
    QObject::connect(ui.actionPrevEv, &QAction::triggered, this,
                     [this]() {
                         emit prevEv(std::pow(m_time, 2.));
                     });
    QObject::connect(ui.actionNextEv, &QAction::triggered, this,
                     [this]() {
                         emit nextEv(std::pow(m_time, 2.));
                     });
    QObject::connect(ui.stepSizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
                     [this](int value) {
                         m_stepSize = m_baseStepSize * (static_cast<double>(value) / 100.);
                     });
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
        QObject::connect(this, &MainWindow::toggleShowWavefront,
                         ogi, &OffCircGraphicsItem::onToggle);
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
        QObject::connect(this, &MainWindow::hideEvPnt,
                         vdgi, &VorDiagGraphicsItem::onHideEvPnt);
        m_vdgi = vdgi;
    } else if (typeid(*item) == typeid(SiteGraphicsItem)) {
        const auto sgi = static_cast<SiteGraphicsItem*> (item);
        QObject::connect(this, &MainWindow::toggleShowSites,
                         sgi, &SiteGraphicsItem::onToggle);
    }
    
    m_scene->addItem(item);
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

void MainWindow::showEvent(QShowEvent *) {
    if (m_bFirstShowEvent) {
        QRectF br;
        const auto sites = m_vdgi->vorDiag().sites();
        double maxWeight = 0.;
        for (const auto &site : sites) {
            double x = CGAL::to_double(site->center().x()),
                    y = CGAL::to_double(site->center().y()),
                    w = CGAL::to_double(site->weight());
            const auto rect = QRectF{x - 5., y - 5., 10., 10.};
            br |= rect;
            
            if (w > maxWeight) {
                maxWeight = w;
            }
        }
        
        qreal w = br.width();
        qreal h = br.height();

        double marginsFactor = 0.1;
        auto withMargins = br.marginsAdded(QMarginsF(w * marginsFactor, h * marginsFactor, 
                                                      w * marginsFactor, h * marginsFactor));
        ui.graphicsView->setSceneRect(withMargins);
        ui.graphicsView->fitInView(withMargins, Qt::KeepAspectRatio);
        m_bFirstShowEvent = false;
        
        const auto evs = m_vdgi->vorDiag().events();
        double midTime = CGAL::to_double(evs.at(evs.size() / 2)->sqrdTime());
        m_baseStepSize = midTime / 100.;
        m_stepSize = m_baseStepSize;
    }
}

#include "moc_view.cpp"

#endif
