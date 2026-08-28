/**
 * @file IAreaOfViewRepository.h
 * @brief Abstract Repository interface for Area of View Polygon Entities.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef IAREAOFVIEWREPOSITORY_H
#define IAREAOFVIEWREPOSITORY_H

#include "../models/AreaOfViewRecord.h"
#include <QVector>
#include <QObject>
#include <optional>

namespace GISApp::Core::Repositories {

class IAreaOfViewRepository : public QObject {
    Q_OBJECT
public:
    explicit IAreaOfViewRepository(QObject *parent = nullptr) : QObject(parent) {}
    ~IAreaOfViewRepository() override = default;

    virtual bool insertOrUpdate(const Models::AreaOfViewRecord &record) = 0;
    virtual bool insertBatch(const QVector<Models::AreaOfViewRecord> &records) = 0;
    virtual QVector<Models::AreaOfViewRecord> getAll() const = 0;
    virtual std::optional<Models::AreaOfViewRecord> getById(int id) const = 0;
    virtual bool deleteById(int id) = 0;
    virtual bool clearAll() = 0;
    virtual int count() const = 0;

signals:
    void areaOfViewUpdated();
};

} // namespace GISApp::Core::Repositories

#endif // IAREAOFVIEWREPOSITORY_H
