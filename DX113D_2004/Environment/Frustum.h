#pragma once


class Cube;
class EmptyObject;

class Frustum
{
public:
	Frustum(float FoV, float aspectRatio, float distanceToNearZ, float distanceToFarZ);
	~Frustum();

	void Update();
	void RenderCollider();
	void RenderDeferredCollider();
	void PostRender();

	bool ContainPoint(Vector3 position);
	bool ContainSphere(Vector3 center, float radius);
	bool ContainBox(Vector3 minBox, Vector3 maxBox);

	void GetPlanes(Float4* cullings);
	void GetPlanesForTerrainFrustumCulling(Float4* cullings);

	void SetView(Matrix view) { mView = view; }
	void SetCamera(Camera* camera) { mCamera = camera; }

private:
	void setCollider(float colliderRectSize, float distanceToColliderRect);
	void initialize();
	void createCollider();

private:
	Vector4 mPlanes[6];
	FrustumCollider* mCollider;

	Matrix mProjectionMatrix;
	Matrix mView;

	float mColliderRectSize;
	float mDistanceToColliderRect;

	bool mbIsCheck;
	Camera* mCamera;
	bool mbHasInitialized;

	EmptyObject* mEmptyObject;

	float mFoV;
	float mAspectRatio;
	float mDistanceToNearZ;
	float mDistanceToFarZ;

	bool mbIsTerrainFrustumCulling;
};