#include "Framework.h"

string utility::ToString(wstring value)
{
	string temp;
	temp.assign(value.begin(), value.end());
	return temp;
}

wstring utility::ToWString(string value)
{
	wstring temp;
	temp.assign(value.begin(), value.end());
	return temp;
}

void utility::Replace(string* str, string comp, string rep)
{
	string temp = *str;

	size_t at = 0;
	while ((at = temp.find(comp, at)) != string::npos)
	{
		temp.replace(at, comp.length(), rep);
		at += rep.length();
	}

	*str = temp;
}

vector<string> utility::SplitString(string origin, string tok)
{
	vector<string> result;

	size_t cutAt = 0;

	while ((cutAt = origin.find_first_of(tok)) != origin.npos)
	{
		if (cutAt > 0)
			result.emplace_back(origin.substr(0, cutAt));

		origin = origin.substr(cutAt + 1);
	}

	return result;
}

wstring utility::GetExtension(wstring path)
{
	size_t index = path.find_last_of('.');

	return path.substr(index + 1, path.length());
}

string utility::GetExtension(string path)
{
	size_t index = path.find_last_of('.');

	return path.substr(index + 1, path.length());
}

string utility::GetDirectoryName(string path)
{
	Replace(&path, "\\", "/");

	size_t index = path.find_last_of('/');

	return path.substr(0, index + 1);
}

string utility::GetFileName(string path)
{
	Replace(&path, "\\", "/");

	size_t index = path.find_last_of('/');

	return path.substr(index + 1, path.length());
}

string utility::GetFileNameWithoutExtension(string path)
{
	string fileName = GetFileName(path);

	size_t index = fileName.find_last_of('.');

	return fileName.substr(0, index);
}

string utility::OpenFileDialog()
{
	const COMDLG_FILTERSPEC c_rgSaveTypes[] =
	{
		{L"FBX File (*.fbx)",       L"*.fbx"},
		{L"Text File (*.txt)",       L"*.txt"},
		{L"All File (*.*)",         L"*.*"}
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
													///L"CommonFileDialogApp", // 파일고르고 확인누르면 뜨는 확인차뜨는 팝업창.
													NULL,
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


char* utility::ToCharPointer(string string)
{
	vector<char> writable(string.begin(), string.end());
	writable.push_back('\0');
	char* ptr = &writable[0];

	return ptr;
}

void utility::CreateFolders(string path)
{
	vector<string> folders = SplitString(path, "/");

	string temp = "";
	for (string folder : folders)
	{
		temp += folder + "/";

		if (!ExistDirectory(temp))
			CreateDirectoryA(temp.c_str(), 0);
	}
}

void utility::IndentRepeatedly(int loopCount)
{
	for (int i = 0; i < loopCount; i++)
	{
		ImGui::Indent();
	}
}

void utility::UnIndentRepeatedly(int loopCount)
{
	for (int i = 0; i < loopCount; i++)
	{
		ImGui::Unindent();
	}
}

void utility::SpacingRepeatedly(int loopCount)
{
	for (int i = 0; i < loopCount; i++)
	{
		ImGui::Spacing();
	}
}

bool utility::ExistDirectory(string path)
{
	DWORD fileValue = GetFileAttributesA(path.c_str());

	BOOL temp = (fileValue != INVALID_FILE_ATTRIBUTES &&
		(fileValue & FILE_ATTRIBUTE_DIRECTORY));

	return temp == TRUE;
}

bool utility::ExistFile(string path)
{
	DWORD fileValue = GetFileAttributesA(path.c_str());

	return fileValue < 0xffffffff;
}

HRESULT utility::CDialogEventHandler_CreateInstance(REFIID riid, void** ppv)
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

