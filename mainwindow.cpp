#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <cmath>

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <QVector2D>

void createNewPointsAndLinesForData(const QPolygonF& inputPoints,
    const int dataCount,
    QPolygonF& outputPoints,
    QVector<QLineF>& outputLines);

double linesLengthBetween2Points(const QPolygonF& pointsSet, const int p1, const int p2);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    _data.resize(64);
    std::generate(_data.begin(), _data.end(), [] { return (std::rand() % 256); });

    _colorMap = ColorMapPresets::controlPointsToLinearColorMap(ColorMapPresets::Jet());


}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter txtPainter(this);
    txtPainter.drawText(QPoint(20, 30), "Test");
    //painter.setPen(QPen(Qt::black, 12, Qt::DashDotLine, Qt::RoundCap));
    //painter.drawLine(0, 0, 200, 200);

    /*QColor c;
    c.setRgba(mInternals->ColorMap.rgb(Min, Max, value));
    pmPainter.setPen(c);
    pmPainter.drawLine(x, paintRect.top(), x, paintRect.bottom());*/

    /*setAttribute(Qt::WA_OpaquePaintEvent);
    QPainter painter(this);
    QPen linepen(Qt::red);
    linepen.setCapStyle(Qt::RoundCap);
    linepen.setWidth(30);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(linepen);
    painter.drawPoint(point);*/
}

double linesLengthBetween2Points(const QPolygonF& pointsSet, const int p1, const int p2)
{
    const int pointsCount = pointsSet.size();
    double length = 0.;

    if (p1 < 0 || p1 >= pointsCount || p2 < 0 || p2 >= pointsCount || p1 == p2) {
        return length;
    }

    QPointF pointA;
    QPointF pointB;

    if (p1 < p2) {
        for (int i = p1; i <= p2 - 1; ++i) {
            pointA = pointsSet[i];
            pointB = pointsSet[i + 1];

            length += QLineF(pointA, pointB).length();
        }
    }
    else if (p1 > p2) {
        // or iterate from p1 and when you reach the end iterate from 0 to p2
        length += QLineF(pointA, pointB).length();
    }

    return length;
}

void createNewPointsAndLinesForData(const QPolygonF& inputPoints, const int dataCount, 
    QPolygonF& outputPoints, QVector<QLineF>& outputLines)
{
    outputPoints.clear();
    outputLines.clear();

    if (inputPoints.size() < 2
        /*|| dataCount < 2 || dataCount < (inputPoints.size() - 1)*/)
    {
        return;
    }

    if (dataCount == inputPoints.size() - 1) // + same treatment when dataCount < 2 || dataCount < (inputPoints.size() - 1) ?
    {
        outputPoints = inputPoints;
        goto createLines;
    }

    int inputPointsIndexA = 0;
    int inputPointsIndexB = 1;
    QPointF pointA(inputPoints[inputPointsIndexA]);
    QPointF pointB(inputPoints[inputPointsIndexB]);

    outputPoints.push_back(pointA);

    const int lastInputPointsIndex = inputPoints.size() - 1;
    const double inputPointsLinesLength = linesLengthBetween2Points(inputPoints, 0, inputPoints.size() - 1);
    const double step = inputPointsLinesLength / dataCount;

    double lengthAB = QLineF(pointA, pointB).length();
    double cosine = (pointB.x() - pointA.x()) / lengthAB;
    double sine = (pointB.y() - pointA.y()) / lengthAB;
    double dotProduct = 0.;

    QPointF point1(pointA);
    QPointF point2;

    for (int i = 0; i < dataCount; ++i)
    {
        point2.setX(point1.x() + cosine * step);
        point2.setY(point1.y() + sine * step);

        QVector2D AB(pointB.x() - pointA.x(), pointB.y() - pointA.y());
        QVector2D BP2(point2.x() - pointB.x(), point2.y() - pointB.y());
        float dotProduct = QVector2D::dotProduct(BP2, AB); // QVector2D::dotProduct outputs a float
        if (dotProduct > 0)
        {
            while (dotProduct > 0 && inputPointsIndexB <= lastInputPointsIndex)
            {
                double overrun = QLineF(point2, pointB).length();

                if (inputPointsIndexB < lastInputPointsIndex)
                {
                    ++inputPointsIndexA; // A becomes B
                    ++inputPointsIndexB; // B becomes its successor
                    pointA = inputPoints[inputPointsIndexA];
                    pointB = inputPoints[inputPointsIndexB];
                }
                else
                {
                    pointA = inputPoints[lastInputPointsIndex];
                    pointB = point2;

                    ++inputPointsIndexB; // increment inputPointsIndexB so we can exit the loop
                }                

                lengthAB = QVector2D::dotProduct(QVector2D(pointA), QVector2D(pointB));
                if (lengthAB == 0)
                {
                    continue;
                }

                cosine = (pointB.x() - pointA.x()) / lengthAB;
                sine = (pointB.y() - pointA.y()) / lengthAB;
                point2.setX(pointA.x() + cosine * overrun);
                point2.setY(pointA.y() + sine * overrun);

                const QVector2D AB{ QPointF { pointB.x() - pointA.x(), pointB.y() - pointA.y() } };
                const QVector2D BP2{ QPointF { point2.x() - pointB.x(), point2.y() - pointB.y() } };
                dotProduct = QVector2D::dotProduct(BP2, AB);
            }
            if (inputPointsIndexB >= lastInputPointsIndex)
            {
                outputPoints.push_back(inputPoints.back());

                break; // no more input points, break the for loop
            }
            else
            {
                outputPoints.push_back(point2);
            }
        }
        else
        {
            outputPoints.push_back(point2);
        }

        point1 = point2;
    }

createLines:
    for (int i = 0; i < outputPoints.size() - 1; i++)
    {
        outputLines.push_back(QLineF(outputPoints[i], outputPoints[i + 1]));
    }
}
