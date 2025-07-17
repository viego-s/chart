#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTableWidget>
#include <QtCharts>
#include <QFile>
#include <QDebug>

QT_CHARTS_USE_NAMESPACE

    class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        // 加载UI文件
        setupUi(this);

        // 初始化图表
        chart = new QChart();
        chartView = findChild<QChartView*>("widget");
        chartView->setChart(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        // 连接信号槽
        connect(findChild<QPushButton*>("btnRead"), &QPushButton::clicked, this, &MainWindow::readDataFromFile);
        connect(findChild<QPushButton*>("btnSaveData"), &QPushButton::clicked, this, &MainWindow::saveDataToFile);
        connect(findChild<QPushButton*>("btnSavePlot"), &QPushButton::clicked, this, &MainWindow::savePlotToFile);
        connect(findChild<QPushButton*>("btnClear"), &QPushButton::clicked, this, &MainWindow::clearAll);

        // 初始化表格
        tableWidget = findChild<QTableWidget*>("tableWidget");
        tableWidget->setColumnCount(2);
        tableWidget->setHorizontalHeaderLabels({"列1", "列2"});

        // 初始化日志框
        textEdit = findChild<QTextEdit*>("textEdit");
    }

private slots:
    void readDataFromFile() {
        QString filePath = QFileDialog::getOpenFileName(
            this, "选择数据文件", "", "CSV文件 (*.csv)");

        if (filePath.isEmpty()) return;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "错误", "无法打开文件!");
            return;
        }

        // 清空现有数据
        clearAll();

        QTextStream in(&file);
        int row = 0;
        QVector<double> col1Data, col2Data;

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList values = line.split(',');

            if (values.size() < 2) continue;

            bool ok1, ok2;
            double val1 = values[0].toDouble(&ok1);
            double val2 = values[1].toDouble(&ok2);

            if (ok1 && ok2) {
                tableWidget->insertRow(row);
                tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(val1)));
                tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(val2)));

                col1Data.append(val1);
                col2Data.append(val2);
                row++;
            }
        }
        file.close();

        // 绘制图表
        plotData(col1Data, col2Data);

        // 计算统计值并输出
        calculateStats(col1Data, col2Data);
    }

    void saveDataToFile() {
        if (tableWidget->rowCount() == 0) {
            QMessageBox::warning(this, "警告", "没有数据可保存!");
            return;
        }

        QString filePath = QFileDialog::getSaveFileName(
            this, "保存数据", "", "CSV文件 (*.csv)");

        if (filePath.isEmpty()) return;

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "错误", "无法创建文件!");
            return;
        }

        QTextStream out(&file);
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < tableWidget->columnCount(); ++col) {
                QTableWidgetItem *item = tableWidget->item(row, col);
                rowData << (item ? item->text() : "");
            }
            out << rowData.join(",") << "\n";
        }
        file.close();

        textEdit->append("数据已保存到: " + filePath);
    }

    void savePlotToFile() {
        if (chart->series().isEmpty()) {
            QMessageBox::warning(this, "警告", "没有图表可保存!");
            return;
        }

        QString filePath = QFileDialog::getSaveFileName(
            this, "保存图表", "", "PNG图片 (*.png);;JPG图片 (*.jpg)");

        if (filePath.isEmpty()) return;

        QPixmap pixmap = chartView->grab();
        if (pixmap.save(filePath)) {
            textEdit->append("图表已保存到: " + filePath);
        } else {
            QMessageBox::critical(this, "错误", "保存图片失败!");
        }
    }

    void clearAll() {
        tableWidget->setRowCount(0);
        chart->removeAllSeries();
        chart->removeAxis(chart->axisX());
        chart->removeAxis(chart->axisY());
        textEdit->clear();
    }

private:
    void plotData(const QVector<double> &col1, const QVector<double> &col2) {
        if (col1.isEmpty() || col2.isEmpty()) return;

        QLineSeries *series = new QLineSeries();
        for (int i = 0; i < col1.size(); ++i) {
            series->append(col1[i], col2[i]);
        }

        chart->addSeries(series);
        chart->createDefaultAxes();
        chart->setTitle("数据曲线图");
    }

    void calculateStats(const QVector<double> &col1, const QVector<double> &col2) {
        if (col1.isEmpty() || col2.isEmpty()) return;

        auto calculate = [](const QVector<double> &data) -> QVector<double> {
            double min = data[0], max = data[0], sum = 0;
            for (double val : data) {
                if (val < min) min = val;
                if (val > max) max = val;
                sum += val;
            }
            return {min, max, sum / data.size()};
        };

        QVector<double> stats1 = calculate(col1);
        QVector<double> stats2 = calculate(col2);

        QString log = "列1统计:\n";
        log += "  最小值: " + QString::number(stats1[0]) + "\n";
        log += "  最大值: " + QString::number(stats1[1]) + "\n";
        log += "  平均值: " + QString::number(stats1[2]) + "\n\n";

        log += "列2统计:\n";
        log += "  最小值: " + QString::number(stats2[0]) + "\n";
        log += "  最大值: " + QString::number(stats2[1]) + "\n";
        log += "  平均值: " + QString::number(stats2[2]);

        textEdit->setText(log);
    }

private:
    QTableWidget *tableWidget;
    QTextEdit *textEdit;
    QChart *chart;
    QChartView *chartView;
};
