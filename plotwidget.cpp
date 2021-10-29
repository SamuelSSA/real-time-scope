#include "plotwidget.h"

PlotWidget::PlotWidget(QString xlabel, QString ylabel, QWidget *parent) : QCustomPlot(parent)
{
    // include this section to fully disable antialiasing for higher performance:
    this->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    this->xAxis->setTickLabelFont(font);
    this->yAxis->setTickLabelFont(font);
    this->legend->setFont(font);

    // plot setup
    this->addGraph();
    this->xAxis->setLabel(xlabel);
    this->yAxis->setLabel(ylabel);
    this->xAxis->setRange(-1.5, 1.5);
    this->yAxis->setRange(-1.5, 1.5);
    this->xAxis->grid()->setSubGridVisible(true);
    this->yAxis->grid()->setSubGridVisible(true);
    this->graph(0)->setPen(QPen(QColor(0, 127, 0)));
    this->graph(0)->setBrush(QBrush(QColor(0, 32, 0, 20)));
    this->xAxis2->setVisible(true);
    this->xAxis2->setTickLabels(false);
    this->yAxis2->setVisible(true);
    this->yAxis2->setTickLabels(false);
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
            QCP::iSelectPlottables);
    
    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(this->xAxis, SIGNAL(rangeChanged(QCPRange)),
            this->xAxis2, SLOT(setRange(QCPRange)));
    connect(this->yAxis, SIGNAL(rangeChanged(QCPRange)),
            this->yAxis2, SLOT(setRange(QCPRange)));
}

PlotWidget::~PlotWidget()
{
}

void PlotWidget::setMargins(int minx, int maxx, int miny, int maxy)
{
    this->xAxis->setRange(minx, maxx);
    this->yAxis->setRange(miny, maxy);
}

void PlotWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        this->graph(0)->rescaleAxes();
        this->replot();
    }
}
