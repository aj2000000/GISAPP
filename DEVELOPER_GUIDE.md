# BrahmaxisGIS — Developer Guide: Unified Entity Management System (EMS)

Welcome to the **BrahmaxisGIS Architecture Developer Guide**. This document outlines how to create, register, store, render, and edit custom spatial entities using the **Unified Entity Management System (EMS)**.

---

## 🏛️ Architectural Overview

The BrahmaxisGIS EMS replaces fragmented, single-use entity classes with a polymorphic, data-driven framework adhering strictly to **SOLID design principles**:

1. **Single Point Registration**: Add new entity types with zero core code modifications.
2. **Generic Persistence Engine**: Automatic SQLite database persistence without writing custom SQL tables or migrations.
3. **Automated GeoJSON Synchronization**: MapLibre rendering automatically updates when entity repositories emit change signals.
4. **Universal Form Inspector**: Auto-generated Qt UI properties dialogs based on entity schema declarations.

```
       +----------------------------+
       |     GisEntityRegistry      |  <-- Single Point Registration (Factory)
       +--------------+-------------+
                      |
                      v
       +----------------------------+
       |      GenericGisEntity      |  <-- Polymorphic Model (QVariantMap attributes)
       +--------------+-------------+
                      |
       +--------------+-------------------+
       |                                  |
       v                                  v
+-----------------------------+ +-----------------------------------+
|   GenericEntityRepository   | |    MapLibreGenericEntityAdapter   |
|   (SQLite + JSON Storage)   | |  (Auto-GeoJSON + MapLibre Sync) |
+--------------+--------------+ +-----------------------------------+
               |
               v
+-----------------------------+
| UniversalEntityEditorDialog | <-- Auto-Generated Dynamic UI Inspector
+-----------------------------+
```

---

## 🚀 Step 1: Registering a New Custom Entity Type

To introduce a new entity type into BrahmaxisGIS (e.g. `drones`, `navigational_beacons`, `sensor_nodes`), register it in `GisEntityRegistry`:

```cpp
#include "core/models/GisEntityRegistry.h"

using namespace GISApp::Core::Models;

void registerMyCustomEntity() {
    EntityTypeDescriptor beaconDesc;
    beaconDesc.typeId = "beacon";
    beaconDesc.displayName = "Navigational Beacon";
    beaconDesc.category = EntityCategory::Waypoint;
    beaconDesc.painterStrategyId = "circle"; // "circle", "line", or "polygon"
    
    // Default visual styling
    beaconDesc.defaultStyle.strokeColor = QColor("#ec4899"); // Pink stroke
    beaconDesc.defaultStyle.fillColor = QColor(236, 72, 153, 100);
    beaconDesc.defaultStyle.strokeWidth = 2.5;

    // Define custom schema properties for auto UI form generation
    beaconDesc.propertySchemas = {
        {"frequency_mhz", "Frequency (MHz)", QMetaType::Double, 433.92, true, {}},
        {"signal_strength", "Signal Strength (dBm)", QMetaType::Double, -75.0, false, {}},
        {"status", "Status", QMetaType::QString, "ACTIVE", true, {"ACTIVE", "STANDBY", "FAULT", "OFFLINE"}}
    };

    // Register with single line call
    GisEntityRegistry::instance().registerEntityType(beaconDesc);
}
```

---

## 💾 Step 2: Creating and Persisting Entities

Create new entities via `GisEntityRegistry::createEntity()` and store them in `GenericEntityRepository`:

```cpp
#include "core/models/GisEntityRegistry.h"
#include "core/models/IGisGeometry.h"
#include "core/repositories/GenericEntityRepository.h"

using namespace GISApp::Core::Models;
using namespace GISApp::Core::Repositories;

void createAndSaveBeacon(GenericEntityRepository *repo, double lat, double lon) {
    // 1. Create Point Geometry
    auto geom = std::make_shared<PointGeometry>(lat, lon);

    // 2. Instantiate Entity from Registry
    auto beacon = GisEntityRegistry::instance().createEntity("beacon", "Alpha Beacon #01");
    if (beacon) {
        beacon->setGeometry(geom);

        // Customize dynamic properties if desired
        beacon->setProperty("frequency_mhz", 915.0);
        beacon->setProperty("status", "ACTIVE");

        // 3. Save to Repository (Persists to SQLite & Triggers Map Redraw)
        repo->addEntity(beacon);
    }
}
```

---

## 🎨 Step 3: MapLibre Automated Map Rendering

`MapLibreGenericEntityAdapter` listens to `GenericEntityRepository` signals (`entityAdded`, `entityUpdated`, `entityRemoved`, `repositoryCleared`) and automatically updates the MapLibre GeoJSON layer in real time:

```cpp
#include "core/services/MapLibreGenericEntityAdapter.h"

// Instantiate adapter when MapLibre widget is ready:
auto adapter = new MapLibreGenericEntityAdapter(mapLibreMap, genericEntityRepo, parentWidget);
```

Zero extra adapter code is required when adding new entity types!

---

## 📝 Step 4: Universal Entity Property Inspector UI

Edit any registered entity dynamically with `UniversalEntityEditorDialog`:

```cpp
#include "ui/entities/UniversalEntityEditorDialog.h"

using namespace GISApp::UI::Entities;

void editEntity(std::shared_ptr<GenericGisEntity> entity, QWidget *parent, GenericEntityRepository *repo) {
    UniversalEntityEditorDialog dialog(entity, parent);
    if (dialog.exec() == QDialog::Accepted) {
        // Persist updated attributes and style back to database
        repo->updateEntity(entity);
    }
}
```

---

## 🎯 Verification Checklist for Adding New Entities

- [ ] Call `GisEntityRegistry::instance().registerEntityType(...)` at application startup.
- [ ] Set geometry (`PointGeometry`, `PolylineGeometry`, or `PolygonGeometry`).
- [ ] Save to `GenericEntityRepository`.
- [ ] Verify map render and dynamic attribute UI forms.

---

*BrahmaxisGIS Core Systems Team — 2026*
