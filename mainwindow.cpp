#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHeaderView>
#include <QPixmap>
#include <QDebug>
#include <limits>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
    setupChart();
    connectSignals();
}

MainWindow::~MainWindow() {

}

void MainWindow::setupUi() {
    resize(800, 600);
    setWindowTitle("Data Visualization");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建表格
    tableWidget = new QTableWidget(centralWidget);
    tableWidget->setGeometry(40, 100, 300, 341);
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels({"X", "Y"});
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 创建图表视图
    chartView = new QChartView(centralWidget);
    chartView->setGeometry(360, 100, 421, 341);

    // 创建文本编辑框
    textEdit = new QTextEdit(centralWidget);
    textEdit->setGeometry(40, 450, 631, 71);
    textEdit->setReadOnly(true);

    // 创建按钮
    btnRead = new QPushButton("从文件读取数据", centralWidget);
    btnRead->setGeometry(40, 530, 141, 51);

    btnSaveData = new QPushButton("保存数据到文件", centralWidget);
    btnSaveData->setGeometry(280, 530, 141, 51);

    btnSavePlot = new QPushButton("保存图片到文件", centralWidget);
    btnSavePlot->setGeometry(530, 530, 141, 51);

    btnClear = new QPushButton("清空", centralWidget);
    btnClear->setGeometry(40, 20, 141, 51);
}

void MainWindow::setupChart() {
    chart = new QChart();
    chart->setTitle("数据曲线图");
    chart->setAnimationOptions(QChart::NoAnimation);

    // 创建系列但不立即添加到图表
    series1 = new QLineSeries();
    series1->setName("X列");
    series2 = new QLineSeries();
    series2->setName("Y列");

    // 创建坐标轴
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();

    // 添加坐标轴到图表
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
}

void MainWindow::connectSignals() {
    connect(btnRead, &QPushButton::clicked, this, &MainWindow::readDataFromFile);
    connect(btnSaveData, &QPushButton::clicked, this, &MainWindow::saveDataToFile);
    connect(btnSavePlot, &QPushButton::clicked, this, &MainWindow::savePlotToFile);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::clearAll);
}

void MainWindow::readDataFromFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "打开数据文件", "", "CSV文件 (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法打开文件: " + file.errorString());
        return;
    }

    clearAll();

    QTextStream in(&file);
    int row = 0;

    // 修复：正确初始化统计变量
    double minX = std::numeric_limits<double>::max();
    double minY = minX;
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = maxX;
    double sumX = 0, sumY = 0;
    int count = 0;

    // 临时存储数据点，减少图表刷新次数
    QVector<QPointF> points1, points2;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList values = line.split(',');
        if (values.size() < 2) continue;

        bool ok1, ok2;
        double x = values[0].toDouble(&ok1);
        double y = values[1].toDouble(&ok2);

        if (!ok1 || !ok2) continue;

        // 更新统计数据
        minX = qMin(minX, x);
        minY = qMin(minY, y);
        maxX = qMax(maxX, x);
        maxY = qMax(maxY, y);
        sumX += x;
        sumY += y;
        count++;

        // 添加到表格
        tableWidget->insertRow(row);
        tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(x, 'f', 6)));
        tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(y, 'f', 6)));

        // 添加到临时点集
        points1.append(QPointF(row, x));
        points2.append(QPointF(row, y));

        row++;
    }

    file.close();

    // 批量添加点到系列
    if (!points1.isEmpty()) {
        series1->replace(points1);
        series2->replace(points2);

        // 添加到图表
        chart->addSeries(series1);
        chart->addSeries(series2);

        // 附加到坐标轴
        QValueAxis *axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
        QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());

        series1->attachAxis(axisX);
        series1->attachAxis(axisY);
        series2->attachAxis(axisX);
        series2->attachAxis(axisY);

        // 设置坐标轴范围
        double xMin = 0;
        double xMax = points1.size() > 0 ? points1.size() - 1 : 1;
        double yMin = qMin(minX, minY) - 0.5;
        double yMax = qMax(maxX, maxY) + 0.5;

        axisX->setRange(xMin, xMax);
        axisY->setRange(yMin, yMax);
    }

    // 计算并显示统计信息
    if (count > 0) {
        QString stats = QString("X列统计: 最小值=%1, 最大值=%2, 平均值=%3\n"
                                "Y列统计: 最小值=%4, 最大值=%5, 平均值=%6")
                            .arg(minX, 0, 'f', 3)
                            .arg(maxX, 0, 'f', 3)
                            .arg(sumX/count, 0, 'f', 3)
                            .arg(minY, 0, 'f', 3)
                            .arg(maxY, 0, 'f', 3)
                            .arg(sumY/count, 0, 'f', 3);
        textEdit->setText(stats);
    } else {
        QMessageBox::warning(this, "警告", "文件未包含有效数据");
    }
}

void MainWindow::saveDataToFile() {
    if (tableWidget->rowCount() == 0) {
        QMessageBox::warning(this, "警告", "没有数据可保存");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "保存数据文件", "", "CSV文件 (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QString x = tableWidget->item(row, 0)->text();
        QString y = tableWidget->item(row, 1)->text();
        out << x << "," << y << "\n";
    }

    file.close();
    textEdit->append("数据已保存到: " + fileName);
}

void MainWindow::savePlotToFile() {
    if (series1->points().isEmpty()) {
        QMessageBox::warning(this, "警告", "没有图表可保存");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "保存图表", "", "PNG图像 (*.png);;JPEG图像 (*.jpg);;所有文件 (*)");

    if (fileName.isEmpty()) return;

    QPixmap pixmap = chartView->grab();
    if (!pixmap.save(fileName)) {
        QMessageBox::critical(this, "错误", "无法保存图像");
    } else {
        textEdit->append("图表已保存到: " + fileName);
    }
}

void MainWindow::clearAll() {
    tableWidget->setRowCount(0);

    // 安全地清除系列
    if (series1) {
        series1->clear();
    }
    if (series2) {
        series2->clear();
    }

    textEdit->clear();

    // 移除系列但不删除
    if (chart) {
        chart->removeSeries(series1);
        chart->removeSeries(series2);
    }

    // 重置坐标轴范围
    if (chart && !chart->axes().isEmpty()) {
        QValueAxis *axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
        QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());

        if (axisX) axisX->setRange(0, 1);
        if (axisY) axisY->setRange(0, 1);
    }
}
