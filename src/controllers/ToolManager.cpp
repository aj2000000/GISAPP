/**
 * @file ToolManager.cpp
 * @brief Implementation of ToolManager strategy switching and delegation.
 */

#include "controllers/ToolManager.h"
#include "../tools/PanTool.h"
#include "../tools/MeasureTool.h"

namespace GISApp::Controllers {

ToolManager::ToolManager(QObject *parent)
    : QObject(parent), m_activeTool(nullptr)
{
}

void ToolManager::registerDefaultTools(QObject *parent)
{
    auto panTool = std::make_shared<GISApp::Tools::PanTool>(parent);
    auto measureTool = std::make_shared<GISApp::Tools::MeasureTool>(parent);
    
    registerTool(panTool);
    registerTool(measureTool);
    setActiveTool("PanTool");
}

void ToolManager::registerTool(std::shared_ptr<GISApp::Core::Interfaces::ITool> tool)
{
    if (tool) {
        m_tools[tool->toolName()] = tool;
        if (!m_activeTool) {
            setActiveTool(tool->toolName());
        }
    }
}

bool ToolManager::setActiveTool(const QString &toolName)
{
    auto it = m_tools.find(toolName);
    if (it != m_tools.end()) {
        if (m_activeTool) {
            m_activeTool->deactivate();
        }
        m_activeTool = it->second;
        m_activeTool->activate();
        emit activeToolChanged(toolName);
        return true;
    }
    return false;
}

GISApp::Core::Interfaces::ITool* ToolManager::activeTool() const
{
    return m_activeTool.get();
}

void ToolManager::handleMousePress(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coord)
{
    if (m_activeTool) {
        m_activeTool->onMousePress(event, coord);
    }
}

void ToolManager::handleMouseMove(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coord)
{
    if (m_activeTool) {
        m_activeTool->onMouseMove(event, coord);
    }
}

void ToolManager::handleMouseRelease(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coord)
{
    if (m_activeTool) {
        m_activeTool->onMouseRelease(event, coord);
    }
}

} // namespace GISApp::Controllers
