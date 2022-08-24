#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <algorithm>
#include <cmath>
#include <numeric>

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

    _data.resize(32);
    //std::srand(time(NULL));
    //std::generate(_data.begin(), _data.end(), [] { return (std::rand() % 256); });
    //_minData = *std::min_element(_data.begin(), _data.end());
    //_maxData = *std::max_element(_data.begin(), _data.end());
    
    std::iota(_data.begin(), _data.end(), 1);
    _minData = 1;
    _maxData = 32;

    _colorMap = ColorMapPresets::controlPointsToLinearColorMap(ColorMapPresets::Jet());

    _points.push_back(QPointF(20, 30));
    _points.push_back(QPointF(45, 40));
    _points.push_back(QPointF(100, 100));
    _points.push_back(QPointF(200, 150));
    _points.push_back(QPointF(150, 300));
    _points.push_back(QPointF(50, 350));

    createNewPointsAndLinesForData(_points, _data.size(), _extendedPoints, _lines);
    Q_ASSERT(_lines.size() <= _data.size()); // createNewPointsAndLinesForData has bugs
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    //setAttribute(Qt::WA_OpaquePaintEvent);

    QPen redPen(Qt::red);
    redPen.setCapStyle(Qt::RoundCap);
    redPen.setWidth(5);

    QPen greenPen(Qt::darkGreen);
    greenPen.setCapStyle(Qt::RoundCap);
    greenPen.setWidth(5);

    QPen bluePen(Qt::blue);
    bluePen.setCapStyle(Qt::RoundCap);
    bluePen.setWidth(5);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawText(QPoint(15, 20), "Original points set");

    painter.setPen(redPen);
    for (const auto& pt : _points)
    {
        painter.drawPoint(pt);
    }

    painter.setPen(greenPen);
    painter.translate(250, 0);
    painter.drawText(QPoint(15, 20), "Extended points set");
    for (const auto& pt : _extendedPoints)
    {
        painter.drawPoint(pt);
    }

    painter.setPen(bluePen);
    painter.translate(250, 0);
    painter.drawText(QPoint(15, 20), "Coloring lines demo");
    
    for (int lineIdx = 0; lineIdx < _lines.size(); ++lineIdx)
    {
        QColor dataColor;
        dataColor.setRgba(_colorMap.rgb(_minData, _maxData, _data[lineIdx]));

        QPen dataPen;
        dataPen.setColor(dataColor);
        dataPen.setCapStyle(Qt::RoundCap);
        //dataPen.setStyle(Qt::SolidLine);
        dataPen.setWidth(5);
        painter.setPen(dataPen);
        painter.drawLine(_lines[lineIdx]);
    }
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

// I think it's better to rewrite this function using the a finite-state machine.
// NB: inputPoints mustn't contain identical consecutive points
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

    // if pointA and pointB are the same, we will have funny values in cosine in sine
    // even better : don't feed this function an inputPoints which can have identical consecutive points
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
                    pointB = point2; // Hummm....

                    ++inputPointsIndexB; // increment inputPointsIndexB so we can exit the loop
                }

                lengthAB = QLineF(pointA, pointB).length();
                
                // Keep it commented as long as inputPoints doesn't have identical consecutive points
                /*if (lengthAB == 0.)
                {
                    continue;
                }*/

                cosine = (pointB.x() - pointA.x()) / lengthAB;
                sine = (pointB.y() - pointA.y()) / lengthAB;
                point2.setX(pointA.x() + cosine * overrun);
                point2.setY(pointA.y() + sine * overrun);

                const QVector2D AB{ QPointF { pointB.x() - pointA.x(), pointB.y() - pointA.y() } };
                const QVector2D BP2{ QPointF { point2.x() - pointB.x(), point2.y() - pointB.y() } };
                dotProduct = QVector2D::dotProduct(BP2, AB);
            }
            if (inputPointsIndexB > lastInputPointsIndex)
            {
                // not a good solution :
                outputPoints.push_back(inputPoints.back());

                // there will be missing points in some cases... why ?

                break; // no more input points, break the for loop
            }
            
            outputPoints.push_back(point2);

            // what if dotProduct is equal to zero ? we need to update 'cosine' and 'sine' values
            if (dotProduct == 0. && inputPointsIndexB < lastInputPointsIndex)
            {
                // This is correct as long as inputPoints doesn't have identical consecutive points
                ++inputPointsIndexA;
                ++inputPointsIndexB;
                pointA = inputPoints[inputPointsIndexA];
                pointB = inputPoints[inputPointsIndexB];
                lengthAB = QLineF(pointA, pointB).length();
                cosine = (pointB.x() - pointA.x()) / lengthAB;
                sine = (pointB.y() - pointA.y()) / lengthAB;
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
