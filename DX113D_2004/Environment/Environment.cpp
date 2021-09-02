#include "Framework.h"

Environment::Environment() :
	mbIsTargetCamera(false),
	mTarget(nullptr),
	mbIsEnabledTargetCamera(true),
	mbIsEnabledWorldCamera(true)
{
	createPerspective();

	mSamplerState = new SamplerState();
	mSamplerState->SetState();

	mTargetCamera = new Camera();
	mTargetCamera->CreateFrustum();
	mWorldCamera = new Camera();

	lightBuffer = new LightBuffer();
	lightBuffer->Add();

	
	/*mSun = new Cube();
	mSun->mScale = { 5.0f,5.0f,5.0f };*/
}

Environment::~Environment()
{
	delete projectionBuffer;
	delete lightBuffer;
	delete mSamplerState;
	//delete mSun;
}

void Environment::PostRender()
{
	//mSun->Update();
	//mSun->Render();

	/*if (lightBuffer->data.lightCount >= 1)
	{
		mSun->mPosition = lightBuffer->data.lights[0].position;
	}*/
	
	ImGui::Text("SelectCamera");

	if (ImGui::Button("TargetCamera"))
	{
		mbIsTargetCamera = true;
		mTargetCamera->SetIsMouseInputing(true); // 월드카메라시점일 때 타겟카메라의 마우스이동량에 따른 회전을 막기위함.
	}

	if (ImGui::Button("WorldCamera"))
	{
		mbIsTargetCamera = false;
		mWorldCamera->mPosition = Vector3(110.0f, 175.0f, -40.0f);
		mWorldCamera->mRotation = Vector3(1.0f, 0.0f, 0.0f);
		mTargetCamera->SetIsMouseInputing(false);
	}

	ImGui::Text("MousePosition : %d, %d", (int)MOUSEPOS.x, (int)MOUSEPOS.y);
	ImGui::Spacing();
	ImGui::Text("PlayerPosition : %.1f, %.1f, %.1f", GM->GetPlayer()->mPosition.x, GM->GetPlayer()->mPosition.y, GM->GetPlayer()->mPosition.z);
	ImGui::Spacing();
	ImGui::Text("TargetCameraPosition : %.1f,  %.1f,  %.1f", mTargetCamera->mPosition.x, mTargetCamera->mPosition.y, mTargetCamera->mPosition.z);
	ImGui::Spacing();
	ImGui::Text("WorldCameraPosition : %.1f,  %.1f,  %.1f", mWorldCamera->mPosition.x, mWorldCamera->mPosition.y, mWorldCamera->mPosition.z);
	ImGui::Spacing();
	ImGui::Text("WorldCameraForward : %.1f,  %.1f,  %.1f", mWorldCamera->Forward().x, mWorldCamera->Forward().y, mWorldCamera->Forward().z);

	ImGui::Spacing();

	showLightInformation();

	mTargetCamera->PostRender();
	mWorldCamera->PostRender();
}

void Environment::showLightInformation()
{
	ImGui::Begin("LightInformation");

	ImGui::Text("LightInfo");
	ImGui::ColorEdit4("AmbientColor", (float*)&lightBuffer->data.ambient);
	ImGui::ColorEdit4("AmbientCeilColor", (float*)&lightBuffer->data.ambientCeil);

	if (ImGui::Button("AddLight"))
	{
		lightBuffer->Add();
	}

	for (UINT i = 0; i < lightBuffer->data.lightCount; i++)
	{
		string name = "Light " + to_string(i);
		if (ImGui::BeginMenu(name.c_str()))
		{
			ImGui::Checkbox("Active", (bool*)&lightBuffer->data.lights[i].active);
			ImGui::SliderInt("Type", (int*)&lightBuffer->data.lights[i].type, 0, 3); // 라이트 타입
			ImGui::SliderFloat3("Direction", (float*)&lightBuffer->data.lights[i].direction, -1, 1); // 라이트 방향
			ImGui::SliderFloat3("LightPosition", (float*)&lightBuffer->data.lights[i].position, -100, 100); // 라이트 위치
			ImGui::SliderFloat("LightRange", (float*)&lightBuffer->data.lights[i].range, 0, 100); // 라이트 크기?
			ImGui::ColorEdit4("LightColor", (float*)&lightBuffer->data.lights[i].color);			// 라이트색깔
			ImGui::SliderFloat("LightInner", (float*)&lightBuffer->data.lights[i].inner, 0, 90);
			ImGui::SliderFloat("LightOuter", (float*)&lightBuffer->data.lights[i].outer, 0, 180);
			ImGui::SliderFloat("LightLength", (float*)&lightBuffer->data.lights[i].length, 0, 180);

			ImGui::EndMenu();
		}
	}

	ImGui::End();
}
void Environment::Set()
{
	SetViewport();
	SetProjection();

	if (mbIsTargetCamera)
	{
		mTargetCamera->SetVS(1);
	}
	else
	{
		mWorldCamera->SetVS(1);
	}

	lightBuffer->SetPSBuffer(0);
}


void Environment::SetViewport(UINT width, UINT height)
{
	mViewPort.Width = (float)width;
	mViewPort.Height = (float)height;
	mViewPort.MinDepth = 0.0f;
	mViewPort.MaxDepth = 1.0f;
	mViewPort.TopLeftX = 0;
	mViewPort.TopLeftY = 0;

	DEVICECONTEXT->RSSetViewports(1, &mViewPort);
}

void Environment::SetTargetToCamera(Transform* target)
{
	this->mTarget = target;
	mTargetCamera->SetTarget(target);
}

void Environment::SetProjection()
{
	projectionBuffer->SetVSBuffer(2);
}

void Environment::createPerspective()
{
	mProjectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, 
		WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f); // 시야각( pie/4니까 90도?,

	//mProjectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4,
	//	WIN_WIDTH / (float)WIN_HEIGHT, 1.0f, 100.0f); // 시야각( pie/4니까 90도?,

	projectionBuffer = new MatrixBuffer();

	projectionBuffer->Set(mProjectionMatrix);
}




Vector3 Environment::GetLightPosition()
{
	Vector3 t;
	t.x = lightBuffer->data.lights[0].position.x;
	t.y = lightBuffer->data.lights[0].position.y;
	t.z = lightBuffer->data.lights[0].position.z;

	return t;
}