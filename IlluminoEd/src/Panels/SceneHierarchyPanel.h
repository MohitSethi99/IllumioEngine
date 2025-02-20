#pragma once

#include <IlluminoEngine.h>

#include "BasePanel.h"

namespace IlluminoEngine
{
	class SceneHierarchyPanel : public BasePanel
	{
	public:
		SceneHierarchyPanel() = default;
		virtual ~SceneHierarchyPanel() = default;

		void OnUpdate(Timestep ts);
		void OnImGuiRender();

		void SetSelectionContext(Scene* scene) { m_SelectionContext = scene; }
		void SetSelection(Entity entity) { m_SelectedEntity = entity; }

		Entity GetSelection() { return m_SelectedEntity; }

	private:
		void DrawEntityNode(Entity entity);

	private:
		Scene* m_SelectionContext = nullptr;
		Entity m_SelectedEntity = {};
		Entity m_HoveredEntity = {};
		Entity m_DeleteEntity = {};
		uint32_t m_CurrentlyVisibleEntities = 0;
	};
}
