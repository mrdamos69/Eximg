#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include <QTimer>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , fileModel(new QFileSystemModel(this))
    , networkManager(new QNetworkAccessManager(this))
    , settingsDialog(new SettingsDialog(this)) // Инициализация окна настроек
{
    ui->setupUi(this);

    // Настройка файловой модели
    fileModel->setRootPath(QDir::homePath());
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    fileModel->setNameFilters(QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp");
    fileModel->setNameFilterDisables(false);

    ui->fileTreeView->setModel(fileModel);

    // Настройка прогресс-бара
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(false);

    // Установка значений по умолчанию
    currentAiProvider = "Gemini AI";
    currentApiKey = "";

    // Подключение сигналов
    connect(ui->browseFolderButton, &QPushButton::clicked, this, &MainWindow::onBrowseFolderClicked);
    connect(ui->fileTreeView, &QTreeView::clicked, this, &MainWindow::onFileSelected);
    connect(ui->generateMetadataButton, &QPushButton::clicked, this, &MainWindow::onGenerateMetadataClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onApiResponseReceived);
}

MainWindow::~MainWindow() {
    delete ui;
}

// Обновление прогресс-бара
void MainWindow::updateProgressBar(int value) {
    ui->progressBar->setValue(value);
    ui->progressBar->setVisible(value < 100); // Скрыть, когда завершено
}

// Открытие диалога выбора папки
void MainWindow::onBrowseFolderClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"), QDir::homePath());
    if (!dir.isEmpty()) {
        fileModel->setRootPath(dir);
        ui->fileTreeView->setRootIndex(fileModel->index(dir));
    }
}

// Выбор файла
void MainWindow::onFileSelected(const QModelIndex &index) {
    if (!index.isValid()) return;

    currentFilePath = fileModel->filePath(index);

    QPixmap pixmap(currentFilePath);
    if (!pixmap.isNull()) {
        ui->photoPreviewLabel->setPixmap(pixmap.scaled(ui->photoPreviewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load the selected image."));
    }
}

// Сохранение настроек AI
void MainWindow::onSettingsButtonClicked() {
    // Показываем диалог настроек
    if (settingsDialog->exec() == QDialog::Accepted) {
        currentAiProvider = settingsDialog->getAiProvider();
        currentApiKey = settingsDialog->getApiKey();
        QMessageBox::information(this, tr("Settings Saved"), tr("AI provider and API key have been updated."));
    }
}

// Отправка изображения на сервер AI
void MainWindow::sendImageToApi(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open the image file."));
        return;
    }

    QByteArray imageData = file.readAll();
    QString imageBase64 = QString::fromLatin1(imageData.toBase64());
    file.close();

    QJsonObject requestBody;

    if (currentAiProvider == "Gemini AI") {
        // Формат запроса для Gemini AI
        requestBody["provider"] = "gemini";
        requestBody["image"] = imageBase64;
    } else if (currentAiProvider == "OpenAI") {
        // Формат запроса для OpenAI
        QJsonObject message;
        message["role"] = "user";
        message["content"] = "Generate metadata (title, description, 25 keywords) for the following image: " + imageBase64;

        requestBody["model"] = "gpt-4";
        requestBody["messages"] = QJsonArray({message});
    }

    QNetworkRequest request(currentAiProvider == "Gemini AI"
                            ? QUrl("https://gemini-ai-api.example.com/generate")
                            : QUrl("https://api.openai.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + currentApiKey.toUtf8());

    // Показ прогресс-бара
    updateProgressBar(0);

    // Отправка запроса
    networkManager->post(request, QJsonDocument(requestBody).toJson());
}

// Генерация метаданных
void MainWindow::onGenerateMetadataClicked() {
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No image selected."));
        return;
    }

    sendImageToApi(currentFilePath);
}

// Обработка ответа от API
void MainWindow::onApiResponseReceived(QNetworkReply *reply) {
    updateProgressBar(100); // Завершение прогресса

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error:" << reply->errorString();
        qDebug() << "HTTP Status Code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "Response body:" << reply->readAll();

        QMessageBox::critical(this, tr("Error"), tr("Failed to generate metadata: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    qDebug() << "API Response:" << responseData;

    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
    if (!responseDoc.isObject()) {
        QMessageBox::critical(this, tr("Error"), tr("Invalid API response."));
        reply->deleteLater();
        return;
    }

    QJsonObject responseObject = responseDoc.object();
    QJsonArray choices = responseObject.value("choices").toArray();
    if (choices.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No response from the model."));
        reply->deleteLater();
        return;
    }

    QString content = choices[0].toObject().value("message").toObject().value("content").toString();

    // Разбиваем ответ на части
    QStringList metadata = content.split("\n");
    if (metadata.size() >= 3) {
        ui->titleField->setText(metadata[0].trimmed());
        ui->descriptionField->setPlainText(metadata[1].trimmed());
        ui->keywordsField->setPlainText(metadata.mid(2).join(", "));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Unexpected response format from API."));
    }

    reply->deleteLater();
}
