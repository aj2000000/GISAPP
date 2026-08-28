/**
 * @file AreaOfViewRepository.h
 * @brief Concrete SQLite repository for Area of View Polygon Entities.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef AREAOFVIEWREPOSITORY_H
#define AREAOFVIEWREPOSITORY_H

#include "IAreaOfViewRepository.h"

namespace GISApp::Core::Repositories {

class AreaOfViewRepository : public IAreaOfViewRepository {
    Q_OBJECT
public:
    explicit AreaOfViewRepository(QObject *parent = nullptr);
    ~AreaOfViewRepository() override = default;

    bool insertOrUpdate(const Models::AreaOfViewRecord &record) override;
    bool insertBatch(const QVector<Models::AreaOfViewRecord> &records) override;
    QVector<Models::AreaOfViewRecord> getAll() const override;
    std::optional<Models::AreaOfViewRecord> getById(int id) const override;
    bool deleteById(int id) override;
    bool clearAll() override;
    int count() const override;

private:
    bool insertOrUpdateInternal(const Models::AreaOfViewRecord &record);
};

} // namespace GISApp::Core::Repositories

#endif // AREAOFVIEWREPOSITORY_H
