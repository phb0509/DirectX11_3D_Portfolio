#include "Framework.h"
#include "ColorPickingScene.h"



ColorPickingScene::ColorPickingScene()
{
	srand(static_cast<unsigned int>(time(NULL)));
	Environment::Get()->SetIsEnabledTargetCamera(false); // 월드카메라만 사용.

	// 카메라 설정.
	WORLDCAMERA->mPosition = { 0.0f, 0.0f, -40.0f };
	WORLDCAMERA->mRotation = { 0.0, 0.0, 0.0 };
	WORLDCAMERA->mMoveSpeed = 50.0f;

	mTerrain = new Terrain();
	mMonster = new Mutant();
	mCube = new Cube();
	mBoxCollider = new BoxCollider();
	mSphereCollider = new SphereCollider();
	mCapsuleCollider = new CapsuleCollider();
	
	mBoxCollider->mScale = { 2.0f,2.0f,2.0f };
	mSphereCollider->mScale = { 2.0f,2.0f,2.0f };
	mCapsuleCollider->mScale = { 2.0f,2.0f,2.0f };

	mColliders.push_back(mBoxCollider);
	//mColliders.push_back(mSphereCollider);
	//mColliders.push_back(mCapsuleCollider);

	//Monster->SetTerrain(mTerrain);
	mDepthStencil = new DepthStencil(WIN_WIDTH, WIN_HEIGHT, true); // 깊이값

	mRenderTargetTexture = new RenderTarget(WIN_WIDTH, WIN_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT);
	mRenderTargets[0] = mRenderTargetTexture;

	//mCube->mPosition = { 40.0f,0.0f,0.0f };
	//mCube->mScale = { 10.0f,10.0f,10.0f };

	mBoxCollider->mPosition = { 0.0f,0.0f,0.0f };
	mSphereCollider->mPosition = { 20.0f,0.0f,0.0f };
	mCapsuleCollider->mPosition = { 0.0f,0.0f,0.0f };

	mBoxCollider->mScale = { 30.0f,30.0f,30.0f };

	// Create ComputeShader
	mComputeShader = Shader::AddCS(L"ComputeColorPicking");
	mComputeStructuredBuffer = new ComputeStructuredBuffer(sizeof(ColorPickingOutputBuffer), 1);

	if (mInputBuffer == nullptr)
		mInputBuffer = new ColorPickingInputBuffer();

	mOutputBuffer = new ColorPickingOutputBuffer[1];
}

ColorPickingScene::~ColorPickingScene()
{
}

void ColorPickingScene::Update()
{
	for (Collider* collider : mColliders)
	{
		collider->Update();
	}

	colorPicking();

	if (KEY_DOWN(VK_LBUTTON)) // 마우스클릭시에만 실행. 일단은 굳이 프레임마다할필욘 없으니까. 
	{
		for (Collider* collider : mColliders)
		{
			Vector3 colliderColor = collider->GetHashColor();

			if (mMousePositionColor.IsEqual(colliderColor))
			{
				if (KEY_DOWN(VK_LBUTTON))
				{
					mPickedCollider = collider;
					mPickedCollider->SetColor(Float4(1.0f, 1.0f, 0.0f, 1.0f));
				}
			}

			else
			{
				if (KEY_DOWN(VK_LBUTTON))
				{
					collider->SetColor(Float4(0.0f, 1.0f, 0.0f, 1.0f));
					mPickedCollider = nullptr;
				}
			}
		}
	}

	if (KEY_PRESS('W'))
	{
		mBoxCollider->mPosition.y += 20.0f * DELTA;
	}

	if (KEY_PRESS('S'))
	{
		mBoxCollider->mPosition.y -= 20.0f * DELTA;
	}

	if (KEY_PRESS('A'))
	{
		mBoxCollider->mRotation.y -= 10.0f * DELTA;
	}

	if (KEY_PRESS('D'))
	{
		mBoxCollider->mRotation.y += 10.0f * DELTA;
	}
}

void ColorPickingScene::PreRender()
{
	Environment::Get()->SetPerspectiveProjectionBuffer();

	RenderTarget::Sets(mRenderTargets, 1, mDepthStencil);

	// 컬러피킹용 렌더타겟텍스쳐에 렌더.

	for (Collider* collider : mColliders)
	{
		collider->PreRenderForColorPicking();
	}

	if (mPickedCollider != nullptr)
	{
		mPickedCollider->PreRenderGizmosForColorPicking();
	}
}

void ColorPickingScene::Render()
{
	Environment::Get()->SetPerspectiveProjectionBuffer();

	for (Collider* collider : mColliders)
	{
		collider->GetMaterial()->SetShader(L"Collider");
		collider->Render();
	}

	if (mPickedCollider != nullptr)
	{
		mPickedCollider->RenderGizmos();
	}
}

void ColorPickingScene::PostRender()
{
	Environment::Get()->SetPerspectiveProjectionBuffer();

	ImGui::Begin("Test Window");

	int frame_padding = 0;
	ImVec2 imageButtonSize = ImVec2(400.0f, 400.0f); // 이미지버튼 크기설정.                     
	ImVec2 imageButtonUV0 = ImVec2(0.0f, 0.0f); // 출력할이미지 uv좌표설정.
	ImVec2 imageButtonUV1 = ImVec2(1.0f, 1.0f); // 전체다 출력할거니까 1.
	ImVec4 imageButtonBackGroundColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // ImGuiWindowBackGroundColor.
	ImVec4 imageButtonTintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	// Render to RenderTargetTexture
	ImGui::ImageButton(mRenderTargetTexture->GetSRV(), imageButtonSize, imageButtonUV0, imageButtonUV1, frame_padding, imageButtonBackGroundColor, imageButtonTintColor);

	int32_t mousePositionX = static_cast<int32_t>(MOUSEPOS.x);
	int32_t mousePositionY = static_cast<int32_t>(MOUSEPOS.y);
	Int2 mousePosition = { mousePositionX,mousePositionY };


	Vector3 temp = mBoxCollider->GetHashColor();

	ImGui::InputFloat3("BoxCollider Color", (float*)&temp);
	SpacingRepeatedly(2);

	temp = mBoxCollider->GetGizmosHashColorX();
	ImGui::InputFloat3("Gizmos X HashColor", (float*)&temp);
	SpacingRepeatedly(2);

	temp = mBoxCollider->GetGizmosHashColorY();
	ImGui::InputFloat3("Gizmos Y HashColor Color", (float*)&temp);
	SpacingRepeatedly(2);

	temp = mBoxCollider->GetGizmosHashColorZ();
	ImGui::InputFloat3("Gizmos Z HashColor Color", (float*)&temp);
	SpacingRepeatedly(3);

	temp = mBoxCollider->mPosition;
	ImGui::InputFloat3("BoxCollider Position", (float*)&temp);
	SpacingRepeatedly(2);

	temp = mBoxCollider->mRotation;
	ImGui::InputFloat3("BoxCollider Rotation", (float*)&temp);
	SpacingRepeatedly(2);

	ImGui::InputFloat3("MousePosition Color", (float*)&mMousePositionColor);
	SpacingRepeatedly(2);

	//ImGui::InputInt2("Mouse Position", (int*)&mousePosition);
	//SpacingRepeatedly(2);

	ImGui::End();
}

void ColorPickingScene::colorPicking()
{
	int32_t mousePositionX = static_cast<int32_t>(MOUSEPOS.x);
	int32_t mousePositionY = static_cast<int32_t>(MOUSEPOS.y);

	Int2 mousePosition = { mousePositionX,mousePositionY };
	mMouseScreenUVPosition = { MOUSEPOS.x / WIN_WIDTH, MOUSEPOS.y / WIN_HEIGHT ,0.0f };
	mInputBuffer->data.mouseScreenUVPosition = { mMouseScreenUVPosition.x,mMouseScreenUVPosition.y }; // 마우스좌표 uv값
	mInputBuffer->data.mouseScreenPosition = mousePosition; // 마우스좌표 uv값

	mComputeShader->Set(); // 디바이스에 Set..
	mInputBuffer->SetCSBuffer(1);

	DEVICECONTEXT->CSSetShaderResources(0, 1, &mRenderTargetTexture->GetSRV());
	DEVICECONTEXT->CSSetUnorderedAccessViews(0, 1, &mComputeStructuredBuffer->GetUAV(), nullptr);

	DEVICECONTEXT->Dispatch(1, 1, 1);

	mComputeStructuredBuffer->Copy(mOutputBuffer, sizeof(ColorPickingOutputBuffer)); // GPU에서 계산한거 받아옴. 
	
	mMousePositionColor = mOutputBuffer->color;
}


