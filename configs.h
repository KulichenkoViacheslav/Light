#ifndef CONFIGS_H
#define CONFIGS_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>

#define MAX_SIZE_DATA_MODE 100

// Оголошення структури
enum Status {
    Active,
    Transition,
    Deactivate
};

struct Data {
    Status status;
    QColor color;
    int interval;
};

namespace Ui {
class Configs;
}

class Configs : public QDialog
{
    Q_OBJECT

public:
    explicit Configs(QWidget *parent = nullptr);
    ~Configs();
    void loadDefaultConfigurationFromFile();

signals:
    void setNewDataMode(const Data newDataMode[], int size);

private slots:
    void cellDoubleClicked(int row, int column);
    void cellClicked(int row, int column);
    void OkButtonClicked();
    void CancelButtonClicked();
    void NewButtonClicked();
    void SaveButtonClicked();
    void LoadButtonClicked();
    void updateCellValue(QComboBox *comboBox, int column, int index);

private:
    Ui::Configs *ui;
    void addComboBoxesToFirstRow();
    void addSpinBoxesToThirdRow();
    void saveConfigurations(const QString &fileName, const Data saveDataMode[], int size);
    void LoadParameters(const QString &fileName, Data * dataMode, int * size);
    void UpdateTableParameters(const Data updateDataMode[], int size);
    Data configsDataMode[MAX_SIZE_DATA_MODE]; // Масив вструктур на 100 елементів
    int  sizeDataMode;
    QString modeDirectory;
};

#endif // CONFIGS_H
