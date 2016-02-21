#include "pch.h"
#include "Scene.h"

#include "Common/DirectXHelper.h"

using namespace DirectX;
using namespace Windows::Foundation;

Engine::RenderScene::RenderScene(const std::shared_ptr<DX::DeviceResources>& deviceResources):
	deviceResources(deviceResources),
	loadingComplete(false)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();

}

void Engine::RenderScene::CreateDeviceDependentResources()
{
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&vertexShader
				)
			);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&inputLayout
				)
			);
	});
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&constantBuffer
				)
			);
	});

	auto createTriangleTask = (createPSTask && createVSTask).then([this]() {


		XMFLOAT3 triangleVertices[] =
		{
			{-1.0f, 0.0f, 0.5f},
			{0.0f, 1.0f, 0.5f },
			{1.0f, 0.0f, 0.5f }
		};

		unsigned short triangleIndices[] =
		{
			0, 1, 2,
		};

		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(XMFLOAT3) * ARRAYSIZE(triangleVertices);
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		vertexBufferData.pSysMem = triangleVertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&vertexBuffer
				)
			);


		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.ByteWidth = sizeof(unsigned short) * ARRAYSIZE(triangleIndices);
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		indexBufferData.pSysMem = triangleIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

		indexCount =  ARRAYSIZE(triangleIndices);

		DX::ThrowIfFailed(
			deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&indexBuffer
				)
			);

	});

	createTriangleTask.then([this]()
	{
		loadingComplete = true;
	});
}

void Engine::RenderScene::CreateWindowSizeDependentResources()
{
	Size outputSize = deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// Это простой пример изменения, которое можно реализовать, когда приложение находится в
	// книжном или прикрепленном представлении.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Обратите внимание, что матрица OrientationTransform3D подвергается здесь дополнительному умножению,
	// чтобы правильно сориентировать сцену в соответствии с ориентацией экрана.
	// Это умножение является обязательным для всех вызовов draw,
	// предназначенных для целевого объекта в цепочке буферов. В случае вызовов draw для других целевых объектов
	// это преобразование не должно применяться.

	// В этом примере используется правая система координат на базе матриц с развертыванием по столбцам.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// Наблюдатель находится в точке (0,0.7,1.5) и смотрит в точку (0,-0.1,0), а вектор вертикали направлен вдоль оси Y.
	static const XMVECTORF32 eye = { -1.0f, -1.0f, -1.0f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

void Engine::RenderScene::ReleaseDeviceDependentResources()
{
	loadingComplete = false;
	vertexShader.Reset();
	inputLayout.Reset();
	pixelShader.Reset();
	constantBuffer.Reset();
	vertexBuffer.Reset();
	indexBuffer.Reset();
}

void Engine::RenderScene::Update(DX::StepTimer const & timer)
{
}

void Engine::RenderScene::Render()
{
	// Загрузка является асинхронной. Геометрию можно рисовать только после ее загрузки.
	if (!loadingComplete)
	{
		return;
	}

	auto context = deviceResources->GetD3DDeviceContext();

	context->IASetInputLayout(inputLayout.Get());

	// Каждая вершина представляет собой экземпляр структуры VertexPositionColor.
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Каждый индекс представляет собой 16-разрядное целое число без знаков (короткое).
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Прикрепление шейдера вершин.
	context->VSSetShader(
		vertexShader.Get(),
		nullptr,
		0
		);

	// Прикрепление шейдера пикселей.
	context->PSSetShader(
		pixelShader.Get(),
		nullptr,
		0
		);

	// Рисование объектов.
	context->DrawIndexed(
		indexCount,
		0,
		0
		);
}
