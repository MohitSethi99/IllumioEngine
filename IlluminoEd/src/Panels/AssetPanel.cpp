#include "AssetPanel.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <filesystem>

#include "../Utils/UI.h"
#include "../Utils/EditorTheme.h"

namespace IlluminoEngine
{
	static const std::filesystem::path s_AssetPath = "assets";

	static const char* GetFileIcon(const char* ext)
	{
		if (!(strcmp(ext, "txt") && strcmp(ext, "md")))
			return ICON_MDI_FILE_DOCUMENT;
		if (!(strcmp(ext, "png") && strcmp(ext, "jpg") && strcmp(ext, "jpeg") && strcmp(ext, "bmp") && strcmp(ext, "gif")))
			return ICON_MDI_FILE_IMAGE;
		if (!(strcmp(ext, "hdr") && strcmp(ext, "tga")))
			return ICON_MDI_IMAGE_FILTER_HDR;
		if (!(strcmp(ext, "glsl")))
			return ICON_MDI_IMAGE_FILTER_BLACK_WHITE;
		if (!(strcmp(ext, "obj") && strcmp(ext, "fbx") && strcmp(ext, "gltf")))
			return ICON_MDI_VECTOR_POLYGON;

		return ICON_MDI_FILE;
	}
	
#define BIT(x) (1 << x)

	std::pair<bool, uint32_t> AssetPanel::DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selectionMask, ImGuiTreeNodeFlags flags)
	{
		OPTICK_EVENT();

		bool anyNodeClicked = false;
		uint32_t nodeClicked = 0;

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			ImGuiTreeNodeFlags nodeFlags = flags;

			bool entryIsFile = !std::filesystem::is_directory(entry.path());
			if (entryIsFile)
				nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			const bool selected = (*selectionMask & BIT(*count)) != 0;
			if (selected)
			{
				nodeFlags |= ImGuiTreeNodeFlags_Selected;
				ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::HeaderSelectedColor);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderSelectedColor);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderHoveredColor);
			}

			bool open = ImGui::TreeNodeEx((void*)(intptr_t)(*count), nodeFlags, "");
			ImGui::PopStyleColor(selected ? 2 : 1);

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				if (!entryIsFile)
					UpdateDirectoryEntries(entry.path());

				nodeClicked = *count;
				anyNodeClicked = true;
			}

			eastl::string name = StringUtils::GetNameWithExtension(entry.path().string().c_str());
			const char* folderIcon;
			if (entryIsFile)
				folderIcon = GetFileIcon(StringUtils::GetExtension((eastl::string&&)name).c_str());
			else
				folderIcon = open ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
			
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
			ImGui::TextUnformatted(folderIcon);
			ImGui::PopStyleColor();
			ImGui::SameLine();
			float x = ImGui::GetContentRegionAvail().x;
			ImGui::TextUnformatted(name.c_str());
			m_CurrentlyVisibleItemsTreeView++;

			(*count)--;

			if (!entryIsFile)
			{
				if (open)
				{
					auto clickState = DirectoryTreeViewRecursive(entry.path(), count, selectionMask, flags);

					if (!anyNodeClicked)
					{
						anyNodeClicked = clickState.first;
						nodeClicked = clickState.second;
					}

					ImGui::TreePop();
				}
				else
				{
					for (const auto& e : std::filesystem::recursive_directory_iterator(entry.path()))
						(*count)--;
				}
			}
		}

		return { anyNodeClicked, nodeClicked };
	}

	AssetPanel::AssetPanel()
		: m_CurrentDirectory(s_AssetPath)
	{
		OPTICK_EVENT();

		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");

		m_CurrentDirectory = s_AssetPath;
		UpdateDirectoryEntries(s_AssetPath);
	}

	void AssetPanel::OnImGuiRender()
	{
		OPTICK_EVENT();

		if (OnBegin(ICON_MDI_FOLDER, "Assets"))
		{
			RenderHeader();
			
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

			ImGui::Columns(2, nullptr, true);
			{
				ImVec2 region = ImGui::GetContentRegionAvail();
				static bool firstTime = true;
				if (!firstTime)
					m_TreeViewColumnWidth = ImGui::GetColumnWidth(0);
				else
					firstTime = false;
				ImGui::SetColumnWidth(0, m_TreeViewColumnWidth);

				float cursorPosY = ImGui::GetCursorPosY() + 10.0f;
				ImGui::SetCursorPosY(cursorPosY);
				ImGui::BeginChild("TreeView");
				{
					RenderSideView();
				}
				ImGui::EndChild();
				ImGui::NextColumn();
				ImGui::Separator();
				ImGui::SetCursorPosY(cursorPosY);
				ImGui::BeginChild("FolderView");
				{
					RenderBody();
				}
				ImGui::EndChild();
			}
			ImGui::PopStyleVar();
			ImGui::Columns(1);

			OnEnd();
		}
	}

	void AssetPanel::RenderHeader()
	{
		OPTICK_EVENT();

		if (ImGui::Button(ICON_MDI_COGS))
			ImGui::OpenPopup("SettingsPopup");
		if (ImGui::BeginPopup("SettingsPopup"))
		{
			ImGui::SliderFloat("Thumbnail Size", &m_ThumbnailSize, 64.0f, 128.0f);
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		ImGui::TextUnformatted(ICON_MDI_MAGNIFY);
		ImGui::SameLine();

		float spacing = ImGui::GetStyle().ItemSpacing.x;
        ImGui::GetStyle().ItemSpacing.x = 2;

		{
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[1];
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
			m_Filter.Draw("###ConsoleFilter", ImGui::GetContentRegionAvail().x);
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[0];
		}

		ImGui::GetStyle().ItemSpacing.x = spacing;
		
		if(!m_Filter.IsActive())
		{
			ImGui::SameLine();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[1];
			ImGui::SetCursorPosX(ImGui::GetFontSize() * 4.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, ImGui::GetStyle().FramePadding.y));
			ImGui::TextUnformatted("Search...");
			ImGui::PopStyleVar();
			ImGui::GetIO().FontDefault = ImGui::GetIO().Fonts->Fonts[0];
		}

		// Back button
		{
			bool disabledBackButton = false;
			if (m_CurrentDirectory == std::filesystem::path(s_AssetPath))
				disabledBackButton = true;
		
			if (disabledBackButton)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::Button(ICON_MDI_ARROW_LEFT))
				UpdateDirectoryEntries(m_CurrentDirectory.parent_path());

			if (disabledBackButton)
			{
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}
		}
		ImGui::SameLine();
		ImGui::Text(m_CurrentDirectory.string().c_str());
	}

	void AssetPanel::RenderSideView()
	{
		OPTICK_EVENT();

		static int selectionMask = 0;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_FramePadding
			| ImGuiTreeNodeFlags_SpanFullWidth;

		float x1 = ImGui::GetCurrentWindow()->WorkRect.Min.x;
		float x2 = ImGui::GetCurrentWindow()->WorkRect.Max.x;
		float line_height = ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0f;
		UI::DrawRowsBackground(m_CurrentlyVisibleItemsTreeView, line_height, x1, x2, 0, 0, ImGui::GetColorU32(EditorTheme::WindowBgAlternativeColor));
		
		m_CurrentlyVisibleItemsTreeView = 1;

		ImGuiTreeNodeFlags nodeFlags = flags;
		const bool selected = m_CurrentDirectory == s_AssetPath && selectionMask == 0;
		if (selected)
		{
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
			ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::HeaderSelectedColor);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderSelectedColor);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::HeaderHoveredColor);
		}

		bool opened = ImGui::TreeNodeEx((void*)("Assets"), nodeFlags, "");
		ImGui::PopStyleColor(selected ? 2 : 1);

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			UpdateDirectoryEntries(s_AssetPath);
			selectionMask = 0;
		}
		const char* folderIcon = open ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::AssetIconColor);
		ImGui::TextUnformatted(folderIcon);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		float x = ImGui::GetContentRegionAvail().x;
		ImGui::TextUnformatted("Assets");

		if (opened)
		{
			uint32_t count = 0;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(s_AssetPath))
				count++;

			auto clickState = DirectoryTreeViewRecursive(s_AssetPath, &count, &selectionMask, flags);

			if (clickState.first)
			{
				// (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
				if (ImGui::GetIO().KeyCtrl)
					selectionMask ^= BIT(clickState.second);          // CTRL+click to toggle
				else //if (!(selection_mask & (1 << clickState.second))) // Depending on selection behavior you want, may want to preserve selection when clicking on item that is part of the selection
					selectionMask = BIT(clickState.second);           // Click to single-select
			}

			ImGui::TreePop();
		}
	}

	void AssetPanel::RenderBody()
	{
		OPTICK_EVENT();

		std::filesystem::path directoryToOpen;
		bool directoryOpened = false;

		float padding = 4.0f;
		float cellSize = m_ThumbnailSize + 2 * padding;

		float overlayPaddingY = 6.0f * padding;
		float thumbnailPadding = overlayPaddingY * 0.5f;
		float thumbnailSize = m_ThumbnailSize - thumbnailPadding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int) (panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		uint32_t flags = ImGuiTableFlags_ContextMenuInBody
			| ImGuiTableFlags_SizingStretchSame
			| ImGuiTableFlags_PadOuterX
			| ImGuiTableFlags_NoClip;

		uint32_t i = 0;
		if (ImGui::BeginTable("BodyTable", columnCount, flags))
		{
			for (auto& file : m_DirectoryEntries)
			{
				ImGui::PushID(i++);

				uint64_t textureId = m_DirectoryIcon->GetRendererID();
				const char* fontIcon = nullptr;
				if (!file.DirectoryEntry.is_directory())
				{
					eastl::string ext = StringUtils::GetExtension((eastl::string&&)file.Name);
					if (file.IconTexture)
					{
						textureId = file.IconTexture->GetRendererID();
					}
					else
					{
						fontIcon = GetFileIcon(ext.c_str());
					}
				}

				ImGui::TableNextColumn();

				const char* filename = file.Name.c_str();
				ImVec2 textSize = ImGui::CalcTextSize(filename);
				ImVec2 cursorPos = ImGui::GetCursorPos();
				
				ImGui::InvisibleButton(("##" + std::to_string(i)).c_str(), { m_ThumbnailSize + padding * 2, m_ThumbnailSize + textSize.y + padding * 8 });
				if (ImGui::BeginDragDropSource())
				{
					eastl::string itemPath = file.DirectoryEntry.path().string().c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), (strlen(itemPath.c_str()) + 1) * sizeof(char));
					ImGui::EndDragDropSource();
				}
				
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (file.DirectoryEntry.is_directory())
					{
						directoryToOpen = m_CurrentDirectory / file.DirectoryEntry.path().filename();
						directoryOpened = true;
					}
				}

				ImDrawList& windowDrawList = *ImGui::GetWindowDrawList();

				ImVec2 rectMin = ImGui::GetItemRectMin();
				ImVec2 rectSize = ImGui::GetItemRectSize();

				float f = m_ThumbnailSize + padding * 2;
				ImGui::SetCursorPos(cursorPos);
				ImGui::SetItemAllowOverlap();
				if (fontIcon)
				{
					ImGui::InvisibleButton(fontIcon, { f, f });
					windowDrawList.AddText({ rectMin.x + rectSize.x * 0.5f - ImGui::CalcTextSize(fontIcon).x * 0.5f, rectMin.y + rectSize.y * 0.5f}, ImColor(1.0f, 1.0f, 1.0f), fontIcon);
				}
				else
				{
					ImGui::Image((ImTextureID)textureId, { f, f });
				}

				rectMin = ImGui::GetItemRectMin();
				rectSize = ImGui::GetItemRectSize();

				if (textSize.x + padding * 2 <= rectSize.x)
				{
					float rectMin_x = rectMin.x - padding + (rectSize.x - textSize.x) / 2;
					float rectMin_y = rectMin.y + rectSize.y;

					float rectMax_x = rectMin_x + textSize.x + padding * 2;
					float rectMax_y = rectMin_y + textSize.y + padding * 2;

					windowDrawList.AddText({ rectMin_x + padding, rectMin_y + padding * 2 }, ImColor(1.0f, 1.0f, 1.0f), filename);
				}
				else
				{
					float rectMin_y = rectMin.y + rectSize.y;

					float rectMax_x = rectMin.x + rectSize.x;
					float rectMax_y = rectMin_y + textSize.y + padding * 2;

					ImGui::RenderTextEllipsis(&windowDrawList, { rectMin.x + padding, rectMin_y + padding * 2 }, { rectMax_x, rectMax_y }, rectMax_x, rectMax_x, filename, nullptr, &textSize);
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + textSize.y + padding * 4);

				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		if (directoryOpened)
			UpdateDirectoryEntries(directoryToOpen);
	}

	void AssetPanel::UpdateDirectoryEntries(std::filesystem::path directory)
	{
		OPTICK_EVENT();

		m_CurrentDirectory = directory;
		m_DirectoryEntries.clear();
		for (auto& directoryEntry : std::filesystem::directory_iterator(directory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			eastl::string fileNameString = relativePath.filename().string().c_str();
			eastl::string ext = StringUtils::GetExtension((eastl::string&&)fileNameString);
			Ref<Texture2D> tex = nullptr;
			if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
				tex = Texture2D::Create(path.string().c_str());

			m_DirectoryEntries.push_back({ fileNameString, directoryEntry, tex });
		}
	}
}
