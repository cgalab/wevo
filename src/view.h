#ifndef VIEW_H
#define VIEW_H

#ifdef ENABLE_VIEW

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <CGAL/Qt/GraphicsViewNavigation.h>
#include "defs.h"
#include "ui_mwvd.h"

class VorDiagGraphicsItem;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void addItem(QGraphicsItem *item);
    void showEvent(QShowEvent *);

public slots:
    void onTimeChanged(double t);
    
signals:
    void timeChanged(double t);
    void toggleShowVorDiag(bool bIsVisible);
    void toggleShowWavefront(bool bIsVisible);
    void toggleShowSites(bool bIsVisible);
    void prevEv(double t);
    void nextEv(double t);
    void hideEvPnt();

private:
    void centerWidget();

public:
    Ui::MainWindow ui;
    CGAL::Qt::GraphicsViewNavigation m_navigation;
    QGraphicsScene *m_scene;
    double m_time{0.};
    double m_baseStepSize{0.};
    double m_stepSize{0.};
    bool m_bShowVorDiag{true};
    bool m_bShowWavefront{true};
    bool m_bShowSites{true};
    bool m_bFirstShowEvent{true};
    int m_stepSizePercentage{100};
    VorDiagGraphicsItem *m_vdgi;
};

#endif

#endif /* VIEW_H */

