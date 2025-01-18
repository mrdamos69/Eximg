#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include "Photo.h"
#include "MetadataGenerator.h"
#include <QList>

class MainController {
public:
    MainController();

    void addPhoto(const QString &filePath);
    void generateMetadata();

    const QList<Photo>& getPhotos() const;

private:
    QList<Photo> photos;
    MetadataGenerator metadataGenerator;
};