#ifndef PHOTO_H
#define PHOTO_H

#include <QString>

class Photo {
public:
    Photo(const QString &filePath);

    QString getFilePath() const;
    void setDescription(const QString &description);
    QString getDescription() const;

private:
    QString filePath;
    QString description;
};

#endif // PHOTO_H