/**
 * @file XmlAreaOfViewIngestor.h
 * @brief Ingestion service for loading Area of View Polygons from XML files.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef XMLAREAOFVIEWINGESTOR_H
#define XMLAREAOFVIEWINGESTOR_H

#include "../models/AreaOfViewRecord.h"
#include "../repositories/IAreaOfViewRepository.h"
#include <QString>
#include <QVector>
#include <QObject>

namespace GISApp::Core::Services {

class XmlAreaOfViewIngestor : public QObject {
    Q_OBJECT
public:
    explicit XmlAreaOfViewIngestor(Repositories::IAreaOfViewRepository *repository, QObject *parent = nullptr);
    ~XmlAreaOfViewIngestor() override = default;

    bool parseAndSaveXmlFile(const QString &filePath);
    QVector<Models::AreaOfViewRecord> parseXmlContent(const QString &xmlData);

private:
    Repositories::IAreaOfViewRepository *m_repository{nullptr};
};

} // namespace GISApp::Core::Services

#endif // XMLAREAOFVIEWINGESTOR_H
