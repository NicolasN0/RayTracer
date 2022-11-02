//External includes
#include "SDL.h"
#include "SDL_surface.h"
//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <ppl.h> //parallel_for
#include <future> //async
//#define ASYNC
#define PARALLEL_FOR
using namespace dae;


Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	//const Matrix cameraToWorld = camera.CalculateCameraToWorld(); //In other Part?? Double??
	camera.CalculateCameraToWorld();

	//const float fovAngle = camera.fovAngle * TO_RADIANS;
	//const float FOV = tan(fovAngle / 2.f);
	float FOV = tan(camera.fovAngle / 2);

	const float aspectRatio = m_Width / static_cast<float>(m_Height);

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const uint32_t numPixel = m_Width * m_Height;
	
#if defined(ASYNC)
	//async logic
	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_futures{};
	const uint32_t numPixelsPerTask = numPixel / numCores;
	uint32_t numUnassignedPixels = numPixel % numCores;
	uint32_t currPixelIndex = 0;

	for(uint32_t coreId{0}; coreId < numCores; ++coreId)
	{
		uint32_t taskSize = numPixelsPerTask;
		if(numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}

		async_futures.push_back(std::async(std::launch::async, [=, this]
			{
				//Render all pixels for this task (currPixelIndex > currPixelIndex + taskSize)
				const uint32_t pixelIndexEnd = currPixelIndex + taskSize;
				for(uint32_t pixelIndex{currPixelIndex}; pixelIndex < pixelIndexEnd; ++pixelIndex)
				{
					RenderPixel(pScene, pixelIndex, FOV, aspectRatio, camera, lights, materials);
				}
			}));
		currPixelIndex += taskSize;
	}

	//wait for async completion of all task
	for(const std::future<void>& f : async_futures)
	{
		f.wait();
	}
#elif defined(PARALLEL_FOR)
	//parallel for logic
	concurrency::parallel_for(0u, numPixel, [=, this](int i)
		{
			RenderPixel(pScene, i, FOV, aspectRatio, camera, lights, materials);
		});
#else
	//syncrhonous logic (no threading)
	for(uint32_t i{0};i < numPixel; ++i)
	{
		RenderPixel(pScene, i, FOV, aspectRatio, pScene->GetCamera(), lights, materials);
	}
#endif

	//double for loop
	//for (int px{}; px < m_Width; ++px)
	//{
	//	for (int py{}; py < m_Height; ++py)
	//	{
	//		float gradient = px / static_cast<float>(m_Width);
	//		gradient += py / static_cast<float>(m_Width);
	//		gradient /= 2.0f;



	//		float cx = static_cast<float>((((2 * (px + 0.5)) / m_Width) - 1)) * m_AspectRatio * FOV;
	//		float cy = static_cast<float>(1 - (2 * (py + 0.5)) / m_Height) * FOV;
	//		Vector3 rayDirection{ cx,cy,1 };
	//	
	//		Ray viewRay{ camera.origin,camera.CalculateCameraToWorld().TransformVector(rayDirection.Normalized()) };


	//		ColorRGB finalColor{};

	//		HitRecord closestHit{};

	//		


	//		pScene->GetClosestHit(viewRay, closestHit);



	//		if (closestHit.didHit)
	//		{
	//			ColorRGB ergb;
	//			ColorRGB BRDFrgb;
	//			for (auto light : pScene->GetLights())
	//			{
	//				float dot{ Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(light, closestHit.origin).Normalized()) };
	//				if (dot < 0) {

	//					continue;

	//				}
	//				Vector3 direction{ LightUtils::GetDirectionToLight(light,closestHit.origin) };
	//				Ray lightRay{ closestHit.origin + (closestHit.normal * 0.0001f), direction.Normalized(), 0.0001f, direction.Magnitude() };

	//				if(m_ShadowsEnabled)
	//				{
	//					if(pScene->DoesHit(lightRay))
	//					{
	//						std::cout << "huh";
	//						continue;
	//						
	//					}
	//				}

	//				ergb = LightUtils::GetRadiance(light, closestHit.origin);
	//				BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, camera.forward);
	//				//BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit, -lightRay.direction, viewRay.direction.Normalized());

	//				switch(m_CurrentLightingMode)
	//				{
	//				case LightingMode::ObservedArea:
	//					finalColor += ColorRGB(dot, dot, dot);
	//					break;
	//				case LightingMode::Radiance:
	//					finalColor += ergb;
	//					break;
	//				case LightingMode::BRDF:
	//					finalColor += BRDFrgb;
	//					break;
	//				case LightingMode::Combined:
	//					finalColor += ergb * BRDFrgb * dot;
	//					break;
	//				}
	//				//shadow
	//				/*if (m_ShadowsEnabled)
	//				{
	//					if (pScene->DoesHit(lightRay))
	//					{

	//						finalColor *= 0.5f;

	//					}

	//				}*/

	//			}

	//		}

	//		finalColor.MaxToOne();

	//		m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
	//			static_cast<uint8_t>(finalColor.r * 255),
	//			static_cast<uint8_t>(finalColor.g * 255),
	//			static_cast<uint8_t>(finalColor.b * 255));
	//	}
	//}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void dae::Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera,const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	//const Matrix cameraToWorld = camera.CalculateCameraToWorld();


	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	float rx = px + 0.5f;
	float ry = py + 0.5f;

	float cx = static_cast<float>((((2 * (px + 0.5)) / m_Width) - 1)) * m_AspectRatio * fov;
	float cy = static_cast<float>(1 - (2 * (py + 0.5)) / m_Height) * fov;

	//rest of code from double loop

	Vector3 rayDirection{ cx,cy,1 };

	Ray viewRay{ camera.origin,camera.cameraToWorld.TransformVector(rayDirection.Normalized()) };


	ColorRGB finalColor{};

	HitRecord closestHit{};




	pScene->GetClosestHit(viewRay, closestHit);



	if (closestHit.didHit)
	{
		ColorRGB ergb;
		ColorRGB BRDFrgb;
		for (auto light : pScene->GetLights())
		{
			float dot{ Vector3::Dot(closestHit.normal.Normalized(), LightUtils::GetDirectionToLight(light, closestHit.origin).Normalized()) };
			if (dot < 0) {

				continue;

			}
			Vector3 direction{ LightUtils::GetDirectionToLight(light,closestHit.origin) };
			Ray lightRay{ closestHit.origin + (closestHit.normal * 0.0001f), direction.Normalized(), 0.0001f, direction.Magnitude() };

			if (m_ShadowsEnabled)
			{
				if (pScene->DoesHit(lightRay))
				{
					//std::cout << "huh";
					continue;

				}
			}

			ergb = LightUtils::GetRadiance(light, closestHit.origin);
			BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, camera.forward);
			//BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit, -lightRay.direction, viewRay.direction.Normalized());

			switch (m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
				finalColor += ColorRGB(dot, dot, dot);
				break;
			case LightingMode::Radiance:
				finalColor += ergb;
				break;
			case LightingMode::BRDF:
				finalColor += BRDFrgb;
				break;
			case LightingMode::Combined:
				finalColor += ergb * BRDFrgb * dot;
				break;
			}
			//shadow
			/*if (m_ShadowsEnabled)
			{
				if (pScene->DoesHit(lightRay))
				{

					finalColor *= 0.5f;

				}

			}*/

		}

	}

	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));

}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void dae::Renderer::CycleLightingMode()
{
	switch(m_CurrentLightingMode)
	{
	case LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		break;
	case LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		break;
	case LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRDF;
		break;
	case LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::Combined;
		break;
	}

}
