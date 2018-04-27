#include "mainwindow.h"
#include "ui_mainwindow.h"

QVector<double> vecFreq;
int t = 0;


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  QString comName("COM62");
  updateGraph = true;

  m_SerialPort.setPortName(comName);

  if (m_SerialPort.open(QIODevice::ReadWrite))
  {
    qDebug() << "Opening port" << m_SerialPort.portName();
    m_SerialPort.flush();
    m_SerialPort.setBaudRate(8*115200);
    m_SerialPort.setFlowControl(QSerialPort::NoFlowControl);
    m_SerialPort.setDataBits(QSerialPort::Data8);
    m_SerialPort.setStopBits(QSerialPort::OneStop);
    m_SerialPort.setParity(QSerialPort::NoParity);
    m_SerialPort.setReadBufferSize(8192);
    //m_SerialPort.waitForReadyRead();
  }
  else
  {
    qDebug() << "Failed to open " << m_SerialPort.portName();
  }

  colorScale = new QCPColorScale(ui->customPlot);
  ui->customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
  ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
  ui->customPlot->axisRect()->setupFullAxesBox(true);

  colorMap = new QCPColorMap(ui->customPlot->xAxis, ui->customPlot->yAxis);

  connect(ui->pushButtonClear, SIGNAL(pressed()), SLOT(clearPlot()));
  connect(&m_SerialPort, SIGNAL(readyRead()), SLOT(readyRead()));
  connect(ui->pushButtonStop, SIGNAL(clicked(bool)), SLOT(stopUpdate()));
  connect(ui->pushButtonSave, SIGNAL(clicked(bool)), SLOT(saveFile()));

  for (int i = 0; i < 64; i++)
  {
    vecFreq.append((double)i);
  }



}

MainWindow::~MainWindow()
{
  delete ui;
}

void extractFFT(QByteArray byteArray, QVector<double>* fft)
{
#define INDEX_OFFSET 516
  int32_t data;
  fft->clear();
  for (uint32_t i = 0; i < 256; i+=4)
  {
    data = byteArray.at(INDEX_OFFSET+i)<<24;
    data |= byteArray.at(INDEX_OFFSET+i+1)<<16;
    data |= byteArray.at(INDEX_OFFSET+i+2)<<8;
    data |= byteArray.at(INDEX_OFFSET+i+3);
    fft->append((double)data);
  }
}


void MainWindow::readyRead()
{
#define SD (char)(0xA2)
#define ED (char)(0x16)
#define FC (char)(0xE0)

  static QByteArray rxData;
  char c;
  QVector<double> vecFFT;


  while (m_SerialPort.bytesAvailable())
  {
    m_SerialPort.read(&c,1);

    rxData.append(c);


    if ((c == SD) && (rxData.at(rxData.length()-2) == ED))
    {
      qDebug() << rxData.length();

      if (rxData.length() == 1031)
      {
        ui->customPlot->clearGraphs();
        ui->customPlot->addGraph();
        extractFFT(rxData, &vecFFT);
        updatePlot(vecFFT);
        /*ui->customPlot->graph(0)->setData(vecFreq,vecFFT);
        ui->customPlot->rescaleAxes();
        ui->customPlot->replot();
        */
      }
      rxData.clear();
    }
  }
}



void MainWindow::updatePlot(QVector<double> fft)
{
#define NO_OF_FFT_VECTORS 300


  static bool firstTime = true;

  if (updateGraph)
  {
    colorMap->data()->setSize(NO_OF_FFT_VECTORS,64);
    colorMap->data()->setRange(QCPRange(0,NO_OF_FFT_VECTORS),QCPRange(0,64));

    if (firstTime)
    {
      colorMap->setInterpolate(true);
    }

    for (int i = 0; i < 64; i++)
    {
      colorMap->data()->setCell(t,i,fft.at(i));
    }

    t++;
    if (t > NO_OF_FFT_VECTORS-1)
    {
      t = 0;
    }
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    colorMap->setGradient(QCPColorGradient::gpSpectrum);

    ui->customPlot->rescaleAxes();
    ui->customPlot->xAxis->setRange(0,NO_OF_FFT_VECTORS);

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange();
    //colorMap->setDataRange(QCPRange(-2.0,colorMap->dataRange().upper));

    ui->customPlot->replot();
  }
}


void MainWindow::clearPlot()
{
  t = 0;
  colorMap->data()->clear();

}


void MainWindow::stopUpdate()
{
  updateGraph = !updateGraph;

  if (updateGraph)
  {
    ui->pushButtonStop->setText("Stop");
  }
  else
  {
    ui->pushButtonStop->setText("Run");
  }
}

void MainWindow::saveFile()
{
  QString pdfFileName("pdf.pdf");

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save to PDF"), pdfFileName, tr("PDF Files (*.pdf)"));
  if (!fileName.isEmpty())
  {
    ui->customPlot->savePdf(fileName);
  }
}
