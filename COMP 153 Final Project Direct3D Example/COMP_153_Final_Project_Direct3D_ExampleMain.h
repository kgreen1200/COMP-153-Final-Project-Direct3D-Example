#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace COMP_153_Final_Project_Direct3D_Example
{
	class COMP_153_Final_Project_Direct3D_ExampleMain : public DX::IDeviceNotify
	{
	public:
		COMP_153_Final_Project_Direct3D_ExampleMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~COMP_153_Final_Project_Direct3D_ExampleMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}