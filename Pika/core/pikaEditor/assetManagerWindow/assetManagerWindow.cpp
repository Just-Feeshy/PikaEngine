#include "assetManagerWindow.h"
#include "pikaConfig.h"

#if PIKA_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

#if !PIKA_SHOULD_REMOVE_EDITOR

namespace pika
{

	void AssetManagerWindow::init(pika::pikaImgui::ImGuiIdsManager &idManager)
	{
		imguiId = idManager.getImguiIds();
	}
	
	void AssetManagerWindow::update(bool &open, ContainerManager &containerManager, LoadedDll &currentDll,
		pika::LogManager &logManager, pika::pikaImgui::ImGuiIdsManager &imguiIDsManager, ConsoleWindow *consoleWindow)
	{
		ImGui::PushID(imguiId);


		if (!ImGui::Begin(ICON_NAME, &open))
		{
			ImGui::End();
			ImGui::PopID();
			return;
		}

		//todo: for all windows
		ImGui::SetWindowSize({300,100}, ImGuiCond_FirstUseEver);

		if(std::filesystem::equivalent(currentPath, PIKA_RESOURCES_PATH) || searchText[0] != '\0')
		{
			ImGui::BeginDisabled(1);
		}
		else
		{
			ImGui::BeginDisabled(0);
		}

		if (ImGui::Button(ICON_FK_ARROW_UP))
		{
			currentPath = currentPath.parent_path();
		}

		ImGui::EndDisabled();

		ImGui::SameLine();

		ImGui::InputText("Search file", searchText, sizeof(searchText));

		ImGui::SameLine();

		if (ImGui::Button("Open resources folder"))
		{
		#if PIKA_WINDOWS
			ShellExecuteA(NULL, "open", PIKA_RESOURCES_PATH, NULL, NULL, SW_RESTORE);
		#endif
		}

		std::string longPath = currentPath.string();
		std::string root = PIKA_RESOURCES_PATH;
		std::string enginePath = "PIKA_RESOURCES_PATH/";
		std::string copyPath = "";
		if (longPath.size() > root.size())
		{
			enginePath += (longPath.c_str() + root.size());
			copyPath += (longPath.c_str() + root.size());
		}

		for (char &c : enginePath)
		{
			if (c == '\\')
			{
				c = '/';
			}
		}

		ImGui::Text(enginePath.c_str());

		ImGui::Separator();

		float contentW = ImGui::GetContentRegionAvail().x;
		const float size = 160;
		const float padding = 10;

		ImGui::Columns( std::max(1, (int)(contentW / (size + padding))), 0, false);

		//returns 1 if should break
		auto displayItem = [&](const std::filesystem::directory_entry &p) -> bool
		{


			if (ImGui::BeginChild(p.path().filename().string().c_str(), {size, size + 40}, false,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				//ImGui::
				ImFontAtlas *atlas = ImGui::GetIO().Fonts;

				ImGui::PushFont(atlas->Fonts[1]);

				if (p.is_directory())
				{

					if (ImGui::Button(ICON_FK_FOLDER_O, {size ,size}))
					{
						currentPath = p;

						//todo deffer
						ImGui::PopFont();
						ImGui::EndChild();
						ImGui::Columns();
						return 1;
					}
				}
				else
				{
					if (ImGui::Button(ICON_FK_FILE_O, {size ,size}))
					{
						auto it = currentDll.containerExtensionsSupport.find(p.path().filename().extension().string());
						if (it != currentDll.containerExtensionsSupport.end())
						{

							//todo name						
							containerManager.createContainer(it->second[0], currentDll, logManager, imguiIDsManager, consoleWindow, p.path().string());
						}
					}
				}

				ImGui::PopFont();

				ImGui::Text(p.path().filename().string().c_str());

				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::Button("reveal in explorer"))
					{

					#if PIKA_WINDOWS
						if (p.is_directory())
						{
							ShellExecuteA(NULL, "open", p.path().string().c_str(), NULL, NULL, SW_RESTORE);
						}
						else
						{
							auto path = p.path().parent_path().string();
							ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_RESTORE);
						}
					#endif
						ImGui::CloseCurrentPopup();
					}

					if (ImGui::Button("copy file location"))
					{
						ImGui::SetClipboardText(p.path().string().c_str());
						ImGui::CloseCurrentPopup();
					}

					if (ImGui::Button("copy file location for engine"))
					{
						std::string s = "PIKA_RESOURCES_PATH";
						if (!copyPath.empty())
						{
							s += " \"" + copyPath + "\"";
						}

						ImGui::SetClipboardText(s.c_str());
						ImGui::CloseCurrentPopup();
					}

					if (!p.is_directory())
					{
						if (ImGui::Button("open file"))
						{
						#if PIKA_WINDOWS
							ShellExecuteA(NULL, "open", p.path().string().c_str(), NULL, NULL, SW_RESTORE);
						#endif
							ImGui::CloseCurrentPopup();
						}
					}
				
					{
						auto it = currentDll.containerExtensionsSupport.find(p.path().filename().extension().string());
						if (it != currentDll.containerExtensionsSupport.end())
						{
							if (it->second.size() == 1)
							{
								//todo name						
								if (ImGui::Button("Open In engine"))
								{
									containerManager.createContainer(it->second[0], currentDll, logManager, imguiIDsManager, consoleWindow, p.path().string());
									ImGui::CloseCurrentPopup();
								}
							}
							else
							{
								if (ImGui::BeginMenu("Open with:"))
								{

									for (auto &i : it->second)
									{
										if(ImGui::MenuItem(i.c_str()))
										{
											containerManager.createContainer(i, currentDll, logManager, imguiIDsManager, consoleWindow, p.path().string());
											ImGui::CloseCurrentPopup();
										}
									}

									ImGui::EndMenu();
								}

							}

							
						}
					}
					

					ImGui::EndPopup();
				}


			}

			ImGui::EndChild();

			return 0;
		};

		if (searchText[0] == '\0')
		{
			for (auto &p : std::filesystem::directory_iterator(currentPath))
			{
				if (displayItem(p))
				{
					break;
				}

				ImGui::NextColumn();

			}
		}
		else
		{
			//search filter
			for (auto &p : std::filesystem::recursive_directory_iterator(PIKA_RESOURCES_PATH))
			{
				if (p.is_regular_file())
				{
					auto rez = p.path().filename().string();

					if (rez.find(searchText) != std::string::npos)
					{
						if (displayItem(p))
						{
							break;
						}

						ImGui::NextColumn();
					}

				}
			}
		}

		

		ImGui::Columns(1);

		ImGui::End();
		ImGui::PopID();
	}


};


#endif