#include "configs.h"
#include "ui_configs.h"
#include <QColorDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

Configs::Configs(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Configs)
{
    ui->setupUi(this);

    // Встановлення постійного фонового коліру
    QString styleSheet = "background-color: lightgrey; color: black;"; // Фон - lightgrey, шрифт - чорний
    setStyleSheet(styleSheet);

    // Заповнення першого і третього рядка таблиці
    addComboBoxesToFirstRow();
    addSpinBoxesToThirdRow();

    QString tableStyle = "QTableWidget { background-color: #f2f2f2; }";
    QString headerStyle = "QHeaderView::section { background-color: #cccccc; color: black; }";
    QString lineStyle = "QTableWidget { gridline-color: black; border-width: 2px; }";

    ui->tableWidget->setStyleSheet(tableStyle + headerStyle + lineStyle);

    // Підключення кнопки Ок на генерацію сигналу до форми Light на зміну кольору
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &Configs::cellDoubleClicked);
    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &Configs::cellClicked);

    // Підключення сигналів кнопок до слотів
    connect(ui->OkButton, &QPushButton::clicked, this, &Configs::OkButtonClicked);
    connect(ui->CancelButton, &QPushButton::clicked, this, &Configs::CancelButtonClicked);
    connect(ui->NewButton, &QPushButton::clicked, this, &Configs::NewButtonClicked);
    connect(ui->SaveButton, &QPushButton::clicked, this, &Configs::SaveButtonClicked);
    connect(ui->LoadButton, &QPushButton::clicked, this, &Configs::LoadButtonClicked);

    // Отримання шляху до кореня програми
    QString appRootPath = QCoreApplication::applicationDirPath();
    // Додавання додаткової директорії з режимами роботи
    this->modeDirectory = appRootPath + "/Modes/";
}

Configs::~Configs()
{
    delete ui;
}

void Configs::loadDefaultConfigurationFromFile()
{
    QString defaultConfigFilePath = this->modeDirectory + "default_mode.ini";

    // Перевіряємо, чи існує файл
    QFileInfo fileInfo(defaultConfigFilePath);

    if (fileInfo.exists() && fileInfo.isFile()) {
        // Завантажуємо данні з файлу
        LoadParameters(defaultConfigFilePath, configsDataMode, &sizeDataMode);

        // Відправлення сигналу із даними
        emit setNewDataMode(configsDataMode, sizeDataMode);

        if (sizeDataMode > 0)
        {
            UpdateTableParameters(configsDataMode, sizeDataMode);
        }
    }
}

void Configs::cellDoubleClicked(int row, int column)
{
    // Перевірка, чи клік був у другому рядку
    if (row == 1)
    {
        // Отримання поточного кольору фону ячейки
        QTableWidgetItem *item = ui->tableWidget->item(row, column);
        if (item)
        {
            QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0, column));
            if (!comboBox)
                return;

            QString statusText = comboBox->currentText();
            QColor currentColor = item->background().color();

            if (statusText == "Активно")
            {
                // Створення діалогового вікна вибору кольору
                QColor color = QColorDialog::getColor(currentColor, this, "Виберіть колір", QColorDialog::DontUseNativeDialog);

                // Перевірка, чи був вибраний колір
                if (color.isValid())
                {
                    // Створення об'єкта кисті з вибраним кольором
                    QBrush brush(color);

                    // Встановлення кольору фону для ячейки
                    item->setBackground(brush);

                    // Додавання текстового значення до ячейки
                    item->setText(color.name());
                }
            }
            else {
                QMessageBox::information(this, "Помилка", "Вибір кольору доступний тільки в активному режимі.");
            }
        }
    }
}

void Configs::OkButtonClicked()
{
    // Очистка масиву configsDataMode перед заповненням новими даними
    memset(configsDataMode, 0, sizeof(configsDataMode));

    int dataSize = 0; // Розмір масиву даних
    int columnCount = ui->tableWidget->columnCount(); // Отримання кількості стовбців у таблиці

    // Збір даних з таблиці і збереження їх у масиві configsDataMode
    for (int i = 0; i < columnCount; ++i) {
        // Отримання статусу з комбобоксу у першому рядку стовбця
        QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0, i));
        if (!comboBox)
            continue;

        QString statusText = comboBox->currentText();
        if (statusText == "---") {
            // Якщо статус "---", перестати заповнювати масив та відправити сигнал з обробленою кількістю даних
            break;
        }

        // Отримання кольору з фону QTableWidgetItem у другому рядку стовбця
        QTableWidgetItem *item = ui->tableWidget->item(1, i);
        if (!item)
            continue;

        Status status;
        if (statusText == "Активно")
            status = Active;
        else if (statusText == "Перехід")
            status = Transition;
        else if (statusText == "Вимкнено")
            status = Deactivate;
        else
            continue; // Пропустити невідомі статуси

        configsDataMode[dataSize].status = status;
        configsDataMode[dataSize].color = item->background().color();

        // Отримання значення з QSpinBox у третьому рядку стовбця
        QSpinBox *spinBox = qobject_cast<QSpinBox*>(ui->tableWidget->cellWidget(2, i));
        if (spinBox)
            configsDataMode[dataSize].interval = spinBox->value();
        else
            configsDataMode[dataSize].interval = 0;

        // Збільшення розміру масиву
        ++dataSize;
    }

    // Відправлення сигналу із даними
    emit setNewDataMode(configsDataMode, dataSize);

    // Збереження конфігураційних даних у файл
    QString defaulModeName = this->modeDirectory + "default_mode.ini";
    saveConfigurations(defaulModeName, configsDataMode, dataSize);
}

void Configs::CancelButtonClicked()
{
}

void Configs::NewButtonClicked()
{
    // Переведення всіх елементів ComboBox у початковий стан
    for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
        QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0, column));
        if (comboBox) {
            comboBox->setCurrentIndex(0);
        }
    }

    // Переведення всіх елементів у другому рядку у початковий стан
    for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
        QTableWidgetItem *item = ui->tableWidget->item(1, column);
        if (item) {
            item->setBackground(QBrush(Qt::NoBrush));
            item->setText("");
        }
    }

    // Переведення всіх елементів SpinBox у початковий стан
    for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
        QSpinBox *spinBox = qobject_cast<QSpinBox*>(ui->tableWidget->cellWidget(2, column));
        if (spinBox) {
            spinBox->setValue(0);
        }
    }
}

void Configs::SaveButtonClicked()
{
    // Відкриття діалогового вікна збереження файлу в директорії кореня програми
    QString saveFilePath = QFileDialog::getSaveFileName(nullptr, "Зберегти файл", modeDirectory, "INI Files (*.ini)");

    if (!saveFilePath.isEmpty()) {
        int dataSize = 0; // Розмір масиву даних
        int columnCount = ui->tableWidget->columnCount(); // Отримання кількості стовбців у таблиці

        Data saveDataMode[columnCount];

        // Збір даних з таблиці і збереження їх у масиві configsDataMode
        for (int i = 0; i < columnCount; ++i) {
            // Отримання статусу з комбобоксу у першому рядку стовбця
            QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0, i));
            if (!comboBox)
                continue;

            QString statusText = comboBox->currentText();
            if (statusText == "---") {
                // Якщо статус "---", перестати заповнювати масив та відправити сигнал з обробленою кількістю даних
                break;
            }

            // Отримання кольору з фону QTableWidgetItem у другому рядку стовбця
            QTableWidgetItem *item = ui->tableWidget->item(1, i);
            if (!item)
                continue;

            Status status;
            if (statusText == "Активно")
                status = Active;
            else if (statusText == "Перехід")
                status = Transition;
            else if (statusText == "Вимкнено")
                status = Deactivate;
            else
                continue; // Пропустити невідомі статуси

            saveDataMode[dataSize].status = status;
            saveDataMode[dataSize].color = item->background().color();

            // Отримання значення з QSpinBox у третьому рядку стовбця
            QSpinBox *spinBox = qobject_cast<QSpinBox*>(ui->tableWidget->cellWidget(2, i));
            if (spinBox)
                saveDataMode[dataSize].interval = spinBox->value();
            else
                saveDataMode[dataSize].interval = 0;

            // Збільшення розміру масиву
            ++dataSize;
        }

        // Збереження конфігураційних даних у файл
        saveConfigurations(saveFilePath, saveDataMode, dataSize);
    }
}

void Configs::LoadButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Виберіть файл конфігурацій"), // Назва вікна діалогу
                                                    this->modeDirectory, // Початковий каталог
                                                    tr("INI файли (*.ini)")); // Фільтр для файлів

    if (!fileName.isEmpty()) {
        // Завантажуємо данні з файлу
        LoadParameters(fileName, configsDataMode, &sizeDataMode);

        // Заповнюємо таблицю даними, що були прочитані
        UpdateTableParameters(configsDataMode, sizeDataMode);
    }
}

void Configs::LoadParameters(const QString &fileName, Data * dataMode, int * size)
{
    // Якщо файл існує і є файлом, завантажуємо дані
    QSettings settings(fileName, QSettings::IniFormat);
    *size = settings.beginReadArray("data");
    settings.endArray();

    for (int i = 0; i < *size; ++i) {
        QString groupName = QString("DataMode_%1").arg(i);
        settings.beginGroup(groupName);

        // Отримуємо значення статусу
        int statusInt = settings.value("Status").toInt();
        if (statusInt == static_cast<int>(Active)) {
            dataMode[i].status = Active;
        } else if (statusInt == static_cast<int>(Transition)) {
            dataMode[i].status = Transition;
        } else if (statusInt == static_cast<int>(Deactivate)) {
            dataMode[i].status = Deactivate;
        } else {
            // Обробка помилки: невідомий статус
        }

        // Зчитуємо значення кольору з файлу
        QString colorString = settings.value("Color").toString();
        dataMode[i].color = QColor::fromString(colorString);

        // Зчитуємо значення інтервалу з файлу
        dataMode[i].interval = settings.value("Interval").toInt();

        settings.endGroup();
    }
}

void Configs::UpdateTableParameters(const Data updateDataMode[], int size)
{
    for (int i = 0; i < size; ++i) {
        // Оновлення комбобоксу
        QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0, i));
        if (comboBox) {
            // Встановлення вибраного елемента за індексом `status`
            comboBox->setCurrentIndex(updateDataMode[i].status + 1);
        }

        // Оновлення кольору фону ячейки таблиці
        QTableWidgetItem *item = ui->tableWidget->item(1, i);
        if (item) {
            // Встановлення кольору фону за допомогою `color`
            if (updateDataMode[i].status == Active)
            {
                // Створення об'єкта кисті з вибраним кольором
                QBrush brush(updateDataMode[i].color);

                // Встановлення кольору фону для ячейки
                item->setBackground(brush);

                // Додавання текстового значення до ячейки
                item->setText(updateDataMode[i].color.name());
            }
        }

        // Оновлення значення спінбоксу
        QSpinBox *spinBox = qobject_cast<QSpinBox*>(ui->tableWidget->cellWidget(2, i));
        if (spinBox) {
            // Встановлення значення спінбоксу за інтервалом `interval`
            spinBox->setEnabled(true);
            spinBox->setValue(updateDataMode[i].interval);
            spinBox->setStyleSheet("QSpinBox { background-color: white; color: black; font-weight: bold; }");
        }
    }
    for (int i = size; i < MAX_SIZE_DATA_MODE; ++i) {
        // Оновлення комбобоксу
        QComboBox *comboBox = qobject_cast<QComboBox*>(ui->tableWidget->cellWidget(0, i));
        if (comboBox) {
            // Встановлення вибраного елемента за індексом `status`
            comboBox->setCurrentIndex(0);
        }

        // Видалення коліру заливки для поточної ячейки
        QTableWidgetItem *item = ui->tableWidget->item(1, i);
        if (item) {
            item->setBackground(QBrush(Qt::NoBrush));
            item->setText("");
        }

        // Оновлення значення спінбоксу
        QSpinBox *spinBox = qobject_cast<QSpinBox*>(ui->tableWidget->cellWidget(2, i));
        if (spinBox) {
            spinBox->setEnabled(false);
            spinBox->setValue(0);
            spinBox->setStyleSheet("QSpinBox { background-color: lightgrey; color: darkgrey; }");
        }
    }
}

void Configs::addComboBoxesToFirstRow()
{
    QStringList options = {"---", "Активно", "Перехід", "Вимкнено"};

    for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
        // Створення випадаючого списку
        QComboBox *comboBox = new QComboBox(this);
        comboBox->addItems(options);

        // Підключення сигналу зміни випадаючого списку до слоту для оновлення значення в ячейці таблиці
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            updateCellValue(comboBox, column, index);
        });

        // Встановлення значення списку у відповідну ячейку першого рядка
        comboBox->setCurrentIndex(0);
        ui->tableWidget->setCellWidget(0, column, comboBox);
    }
}

void Configs::updateCellValue(QComboBox *comboBox, int column, int index)
{
    QString selectedItem = comboBox->itemText(index);
    QTableWidgetItem *item = ui->tableWidget->item(0, column);
    if (item) {
        item->setText(selectedItem);
        // Зміна стилю тексту у залежності від вибраного елемента комбобоксу
        QFont font = item->font();
        if (selectedItem == "---") {
            // Якщо вибрано "---", робимо текст звичайного стилю
            font.setWeight(QFont::Normal);
        } else {
            // Інакше робимо текст жирним
            font.setWeight(QFont::Bold);
        }
        item->setFont(font);
    }

    // Отримуємо відповідний QSpinBox у третьому рядку для цього комбінованого поля
    QSpinBox *spinBox = qobject_cast<QSpinBox*>(ui->tableWidget->cellWidget(2, column));
    if (spinBox) {
        // Встановимо стан QSpinBox залежно від обраного значення комбобоксу
        spinBox->setEnabled(selectedItem != "---");

        // Зміна стилю спінбоксу у залежності від вибраного елемента комбобоксу
        if (selectedItem == "---") {
            // Якщо вибрано "---", робимо фон світлосірим і текст темно-сірим
            spinBox->setStyleSheet("QSpinBox { background-color: lightgrey; color: darkgrey; }");
            spinBox->setValue(0);
        } else {
            spinBox->setStyleSheet("QSpinBox { background-color: white; color: black; font-weight: bold; }");
            if (spinBox->value() == 0) {
                spinBox->setValue(1);
            }
        }
    }

    // Перевірка, чи вибраний статус "---" або "Перехід"
    if (selectedItem == "---" || selectedItem == "Перехід")
    {
        // Видалення коліру заливки для поточної ячейки
        item = ui->tableWidget->item(1, column);
        if (item) {
            item->setBackground(QBrush(Qt::NoBrush));
            item->setText("");
        }
    }
}

void Configs::addSpinBoxesToThirdRow()
{
    for (int column = 0; column < ui->tableWidget->columnCount(); ++column) {
        // Створення спінбоксу
        QSpinBox *spinBox = new QSpinBox(this);
        spinBox->setStyleSheet("QSpinBox { background-color: lightGray; color: darkGray; }");
        spinBox->setMinimum(0);
        spinBox->setMaximum(10000); // Vаксимальне значення встановлено як 10 секунд

        // Підключення сигналу зміни значення спінбоксу до слоту для оновлення значення в ячейці таблиці
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
            QTableWidgetItem *item = ui->tableWidget->item(2, column);
            if (item) {
                item->setText(QString::number(value));
            }
        });

        // Встановлення значення спінбоксу у відповідну ячейку третього рядка
        spinBox->setValue(0); // Наприклад, початкове значення встановлено як 0
        spinBox->setEnabled(false); // За замовчуванням створюємо елемент виключеним
        ui->tableWidget->setCellWidget(2, column, spinBox);
    }
}

void Configs::cellClicked(int row, int column)
{

}

void Configs::saveConfigurations(const QString &fileName, const Data saveDataMode[], int size) {
    // Створення або відкриття файлу конфігурації для запису
    QSettings settings(fileName, QSettings::IniFormat);

    // Збереження розміру масиву
    settings.beginGroup("data");
    settings.setValue("size", size);
    settings.endGroup();

    // Збереження конфігураційних даних у файл
    for (int i = 0; i < size; ++i) {
        // Формування назви групи для кожного режиму даних
        QString groupName = QString("DataMode_%1").arg(i);

        // Встановлення групи
        settings.beginGroup(groupName);

        // Збереження значень для кожного режиму даних
        settings.setValue("Status", static_cast<int>(saveDataMode[i].status));
        settings.setValue("Color", saveDataMode[i].color.name());
        settings.setValue("Interval", saveDataMode[i].interval);

        // Завершення групи
        settings.endGroup();
    }
}



