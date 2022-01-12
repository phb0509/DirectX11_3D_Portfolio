#pragma once

class Device : public Singleton<Device>
{
	
public:
	void CreateDeviceAndSwapchain();
	void CreateBackBuffer();

	void SetRenderTarget();
	void Clear(Float4 color = Float4(0.0f, 0.125f, 0.3f, 1.0f));
	void Present();

	ID3D11Device* GetDevice() { return mDevice; }
	ID3D11DeviceContext* GetDeviceContext() { return deviceContext; }
	IDXGISwapChain* GetSwapChain() { return swapChain; }
	ID3D11ShaderResourceView*& GetDepthSRV() { return mDSVsrv; }


private:
	Device();
	~Device();

private:
	friend class Singleton;

	ID3D11Device* mDevice;
	ID3D11DeviceContext* deviceContext;

	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* mDSVtexture;
	ID3D11ShaderResourceView* mDSVsrv;
	bool mbIsStencil;
};