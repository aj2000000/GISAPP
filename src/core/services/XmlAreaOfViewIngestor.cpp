#include "XmlAreaOfViewIngestor.h"
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QRegularExpression>
#include <QDebug>
#include <cmath>
#include <optional>

namespace GISApp::Core::Services {

XmlAreaOfViewIngestor::XmlAreaOfViewIngestor(Repositories::IAreaOfViewRepository *repository, QObject *parent)
    : QObject(parent), m_repository(repository)
{
}

bool XmlAreaOfViewIngestor::parseAndSaveXmlFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "[XmlAreaOfViewIngestor] Failed to open XML file:" << filePath;
        return false;
    }

    QString xmlContent = QString::fromUtf8(file.readAll());
    file.close();

    QVector<Models::AreaOfViewRecord> records = parseXmlContent(xmlContent);
    if (records.isEmpty()) {
        qWarning() << "[XmlAreaOfViewIngestor] No valid Area of View records parsed from XML:" << filePath;
        return false;
    }

    // System rule: Always maintain a single Area of View polygon
    if (records.size() > 1) {
        records = { records.first() };
    }

    if (m_repository) {
        m_repository->clearAll();
        bool ok = m_repository->insertBatch(records);
        if (ok) {
            qDebug() << "[XmlAreaOfViewIngestor] Ingested single Area of View polygon from XML into repository.";
        }
        return ok;
    }

    return true;
}

static std::optional<Models::AreaOfViewRecord> parseSinglePolygonElem(const QDomElement &elem, int defaultIndex)
{
    Models::AreaOfViewRecord rec;

    // 1. Name
    if (elem.hasAttribute("name")) rec.name = elem.attribute("name");
    else if (elem.hasAttribute("Name")) rec.name = elem.attribute("Name");

    if (rec.name.isEmpty()) {
        QDomNodeList nameNodes = elem.elementsByTagName("Name");
        if (nameNodes.isEmpty()) nameNodes = elem.elementsByTagName("name");
        if (!nameNodes.isEmpty()) {
            rec.name = nameNodes.at(0).toElement().text().trimmed();
        }
    }

    // 2. NPoints
    if (elem.hasAttribute("n_points")) rec.nPoints = elem.attribute("n_points").toInt();
    else if (elem.hasAttribute("NPoints")) rec.nPoints = elem.attribute("NPoints").toInt();
    else if (elem.hasAttribute("npoints")) rec.nPoints = elem.attribute("npoints").toInt();

    if (rec.nPoints == 0) {
        QDomNodeList npNodes = elem.elementsByTagName("NPoints");
        if (npNodes.isEmpty()) npNodes = elem.elementsByTagName("n_points");
        if (npNodes.isEmpty()) npNodes = elem.elementsByTagName("npoints");
        if (!npNodes.isEmpty()) {
            rec.nPoints = npNodes.at(0).toElement().text().trimmed().toInt();
        }
    }

    // 3. Extract Points
    // Check for child elements: <Point>, <point>, <Vertex>, <vertex>
    QDomNodeList pointNodes = elem.elementsByTagName("Point");
    if (pointNodes.isEmpty()) pointNodes = elem.elementsByTagName("point");
    if (pointNodes.isEmpty()) pointNodes = elem.elementsByTagName("Vertex");
    if (pointNodes.isEmpty()) pointNodes = elem.elementsByTagName("vertex");

    for (int i = 0; i < pointNodes.size(); ++i) {
        QDomElement ptElem = pointNodes.at(i).toElement();
        double lat = 0.0, lon = 0.0, height = 0.0;

        // Check attributes first
        if (ptElem.hasAttribute("lat")) lat = ptElem.attribute("lat").toDouble();
        else if (ptElem.hasAttribute("latitude")) lat = ptElem.attribute("latitude").toDouble();
        else if (ptElem.hasAttribute("Latitude")) lat = ptElem.attribute("Latitude").toDouble();

        if (ptElem.hasAttribute("lon")) lon = ptElem.attribute("lon").toDouble();
        else if (ptElem.hasAttribute("lng")) lon = ptElem.attribute("lng").toDouble();
        else if (ptElem.hasAttribute("long")) lon = ptElem.attribute("long").toDouble();
        else if (ptElem.hasAttribute("longitude")) lon = ptElem.attribute("longitude").toDouble();
        else if (ptElem.hasAttribute("Longitude")) lon = ptElem.attribute("Longitude").toDouble();

        if (ptElem.hasAttribute("height")) height = ptElem.attribute("height").toDouble();
        else if (ptElem.hasAttribute("Height")) height = ptElem.attribute("Height").toDouble();
        else if (ptElem.hasAttribute("alt")) height = ptElem.attribute("alt").toDouble();
        else if (ptElem.hasAttribute("altitude")) height = ptElem.attribute("altitude").toDouble();

        // If lat/lon not found in attributes, check child elements (<Latitude>, <Longitude>, <Height>)
        if (lat == 0.0 && lon == 0.0) {
            QDomElement latNode = ptElem.firstChildElement("Latitude");
            if (latNode.isNull()) latNode = ptElem.firstChildElement("lat");
            if (latNode.isNull()) latNode = ptElem.firstChildElement("latitude");
            if (!latNode.isNull()) lat = latNode.text().trimmed().toDouble();

            QDomElement lonNode = ptElem.firstChildElement("Longitude");
            if (lonNode.isNull()) lonNode = ptElem.firstChildElement("lon");
            if (lonNode.isNull()) lonNode = ptElem.firstChildElement("lng");
            if (lonNode.isNull()) lonNode = ptElem.firstChildElement("long");
            if (lonNode.isNull()) lonNode = ptElem.firstChildElement("longitude");
            if (!lonNode.isNull()) lon = lonNode.text().trimmed().toDouble();

            QDomElement hNode = ptElem.firstChildElement("Height");
            if (hNode.isNull()) hNode = ptElem.firstChildElement("height");
            if (hNode.isNull()) hNode = ptElem.firstChildElement("alt");
            if (hNode.isNull()) hNode = ptElem.firstChildElement("altitude");
            if (!hNode.isNull()) height = hNode.text().trimmed().toDouble();
        }

        if (lat != 0.0 || lon != 0.0) {
            rec.points.append(Models::Coordinate3D(lat, lon, height));
        }
    }

    // Check for <coordinates> tag if no <Point> nodes parsed
    if (rec.points.isEmpty()) {
        QDomNodeList coordsNodes = elem.elementsByTagName("coordinates");
        if (coordsNodes.isEmpty()) coordsNodes = elem.elementsByTagName("Coordinates");
        if (!coordsNodes.isEmpty()) {
            QString coordsText = coordsNodes.at(0).toElement().text().trimmed();
            QStringList tokens = coordsText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            for (const QString &token : tokens) {
                QStringList parts = token.split(',', Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    double lon = parts[0].toDouble();
                    double lat = parts[1].toDouble();
                    double height = (parts.size() >= 3) ? parts[2].toDouble() : 0.0;
                    rec.points.append(Models::Coordinate3D(lat, lon, height));
                }
            }
        }
    }

    // Sequential <Latitude>/<Longitude> tags directly inside elem
    if (rec.points.isEmpty()) {
        double currentLat = 0.0, currentLon = 0.0, currentH = 0.0;
        bool hasLat = false, hasLon = false;

        QDomNodeList allChildren = elem.childNodes();
        for (int i = 0; i < allChildren.size(); ++i) {
            QDomNode child = allChildren.at(i);
            if (child.isElement()) {
                QDomElement childElem = child.toElement();
                QString tag = childElem.tagName().toLower();
                if (tag == "lat" || tag == "latitude") {
                    if (hasLat && hasLon) {
                        rec.points.append(Models::Coordinate3D(currentLat, currentLon, currentH));
                        hasLat = false; hasLon = false; currentH = 0.0;
                    }
                    currentLat = childElem.text().trimmed().toDouble();
                    hasLat = true;
                } else if (tag == "lon" || tag == "long" || tag == "lng" || tag == "longitude") {
                    currentLon = childElem.text().trimmed().toDouble();
                    hasLon = true;
                } else if (tag == "height" || tag == "alt" || tag == "altitude") {
                    currentH = childElem.text().trimmed().toDouble();
                }
            }
        }
        if (hasLat && hasLon) {
            rec.points.append(Models::Coordinate3D(currentLat, currentLon, currentH));
        }
    }

    if (rec.name.isEmpty()) {
        rec.name = QString("Area of View %1").arg(defaultIndex);
    }
    if (rec.nPoints == 0) {
        rec.nPoints = rec.points.size();
    }

    // Ensure polygon ring closure
    if (rec.points.size() >= 3) {
        const auto &first = rec.points.first();
        const auto &last = rec.points.last();
        if (std::abs(first.latitude - last.latitude) > 1e-6 ||
            std::abs(first.longitude - last.longitude) > 1e-6) {
            rec.points.append(first);
        }
        return rec;
    }

    return std::nullopt;
}

QVector<Models::AreaOfViewRecord> XmlAreaOfViewIngestor::parseXmlContent(const QString &xmlData)
{
    QVector<Models::AreaOfViewRecord> records;
    QDomDocument doc;
    QDomDocument::ParseResult parseResult = doc.setContent(xmlData);
    if (!parseResult) {
        qWarning() << "[XmlAreaOfViewIngestor] XML DOM parse error:" << parseResult.errorMessage << "at line" << parseResult.errorLine;
        return records;
    }

    QDomElement root = doc.documentElement();

    auto isPolygonTag = [](const QString &tagName) {
        QString lower = tagName.toLower();
        return lower == "polygon" || lower == "areaofview" || lower == "aov" ||
               lower == "area_of_view" || lower == "area" || lower == "zone";
    };

    QList<QDomElement> polygonElements;

    // Check children of root element
    QDomNodeList children = root.childNodes();
    for (int i = 0; i < children.size(); ++i) {
        QDomNode node = children.at(i);
        if (node.isElement()) {
            QDomElement childElem = node.toElement();
            if (isPolygonTag(childElem.tagName())) {
                polygonElements.append(childElem);
            }
        }
    }

    // If no children polygon tags, check if root itself is a polygon
    if (polygonElements.isEmpty() && isPolygonTag(root.tagName())) {
        polygonElements.append(root);
    }

    // If still empty, search all descendants with tag Polygon, AreaOfView, AoV, etc.
    if (polygonElements.isEmpty()) {
        QDomNodeList allPolygons = root.elementsByTagName("Polygon");
        if (allPolygons.isEmpty()) allPolygons = root.elementsByTagName("AreaOfView");
        if (allPolygons.isEmpty()) allPolygons = root.elementsByTagName("AoV");
        if (allPolygons.isEmpty()) allPolygons = root.elementsByTagName("Zone");
        for (int i = 0; i < allPolygons.size(); ++i) {
            polygonElements.append(allPolygons.at(i).toElement());
        }
    }

    for (int i = 0; i < polygonElements.size(); ++i) {
        auto recOpt = parseSinglePolygonElem(polygonElements.at(i), records.size() + 1);
        if (recOpt.has_value()) {
            records.append(recOpt.value());
        }
    }

    return records;
}

} // namespace GISApp::Core::Services
