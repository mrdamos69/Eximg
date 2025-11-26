#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // Установка значений по умолчанию
    ui->aiProviderComboBox->addItems(QStringList() << "Gemini AI" << "OpenAI");
    ui->apiKeyLineEdit->setPlaceholderText("Enter your API Key");
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

QString SettingsDialog::getAiProvider() const
{
    return ui->aiProviderComboBox->currentText();
}

QString SettingsDialog::getApiKey() const
{
    return ui->apiKeyLineEdit->text();
}
