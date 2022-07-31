#pragma once

class UIImage : public Transform
{
public:
	UIImage(wstring shaderFile);
	virtual ~UIImage();

	virtual void Update();
	virtual void Render();
	
	void SetSRV(ID3D11ShaderResourceView* srv) { this->mSRV = srv; }
	void SetWidthRatio(float widthRatio) { mWidthRatio = widthRatio; }
	void SetHeightRatio(float heightRatio) { mHeightRatio = heightRatio; }

private:
	void createMesh();
	void createViewBuffer();

protected:
	Material* mMaterial;
	Mesh* mMesh;

	ID3D11ShaderResourceView* mSRV;

	Matrix mViewMatrix;
	Matrix mOrthographicMatrix;

	ViewBuffer* mViewBuffer;
	ProjectionBuffer* mProjectionBuffer;
	TextureBuffer* mTextureBuffer;

	vector<BlendState*> mBlendStates;
	vector<DepthStencilState*> mDepthModes;

	float mWidthRatio;
	float mHeightRatio;
};