#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include "qcustomplot.h"

class PlotWidget : public QCustomPlot
{
    Q_OBJECT
      
    public:
        explicit PlotWidget(QString xlabel = "Samples", QString ylabel = "Magnitude", QWidget *parent = 0);
        ~PlotWidget();

        void setMargins(int minx, int maxx, int miny, int maxy);

    private:
        void mouseDoubleClickEvent(QMouseEvent *event) override;

    private slots:
};

#endif // PLOTWIDGET_H
