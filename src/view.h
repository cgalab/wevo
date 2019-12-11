#ifndef VIEW_H
#define VIEW_H

#ifdef ENABLE_VIEW

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <CGAL/Qt/GraphicsViewNavigation.h>
#include "defs.h"
#include "ui_mwvd.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void addItem(QGraphicsItem *item);

public slots:
    void onTimeChanged(double t);
    
signals:
    void timeChanged(double t);
    void toggleShowVorDiag(bool bIsVisible);
    void prevEv(double t);
    void nextEv(double t);

private:
    void centerWidget();
    
    Ui::MainWindow ui;
    CGAL::Qt::GraphicsViewNavigation m_navigation;
    QGraphicsScene m_scene;
    double m_time{0.};
    double m_stepSize{.001};
    bool m_bShowVorDiag{true};
};

#endif

#endif /* VIEW_H */

