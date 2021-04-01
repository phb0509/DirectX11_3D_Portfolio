#include "Framework.h"

Environment::Environment()
{
	CreatePerspective();

	samplerState = new SamplerState();
	samplerState->SetState();

	mainCamera = new Camera();
	mainCamera->position = { 0, 5, -5 };	

	lightBuffer = new LightBuffer();
	lightBuffer->Add();

}

Environment::~Environment()
{
	delete projectionBuffer;
	delete lightBuffer;

	delete samplerState;	
}

void Environment::PostRender()
{
	mainCamera->PostRender();
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
	
}

void Environment::Set()
{
	SetViewport();
	SetProjection();
	mainCamera->SetVS(1);
	lightBuffer->SetPSBuffer(0);
}

void Environment::SetProjection()
{	
	projectionBuffer->SetVSBuffer(2);
}

void Environment::SetViewport(UINT width, UINT height)
{
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	DC->RSSetViewports(1, &viewport);
}

Vector3 Environment::GetLightPosition()
{
	Vector3 t;
	t.x = lightBuffer->data.lights[0].position.x;
	t.y = lightBuffer->data.lights[0].position.y;
	t.z = lightBuffer->data.lights[0].position.z;

	return t;
}

void Environment::CreatePerspective()
{	
	projection = XMMatrixPerspectiveFovLH(XM_PIDIV4,
		WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f); // 시야각( pie/4니까 90도?,

	projectionBuffer = new MatrixBuffer();
		
	projectionBuffer->Set(projection);
}
