#include "Framework.h"


PlayerStatusBar::PlayerStatusBar() :
    mCurHP(1.0f),
    mMaxHP(1.0f),
    mHPRate(1.0f),
    mCurMP(1.0f),
    mMaxMP(1.0f),
    mMPRate(1.0f),
    mStandScaleOffset(1.0f)
{
	mBackGroundTexture = Texture::Add(L"UI_Resource/Player_Status/Status_BackGround.png");
	mHPBarTexture = Texture::Add(L"UI_Resource/Player_Status/HP_Bar.png");
	mMPBarTexture = Texture::Add(L"UI_Resource/Player_Status/MP_Bar.png");
	mPortraitTexture = Texture::Add(L"UI_Resource/Player_Status/Default_Portrait.png");
	mPortraitBackGroundTexture = Texture::Add(L"UI_Resource/Player_Status/Default_Portrait_BackGround.png");

	float textureWidth = mBackGroundTexture->GetWidth();
	float textureHeight = mBackGroundTexture->GetHeight();
	mStandScaleOffset = 3 / 4.0f * WIN_WIDTH / MAIN_WIN_WIDTH;

	mBackGroundUI = new UIImage(L"Texture");
	mBackGroundUI->SetSRV(mBackGroundTexture->GetSRV());
	mBackGroundUI->SetTag("PlayerStatusBarBackGround");
	mBackGroundUI->mScale = { textureWidth * mStandScaleOffset,textureHeight * mStandScaleOffset ,0.0f };
	
	//mHPBarUI = new UIImage(L"VariableTexture");
	mHPBarUI = new UIImage(L"VariableTexture");
	mHPBarUI->SetSRV(mHPBarTexture->GetSRV());
	mHPBarUI->SetTag("PlayerHPBar");
	textureWidth = mHPBarTexture->GetWidth();
	textureHeight = mHPBarTexture->GetHeight();
	mHPBarUI->mScale = { textureWidth * mStandScaleOffset,textureHeight * mStandScaleOffset ,0.0f };

	//mMPBarUI = new UIImage(L"VariableTexture");
	mMPBarUI = new UIImage(L"VariableTexture");
	mMPBarUI->SetSRV(mMPBarTexture->GetSRV());
	mMPBarUI->SetTag("PlayerMPBar");
	textureWidth = mMPBarTexture->GetWidth();
	textureHeight = mMPBarTexture->GetHeight();
	mMPBarUI->mScale = { textureWidth * mStandScaleOffset,textureHeight * mStandScaleOffset ,0.0f };

	mPortraitUI = new UIImage(L"Texture");
	mPortraitUI->SetSRV(mPortraitTexture->GetSRV());
	mPortraitUI->SetTag("PortraitUI");
	textureWidth = mPortraitTexture->GetWidth();
	textureHeight = mPortraitTexture->GetHeight();
	mPortraitUI->mScale = { textureWidth * mStandScaleOffset, textureHeight * mStandScaleOffset , 0.0f };

	mPortraitBackGroundUI = new UIImage(L"Texture");
	mPortraitBackGroundUI->SetSRV(mPortraitBackGroundTexture->GetSRV());
	mPortraitBackGroundUI->SetTag("PortraitBackGroundUI");
	textureWidth = mPortraitBackGroundTexture->GetWidth();
	textureHeight = mPortraitBackGroundTexture->GetHeight();
	mPortraitBackGroundUI->mScale = { textureWidth * mStandScaleOffset, textureHeight * mStandScaleOffset , 0.0f };

	// 최상위객체 시작지점. 고정값. 고정윈도우크기(1600,900)를 기준으로하는 시작지점.
	mStandPosition.x = 312.75f;
	mStandPosition.y = 110.25f;

	// Offset값 적용.
	mBackGroundUI->mPosition = mStandPosition;
	mHPBarUI->mPosition = { mStandPosition.x + 101.2f * mStandScaleOffset , mStandPosition.y + 42.73f * mStandScaleOffset, 0.0f };
	mMPBarUI->mPosition = { mStandPosition.x + 52.0f * mStandScaleOffset, mStandPosition.y - 13.3f * mStandScaleOffset, 0.0f };
	mPortraitUI->mPosition = { mStandPosition.x - 216.0f * mStandScaleOffset, mStandPosition.y - 2.67f * mStandScaleOffset, 0.0f };
	mPortraitBackGroundUI->mPosition = mPortraitUI->mPosition;
}

PlayerStatusBar::~PlayerStatusBar()
{
	GM->SafeDelete(mBackGroundUI);
	GM->SafeDelete(mHPBarUI);
	GM->SafeDelete(mMPBarUI);
	GM->SafeDelete(mPortraitUI);
}

void PlayerStatusBar::Update()
{
	mHPRate = mCurHP / mMaxHP;
	mHPBarUI->SetWidthRatio(mHPRate);
	mMPBarUI->SetWidthRatio(mMPRate);

	UpdateWorld();
	mBackGroundUI->Update();
	mHPBarUI->Update();
	mMPBarUI->Update();
	mPortraitUI->Update();
	mPortraitBackGroundUI->Update();
	
}

void PlayerStatusBar::Render()
{
	mHPBarUI->Render();
	mMPBarUI->Render();
	mPortraitBackGroundUI->Render();
	mPortraitUI->Render();
	mBackGroundUI->Render();
}

void PlayerStatusBar::PostRender()
{
	mBackGroundUI->PostTransformRender();
	mHPBarUI->PostTransformRender();
	mMPBarUI->PostTransformRender();
	mPortraitUI->PostTransformRender();
	mPortraitBackGroundUI->PostTransformRender();
}

void PlayerStatusBar::Hide()
{
	
}

void PlayerStatusBar::Show()
{
}


