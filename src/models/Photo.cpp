#include "Photo.h"

Photo::Photo(const QString &filePath) : filePath(filePath) {}

QString Photo::getFilePath() const {
    return filePath;
}

void Photo::setDescription(const QString &description) {
    this->description = description;
}

QString Photo::getDescription() const {
    return description;
}