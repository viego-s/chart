#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtCharts>  // 包含所有Qt Charts相关头文件
#include <QVector>
#include <QDebug>

// 使用Qt Charts命名空间
QT_CHARTS_USE_NAMESPACE

    namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void readDataFromFile();      // 从文件读取数据
    void saveDataToFile();        // 保存数据到文件
    void savePlotToFile();        // 保存图片到文件
    void clearAll();              // 清空所有内容

private:
    void plotData(const QVector<double> &col1, const QVector<double> &col2);  // 绘制曲线图
    void calculateStats(const QVector<double> &col1, const QVector<double> &col2);  // 计算统计值

    Ui::MainWindow *ui;
    QChart *chart;                // 图表对象
};

#endif // MAINWINDOW_H
