#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QProgressBar>
#include <QTreeView>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include "settingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseFolderClicked();
    void onFileSelected(const QModelIndex &index);
    void onGenerateMetadataClicked();
    void onSettingsButtonClicked();
    void sendImageToApi(const QString &filePath);
    void onApiResponseReceived(QNetworkReply *reply);
    void updateProgressBar(int value);

private:
    Ui::MainWindow *ui;
    QFileSystemModel *fileModel;
    QNetworkAccessManager *networkManager;
    QString currentFilePath; // Хранение пути к выбранному файлу

    QString currentApiKey;          // Хранение текущего API-ключа
    QString currentAiProvider;      // Хранение текущего провайдера (Gemini AI или OpenAI)

    SettingsDialog *settingsDialog; // Указатель на окно настроек
};

#endif // MAINWINDOW_H