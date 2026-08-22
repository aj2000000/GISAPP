/**
 * @file ToolManager.h
 * @brief Context class in Strategy Pattern managing active GIS tool execution.
 */

#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QObject>
#include <memory>
#include <unordered_map>
#include "core/interfaces/ITool.h"

namespace GISApp::Controllers {

/**
 * @class ToolManager
 * @brief Manages tool registration, activation, and mouse event delegation.
 */
class ToolManager : public QObject {
    Q_OBJECT

public:
    explicit ToolManager(QObject *parent = nullptr);
    ~ToolManager() override = default;

    /**
     * @brief Registers a tool strategy with the manager.
     */
    void registerTool(std::shared_ptr<GISApp::Core::Interfaces::ITool> tool);

    /**
     * @brief Sets the active tool strategy by name.
     */
    bool setActiveTool(const QString &toolName);

    /**
     * @brief Gets currently active tool strategy.
     */
    GISApp::Core::Interfaces::ITool* activeTool() const;

    // --- Mouse Event Delegates ---
    void handleMousePress(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coord);
    void handleMouseMove(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coord);
    void handleMouseRelease(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coord);

signals:
    /**
     * @brief Emitted when the active tool changes.
     */
    void activeToolChanged(const QString &toolName);

private:
    std::unordered_map<QString, std::shared_ptr<GISApp::Core::Interfaces::ITool>> m_tools;
    std::shared_ptr<GISApp::Core::Interfaces::ITool> m_activeTool;
};

} // namespace GISApp::Controllers

#endif // TOOLMANAGER_H
