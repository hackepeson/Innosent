#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qcustomplot.h"
#include <QSerialPort>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  QCPColorScale *colorScale;
  QSerialPort m_SerialPort;
  QCPColorMap* colorMap;
  bool updateGraph;

private slots:
  void updatePlot(QVector<double> fft);
  void readyRead();
  void clearPlot();
  void stopUpdate();
  void saveFile();
};

#endif // MAINWINDOW_H
