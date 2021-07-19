#include "Framework.h"
#include "DeferredRenderingScene.h"

DeferredRenderingScene::DeferredRenderingScene()
{
	CreateModels();

	gBuffer = new GBuffer();

	material = new Material(L"DeferredLighting");

	CreateMesh();
}

DeferredRenderingScene::~DeferredRenderingScene()
{
	delete bunny;
	delete plane;
	delete groot;
	delete sphere;

	delete gBuffer;

	delete material;
	delete vertexBuffer;
}

void DeferredRenderingScene::Update()
{
	groot->Update();
	sphere->Update();
}

void DeferredRenderingScene::PreRender()
{
	gBuffer->PreRender(); // 여기서 OM에 Set. (rtv 3개, dev 1개)

	bunny->GetModel()->SetShader(L"GBuffer");
	groot->SetShader(L"GBuffer");
	sphere->GetMaterial()->SetShader(L"GBuffer");
	plane->GetModel()->SetShader(L"GBuffer");
	// 디퍼드라이팅셰이더에서 쓸 텍스쳐들 4개 생성.
	// GBuffer의 픽셀셰이더에서 OM의 렌더타겟에 float4형태로 리턴시켜놨음.


	bunny->Render();
	groot->Render();
	sphere->Render();
	plane->Render();
}

void DeferredRenderingScene::Render()
{
	CAMERA->GetViewBuffer()->SetPSBuffer(3); // 카메라 뷰버퍼 PS 3번에 셋
	Environment::Get()->GetProjectionBuffer()->SetPSBuffer(2);

	gBuffer->Render(); // GBuffer에서 만들어서 OM에 셋팅해놨던 RTV들의 SRV를 픽셀셰이더에 Set. 디퍼드라이팅셰이더에서 사용할거임.

	vertexBuffer->IASet();
	DEVICECONTEXT->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	material->Set(); // 디퍼드라이팅셰이더파일 Set. 이거 Set하고 Draw했으니 딱 맞다.

	DEVICECONTEXT->Draw(4, 0);
}

void DeferredRenderingScene::PostRender()
{
	gBuffer->PostRender(); // UIImage들 렌더.
}

void DeferredRenderingScene::CreateModels()
{
	CAMERA->mPosition = { 0, 10, -20 };
	CAMERA->mRotation.x = 0.4f;

	bunny = new ModelObject("StanfordBunny/StanfordBunny", Collider::NONE);
	bunny->mScale = { 0.01f, 0.01f, 0.01f };
	bunny->mPosition.y = 2.35f;
	bunny->mPosition.x = -3.0f;

	plane = new ModelObject("Plane/Plane", Collider::NONE);
	plane->mScale = { 2.0f, 2.0f, 2.0f };

	groot = new ModelAnimObject("Groot/Groot");
	groot->mScale = { 0.01f, 0.01f, 0.01f };
	groot->mPosition.x = 3.0f;
	groot->SetShader(L"Lighting");
	groot->ReadClip("Groot/Dancing0");

	groot->SetDiffuseMap(L"ModelData/Materials/Groot/groot_diffuse.png");
	groot->SetSpecularMap(L"ModelData/Materials/Groot/groot_glossiness.png");
	groot->SetNormalMap(L"ModelData/Materials/Groot/groot_normal_map.png");

	groot->PlayClip(0);

	sphere = new Sphere(L"Lighting");
	sphere->mPosition = { 0.0f, 1.0f, -3.0f };
	sphere->GetMaterial()->SetDiffuseMap(L"Textures/Landscape/Fieldstone_DM.tga");
	sphere->GetMaterial()->SetNormalMap(L"Textures/Landscape/fieldstone_NM.tga");
}

void DeferredRenderingScene::CreateMesh()
{
	UINT vertices[4] = { 0, 1, 2, 3 };
	vertexBuffer = new VertexBuffer(vertices, sizeof(UINT), 4);
}
