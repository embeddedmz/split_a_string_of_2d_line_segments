#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLineF>
#include <QMainWindow>
#include <QPolygonF>

#include "colorMapPresets.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void paintEvent(QPaintEvent* event) override;


private:
    Ui::MainWindow* _ui;

    QVector<double> _data;
    double _minData;
    double _maxData;

    QPolygonF _points;
    QPolygonF _extendedPoints;
    QVector<QLineF> _lines;

    LinearColorMap _colorMap;
};
#endif // MAINWINDOW_H
