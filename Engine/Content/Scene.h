#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"

namespace Engine
{
	class RenderScene {
	public:
		RenderScene(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		
	private:
		std::shared_ptr<DX::DeviceResources> deviceResources;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>  pixelShader;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>  inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	   constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	   vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>	   indexBuffer;

		ModelViewProjectionConstantBuffer	constantBufferData;

		uint32 indexCount;
		bool loadingComplete;
	};
}