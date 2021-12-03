#pragma once

class Terrain : public Transform
{
private:
	typedef VertexUVNormalTangent VertexType;

	struct InputDesc
	{
		UINT index;
		Float3 v0, v1, v2;
	};

	struct OutputDesc
	{
		int picked;
		float u, v, distance;
	};


public:
	Terrain();
	~Terrain();

	void Update();	
	void Render();
	
	bool Picking(OUT Vector3* position);
	bool ComputePicking(OUT Vector3* position);

	float GetHeight(Vector3 position);

	Float2 GetSize() { return Float2(mWidth, mHeight); }
private:
	void CreateMesh();
	void CreateNormal();
	void CreateTangent();




private:
	Material* mMaterial;
	Mesh* mMesh;

	vector<VertexType> mVertices;
	vector<UINT> mIndices;

	UINT mWidth;
	UINT mHeight;

	Texture* mHeightMap;

	RasterizerState* mFillMode[2];

	ComputeShader* mComputeShader;
	RayBuffer* mRayBuffer;
	ComputeStructuredBuffer* mStructuredBuffer;
	InputDesc* mInput;
	OutputDesc* mOutput;

	TypeBuffer* mTypeBuffer;

	UINT mPolygonCount;
};