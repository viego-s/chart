#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QChartView>
#include <QTextEdit>
#include <QPushButton>
#include <QtCharts>

QT_CHARTS_USE_NAMESPACE

    class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupUi();
    void setupChart();
    void connectSignals();

    QTableWidget *tableWidget;
    QChartView *chartView;
    QTextEdit *textEdit;
    QPushButton *btnRead;
    QPushButton *btnSaveData;
    QPushButton *btnSavePlot;
    QPushButton *btnClear;

    QChart *chart;
    QLineSeries *series1;
    QLineSeries *series2;

private slots:
    void readDataFromFile();
    void saveDataToFile();
    void savePlotToFile();
    void clearAll();
};

#endif // MAINWINDOW_H
