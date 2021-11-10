#include "Framework.h"
#include "ColliderSettingScene.h"

void exportFBX()
{
	ModelExporter* exporter = new ModelExporter("ModelData/Mutant.fbx");
	exporter->ExportMaterial("Mutant/Mutant");
	exporter->ExportMesh("Mutant/Mutant");
}


ColliderSettingScene::ColliderSettingScene() :
	mModel(nullptr),
	mCurrentModel(nullptr),
	mCurrentModelIndex(0),
	mCurrentModelName(""),
	mBeforeModelIndex(0),
	mExtractor(nullptr)
{

	// 카메라 위치설정.
	TARGETCAMERA->mPosition = { -9.4f, 15.5f, -14.8f };
	TARGETCAMERA->mRotation = { 0.3f, 0.7f, 0.0f };

	igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", 0, ".");
	mProjectPath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
	igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");

	// 셋팅할 모델들
	mModelList = {};
	TARGETCAMERA->moveSpeed = 70.0f;

	mExtensionPreviewImages["mesh"] = Texture::Add(L"ModelData/Mesh_PreviewImage.png");
	mExtensionPreviewImages["clip"] = Texture::Add(L"ModelData/Clip_PreviewImage.png");
	mExtensionPreviewImages["mat"] = Texture::Add(L"ModelData/Material_PreviewImage.png");
	mExtensionPreviewImages["fbx"] = Texture::Add(L"ModelData/FBX_PreviewImage.png");


	//// mModelList뿐만 아니라 mModels도 일단 비워놓고, AddModel할때마다 추가해줘야한다.
	//for (int i = 0; i < mModelList.size(); i++)
	//{
	//	mModel = new ToolModel(mModelList[i]);
	//	mModels.push_back(mModel);
	//}


	//// 모델들에 기본적으로 TPose 셋팅.
	//for (int i = 0; i < mModelList.size(); i++)
	//{
	//	mClipsMap[mModelList[i]].push_back("TPose0.clip"); // 씬에서 별개로 각 모델이 어떤 클립을 갖고있는지 갱신해줘야됨. 
	//												// 클립 추가할때마다 메모리해제했다가 새로할당받기 때문에 각 모델의 clips가 초기화됨.
	//	mModels[i]->clips.push_back("TPose0.clip");
	//	mModels[i]->ReadClip(mModelList[i] + "/TPose0.clip");
	//}

}

ColliderSettingScene::~ColliderSettingScene()
{
}

void ColliderSettingScene::Update()
{
	if (mModels.size() != 0) // 메쉬드래그드랍으로 ToolModel할당전까진 업데이트X.
	{
		mCurrentModel = mModels[mCurrentModelIndex];
		mCurrentModelName = mModelList[mCurrentModelIndex];

		mCurrentModel->SetAnimation(mCurrentModel->currentClipIndex);
		mCurrentModel->Update();

		for (auto it = mNodeCollidersMap.begin(); it != mNodeCollidersMap.end(); it++)
		{
			Matrix matrix;

			matrix = mCurrentModel->GetTransformByNode(it->first) * (*(mCurrentModel->GetWorld()));

			it->second.collider->SetParent(&matrix);

			it->second.collider->Update();
		}
	}
}


void ColliderSettingScene::PreRender()
{
}

void ColliderSettingScene::Render()
{
	if (mModels.size() != 0) // 메쉬드래그드랍으로 ToolModel할당전까진 렌더X.
	{
		mCurrentModel->Render();

		for (auto it = mNodeCollidersMap.begin(); it != mNodeCollidersMap.end(); it++) // 현재모델 셋팅한 컬라이더 렌더.
		{
			it->second.collider->Render();
			//it->second->RenderAxis();
		}
	}
}

void ColliderSettingScene::PostRender()
{
	showModelSelect();

	if (mModels.size() != 0) // 이건 ToolModel이 반드시 있어야함...
	{
		showModelHierarchy();
		showColliderEditor();
	}

	if (mModelList.size() != 0)
	{
		showAssets();
	}
}




void ColliderSettingScene::showModelSelect()
{
	ImGui::Begin("SelectModel");
	ImGuiWindowFlags CollapsingHeader_flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
	ImGuiTreeNodeFlags TreeNodeEx_flags = ImGuiTreeNodeFlags_None;

	selectModel();
	//SelectClip();

	ImGui::End();
}


void ColliderSettingScene::selectModel() // perFrame
{
	for (int i = 0; i < mModelList.size(); i++) // ImGui 리스트 갱신.
	{
		mModelTypes[i] = mModelList[i].c_str();
	}

	ImGui::Combo("Models", (int*)&mCurrentModelIndex, mModelTypes, mModelList.size());

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	showAddButton();

	if (mModels.size() != 0)
	{
		// 모델 변경여부와 상관없이 계속 초기화시켜줘야하는 곳.

		for (auto it = mPreprocessedNodes.begin(); it != mPreprocessedNodes.end(); it++) // 전처리된 노드맵 초기화. 
		{
			it->second.clear();
		}

		// 모델이 바뀔때만 초기화해주면 되는곳.

		if (mCurrentModelIndex != mBeforeModelIndex)
		{
			for (auto it = mCreatedCollidersCheck.begin(); it != mCreatedCollidersCheck.end(); it++) // 노드콜라이더 생성여부체크맵 초기화.
			{
				it->second = false;
			}

			for (auto it = mNodeCollidersMap.begin(); it != mNodeCollidersMap.end(); it++) // 모델의 생성됐던 콜라이더들 초기화.
			{
				delete it->second.collider;
			}
			mNodeCollidersMap.clear();

			for (auto it = mNodeNameMap.begin(); it != mNodeNameMap.end(); it++)
			{
				it->second = "";
			}

			mBeforeModelIndex = mCurrentModelIndex;
		}

		treeNodePreProcessing();
	}

}

void ColliderSettingScene::treeNodePreProcessing() // 부모에 어떤 노드index가 자식으로있는지 세팅.
{
	mNodes = mCurrentModel->GetNodes();

	for (int i = 0; i < mNodes.size(); i++)
	{
		int parent = mNodes[i]->index; // 맨 처음 인덱스 부모로잡고.

		for (int j = 0; j < mNodes.size(); j++)
		{
			if (mNodes[j]->parent == parent)
			{
				if (mNodes[j]->index == parent)
				{
					continue;
				}

				mPreprocessedNodes[parent].push_back(mNodes[j]->index);
			}
		}
	}
}

void ColliderSettingScene::showAddButton()
{
	if (ImGui::Button("AddModel.."))
		ImGui::OpenPopup("AddModel");

	ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("AddModel", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char inputText[128] = "";
		ImGui::InputText("Input ModelName", inputText, IM_ARRAYSIZE(inputText));

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			// ModelData폴더에 InputText에 작성한 값 그대로 폴더 생성. 

			string savePath = "ModelData/";
			string name;

			int i = 0;

			while (inputText[i] != '\0')
			{
				name += inputText[i];
				i++;
			}

			mModelList.push_back(name);
			//mModels.push_back(new ToolModel(name));
			mCurrentModelIndex = mModels.size() - 1;
			//mCurrentModel = mModels[mCurrentModelIndex];
			mCurrentModelName = name;

			//mCurrentModel->ReadTPoseClip();

			savePath += name;
			savePath += '/';
			CreateFolders(savePath);

			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			// Cancel 이벤트는 딱히 넣을게없다.
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void ColliderSettingScene::selectClip()
{
	if (ImGui::Button("SelectClip"))
		igfd::ImGuiFileDialog::Instance()->OpenDialog("TextureKey", "Choose Clip", ".clip", ".");

	// display
	if (igfd::ImGuiFileDialog::Instance()->FileDialog("TextureKey"))
	{
		if (igfd::ImGuiFileDialog::Instance()->IsOk == true)
		{
			mPath = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
			mPath = mPath.substr(mProjectPath.size() + 1, mPath.length());
			Replace(&mPath, "\\", "/");
			mPath = mPath.substr(17 + mCurrentModelName.size(), mPath.length());

			mClipsMap[mCurrentModelName].push_back(mPath); // "SmashAttack0.clip" push_back

			int t_currentClipIndex = mCurrentModel->currentClipIndex;

			delete mModels[mCurrentModelIndex];
			mModel = new ToolModel(mCurrentModelName);
			mModel->currentClipIndex = t_currentClipIndex;

			for (int i = 0; i < mClipsMap[mCurrentModelName].size(); i++) // 클립추가할때마다 새로 할당받은 후 현재모델의 클립을 전부 새로 리드클립.
			{
				string t = mCurrentModelName + "/" + mClipsMap[mCurrentModelName][i];
				mModel->ReadClip(t); // 새로 할당받은 model에 씬에서 자체적으로 관리하는 클립리스트 내용대로 ReadClip
			}

			mModels[mCurrentModelIndex] = mModel; // 새로 할당받은 모델 다시 원래 인덱스에 넣어놓기.
		}

		// close
		igfd::ImGuiFileDialog::Instance()->CloseDialog("TextureKey");
	}

	for (int i = 0; i < mClipsMap[mCurrentModelName].size(); i++) // imgui의 클립리스트를 clips의 내용물로 계속 갱신.
	{
		mClipTypes[i] = mClipsMap[mCurrentModelName][i].c_str();
	}

	ImGui::Combo("Clips", (int*)&mCurrentModel->currentClipIndex, mClipTypes, mClipsMap[mCurrentModelName].size());
}


void ColliderSettingScene::showModelHierarchy()
{
	ImGui::Begin("Hierachy");
	ImGuiWindowFlags CollapsingHeader_flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
	ImGuiTreeNodeFlags TreeNodeEx_flags = ImGuiTreeNodeFlags_None;

	if (ImGui::TreeNodeEx("ModelBone", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Selected))
	{
		ImGui::TreePop();

		treeNodeRecurs(0);
	}

	for (auto it = mNodeCheck.begin(); it != mNodeCheck.end(); ++it) // 노드방문흔적 초기화.
	{
		it->second = false;
	}

	ImGui::End();
}

void ColliderSettingScene::treeNodeRecurs(int nodesIndex)
{
	if (mNodeCheck[mNodes[nodesIndex]->index])
	{
		return;
	}

	ImGui::Indent();
	mNodeCheck[mNodes[nodesIndex]->index] = true;
	mNodeNameMap[mNodes[nodesIndex]->index] = mNodes[nodesIndex]->name;


	if (ImGui::TreeNodeEx(mNodes[nodesIndex]->name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) // 노드이름 출력.
	{
		ImGui::SameLine();

		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_OpenOnArrow))
		{
			const char* colliderTypes[3] = { "SphereCollider","BoxCollider","CapsuleCollider" };
			ImGui::Combo("Colliders", (int*)&mCurrentColliderIndex, colliderTypes, 3);
			ImGui::Text("");


			if (ImGui::Button("CreateCollider"))
			{
				if (mCreatedCollidersCheck[mNodes[nodesIndex]->index]) {} // 이미 만들어져있으면 넘기기
				else // 안만들어져있으면 만들어주기.
				{
					mCreatedCollidersCheck[mNodes[nodesIndex]->index] = true;

					switch (mCurrentColliderIndex)
					{
					case 0:
						mNodeCollider = new SphereCollider();
						break;

					case 1:
						mNodeCollider = new BoxCollider();
						break;

					case 2:
						mNodeCollider = new CapsuleCollider();
						break;

					default:
						break;
					}


					mNodeCollider->mScale = { 10.0f,10.0f,10.0f }; // 생성했을 때 너무 작으면 안보여서 10으로 세팅.

					//mNodeColliders.push_back(mNodeCollider); // 안쓰여서 일단 주석.

					TreeNodeData treeNodeData;
					treeNodeData.collider = mNodeCollider;
					treeNodeData.nodeName = mNodes[nodesIndex]->name; // 노드이름 for binarysave

					mNodeCollidersMap[mNodes[nodesIndex]->index] = treeNodeData;
				}
			}

			ImGui::Text("");
			ImGui::TreePop();
		}

		if (mPreprocessedNodes[mNodes[nodesIndex]->index].size() != 0) // 자식이 있다면.
		{
			for (int i = 0; i < mPreprocessedNodes[mNodes[nodesIndex]->index].size(); i++)
			{
				treeNodeRecurs(mPreprocessedNodes[mNodes[nodesIndex]->index][i]);
			}
		}

		ImGui::TreePop();
	}

	ImGui::Unindent();
}

void ColliderSettingScene::showColliderEditor()
{
	ImGui::Begin("ColliderEditor");
	ImGuiWindowFlags CollapsingHeader_flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow;
	ImGuiTreeNodeFlags TreeNodeEx_flags = ImGuiTreeNodeFlags_None;

	for (auto it = mCreatedCollidersCheck.begin(); it != mCreatedCollidersCheck.end(); it++)
	{
		if (it->second)
		{
			string labelName = mNodeNameMap[it->first];
			string deleteName = "Delete " + mNodeNameMap[it->first];
			string Position = labelName + " Position";
			string Rotation = labelName + " Rotation";
			string Scale = labelName + " Scale";

			string tagName = labelName + " Collider";

			ImGui::Text(labelName.c_str());

			if (ImGui::Button(deleteName.c_str()))
			{
				it->second = false;
				delete mNodeCollidersMap[it->first].collider;
				mNodeCollidersMap.erase(it->first);
				continue;
			}

			ImGui::InputText(tagName.c_str(), mColliderNameMap[it->first], 100);

			ImGui::Text("");

			ImGui::InputFloat3(Position.c_str(), (float*)&mNodeCollidersMap[it->first].collider->mPosition);
			ImGui::InputFloat3(Rotation.c_str(), (float*)&mNodeCollidersMap[it->first].collider->mRotation);
			ImGui::InputFloat3(Scale.c_str(), (float*)&mNodeCollidersMap[it->first].collider->mScale);
		}
	}


	if (ImGui::Button("Save"))
	{
		save();
	}

	ImGui::End();
}



void ColliderSettingScene::save()
{
	string path = "TextData/";
	string name = mCurrentModelName;

	wstring t = ToWString(path + name + ".map");

	BinaryWriter colliderWriter(t);

	vector<ColliderDataForSave> colliderDatas;

	int tSize = 0;

	for (auto it = mCreatedCollidersCheck.begin(); it != mCreatedCollidersCheck.end(); it++)
	{
		if (it->second)
		{
			tSize++;
		}
	}

	colliderWriter.UInt(tSize); // 저장할 컬라이더개수.

	for (auto it = mCreatedCollidersCheck.begin(); it != mCreatedCollidersCheck.end(); it++)
	{
		if (it->second)
		{
			colliderWriter.String(mColliderNameMap[it->first]);
			colliderWriter.String(mNodeCollidersMap[it->first].nodeName);

			ColliderDataForSave data;
			data.position = mNodeCollidersMap[it->first].collider->mPosition;
			data.rotation = mNodeCollidersMap[it->first].collider->mRotation;
			data.scale = mNodeCollidersMap[it->first].collider->mScale;

			colliderDatas.push_back(data);
		}
	}

	colliderWriter.Byte(colliderDatas.data(), sizeof(ColliderDataForSave) * colliderDatas.size());

	printToCSV(); // 보기쉽게 CSV로 저장도 해줌.
}

void ColliderSettingScene::LoadFileList(string folderName, vector<string>& fileList)
{
	string path = "C:\\Users\\pok98\\source\\repos\\DirectX11_3D_Portfolio\\DX113D_2004\\ModelData\\";
	path = path += folderName + "\\";
	path += "*.*";

	struct _finddata_t fd;
	intptr_t handle;

	int i = 0;

	if ((handle = _findfirst(path.c_str(), &fd)) == -1L)
	{
		// 파일 없을때 발생시킬 이벤트.
	}
	do
	{
		if (i >= 2)
		{
			fd.name;
			string temp(fd.name);
			fileList.push_back(temp);
			int a = 0;
		}

		i++;
	} while (_findnext(handle, &fd) == 0);

	_findclose(handle);
}

void ColliderSettingScene::showAssets() // ex)ModelData/Mutant내의 모든 assets들.
{
	// 이미 mModelsList.size() 1이상체크하고 들어왔다.

	string tempImGuiName = mCurrentModelName + "Assets";

	ImGui::Begin(tempImGuiName.c_str());

	if (ImGui::Button("Import")) // FBX파일 추출. ExportFBX
	{
		ImGui::OpenPopup("Extractor");
	}

	ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Extractor", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// 옵션 선택.
		ImGui::Checkbox("Export Mesh", &mIsExportMesh);
		ImGui::Checkbox("Export Material", &mIsExportMaterial);
		ImGui::Checkbox("Export Animation", &mIsExportAnimation);

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::Button("Open...")) // 여기부분에서부터 문제가 생기는듯하다.
		{
			string t = openFileDialog();
			thread t1(exportFBX);

			t1.join();
		}


		if (ImGui::Button("OK", ImVec2(120, 0))) // 옵션고르고 추출실행.
		{
			//tempCheck = true;
		}

		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			// Cancel 이벤트는 딱히 넣을게없다.
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}


	// mCurrentModelAssets 표시.(.mat, .mest, .clip 등등)
	vector<string> fileList;
	LoadFileList(mCurrentModelName, fileList); // 확장자까지 포함한 파일명들 리턴.

	for (int i = 0; i < fileList.size(); i++)
	{
		if ((i % 3) != 0)
			ImGui::SameLine();

		int frame_padding = 2;
		ImVec2 size = ImVec2(64.0f, 64.0f); // 이미지버튼 크기설정.                     
		ImVec2 uv0 = ImVec2(0.0f, 0.0f); // 출력할이미지 uv좌표설정.
		ImVec2 uv1 = ImVec2(1.0f, 1.0f); // 전체다 출력할거니까 1.
		ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f); // 바탕색.(Background Color)        
		ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		string fileExtension = GetExtension(fileList[i]);

		if (fileExtension == "png")
		{
			string temp = "ModelData/" + mCurrentModelName + "/" + fileList[i];
			mExtensionPreviewImages[fileExtension] = Texture::Add(ToWString(temp));
		}

		ImGui::ImageButton(mExtensionPreviewImages[fileExtension]->GetSRV(), size, uv0, uv1, frame_padding, bg_col, tint_col);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) // 원본 드래그 이벤트.
		{
			ImGui::SetDragDropPayload("DND_DEMO_CELL", &i, sizeof(int)); // 드래그할 때 인덱스(int값) 정보 가지고있음.
			ImGui::EndDragDropSource();
		}
	}



	ImGui::End();
}



void ColliderSettingScene::printToCSV()
{
	FILE* file;

	string str = "TextData/Saved" + mCurrentModelName + "Colliders.csv";
	const char* fileName = str.c_str();

	fopen_s(&file, fileName, "w");


	fprintf( // 컬럼명
		file,
		"%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
		"ColliderName", "",
		"NodeName", "",
		"Position.x", "Position.y", "Position.z", "",
		"Rotation.x", "Rotation.y", "Rotation.z", "",
		"Scale.x", "Scale.y", "Scale.z"
	);

	fprintf( // 줄바꿈용
		file,
		"\n"
	);

	for (auto it = mCreatedCollidersCheck.begin(); it != mCreatedCollidersCheck.end(); it++)
	{
		if (it->second)
		{
			ColliderDataForSave data;
			data.position = mNodeCollidersMap[it->first].collider->mPosition;
			data.rotation = mNodeCollidersMap[it->first].collider->mRotation;
			data.scale = mNodeCollidersMap[it->first].collider->mScale;

			const char* modelName = mCurrentModelName.c_str();

			string colliderName = mColliderNameMap[it->first];
			string nodeName = mNodeCollidersMap[it->first].nodeName;

			fprintf(
				file,
				"%s,%s,%s,%s,%.3f,%.3f,%.3f,%s, %.3f,%.3f,%.3f,%s, %.3f,%.3f,%.3f\n",
				colliderName.c_str(), "", nodeName.c_str(), "",
				data.position.x, data.position.y, data.position.z, "",
				data.rotation.x, data.rotation.y, data.rotation.z, "",
				data.scale.x, data.scale.y, data.scale.z
			);
		}
	}


	fclose(file);
}

string ColliderSettingScene::openFileDialog()
{
	const COMDLG_FILTERSPEC c_rgSaveTypes[] =
	{
		{L"Word Document (*.doc)",       L"*.doc"},
		{L"Web Page (*.htm; *.html)",    L"*.htm;*.html"},
		{L"Text Document (*.txt)",       L"*.txt"},
		{L"All Documents (*.*)",         L"*.*"}
	};

	string filePath;

	IFileDialog* pfd = NULL;

	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pfd));

	if (SUCCEEDED(hr))
	{
		// Create an event handling object, and hook it up to the dialog.
		IFileDialogEvents* pfde = NULL;
		hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
		if (SUCCEEDED(hr))
		{
			// Hook up the event handler.
			DWORD dwCookie;
			hr = pfd->Advise(pfde, &dwCookie);

			if (SUCCEEDED(hr))
			{
				// Set the options on the dialog.
				DWORD dwFlags;

				// Before setting, always get the options first in order 
				// not to override existing options.
				hr = pfd->GetOptions(&dwFlags);
				if (SUCCEEDED(hr))
				{
					// In this case, get shell items only for file system items.
					hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
					if (SUCCEEDED(hr))
					{
						// Set the file types to display only. 
						// Notice that this is a 1-based array.
						hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes); // 확장자 표시하는곳.
						if (SUCCEEDED(hr))
						{
							// Set the selected file type index to Word Docs for this example.
							hr = pfd->SetFileTypeIndex(INDEX_WORDDOC);
							if (SUCCEEDED(hr))
							{
								// Set the default extension to be ".doc" file.
								hr = pfd->SetDefaultExtension(L"doc;docx");
								if (SUCCEEDED(hr))
								{
									// Show the dialog
									hr = pfd->Show(NULL);
									if (SUCCEEDED(hr))
									{
										// Obtain the result once the user clicks 
										// the 'Open' button.
										// The result is an IShellItem object.
										IShellItem* psiResult;
										hr = pfd->GetResult(&psiResult);
										if (SUCCEEDED(hr))
										{
											// We are just going to print out the 
											// name of the file for sample sake.

											PWSTR pszFilePath = NULL; // 선택된 파일경로.
											hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

											if (SUCCEEDED(hr))
											{
												TaskDialog(NULL,
													NULL,
													L"CommonFileDialogApp", // 파일고르고 확인누르면 뜨는 확인차뜨는 팝업창.
													pszFilePath,
													NULL,
													TDCBF_OK_BUTTON,
													TD_INFORMATION_ICON,
													NULL);
												CoTaskMemFree(pszFilePath);
											}

											filePath = ToString(pszFilePath);

											psiResult->Release();
										}
									}
								}
							}
						}
					}
				}
				// Unhook the event handler.
				pfd->Unadvise(dwCookie);
			}
			pfde->Release();
		}
		pfd->Release();
	}

	return filePath;
}

HRESULT ColliderSettingScene::CDialogEventHandler_CreateInstance(REFIID riid, void** ppv)
{
	*ppv = NULL;
	CDialogEventHandler* pDialogEventHandler = new (std::nothrow) CDialogEventHandler();
	HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;

	if (SUCCEEDED(hr))
	{
		hr = pDialogEventHandler->QueryInterface(riid, ppv);
		pDialogEventHandler->Release();
	}

	return hr;
}


