#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\RenderFPS.h"

// Прорисовывает содержимое Direct2D и 3D на экране.
namespace Engine
{
	class EngineMain : public DX::IDeviceNotify
	{
	public:
		EngineMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~EngineMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Кэшированный указатель на ресурсы устройства.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		std::unique_ptr<RenderFPS> m_fpsTextRenderer;

		// Таймер цикла прорисовки.
		DX::StepTimer m_timer;
	};
}