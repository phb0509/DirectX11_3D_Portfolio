#pragma once

class Frustum
{
public:
	Frustum();
	~Frustum();

	void Update();
	void Render();

	bool ContainPoint(Vector3 position);
	bool ContainSphere(Vector3 center, float radius);
	bool ContainBox(Vector3 minBox, Vector3 maxBox);

	void GetPlanes(Float4* cullings);

	void SetView(Matrix view) { mView = view; }

private:
	void setCollider(float colliderRectSize, float distanceToColliderRect);

private:
	Vector4 planes[6];
	TetrahedronCollider* mCollider;

	Matrix mProjection;
	Matrix mView;

	float mColliderRectSize;
	float mDistanceToColliderRect;

	bool mbIsCheck;
};