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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float FOV = tan(camera.fovAngle / 2);
	const Matrix cameraToWorld = camera.CalculateCameraToWorld();

	//float screenWidth{ static_cast<float>(m_Width) };
	//float screenHeight{ static_cast<float>(m_Height) };
	//float aspectRatio{ screenWidth / screenHeight };

	/*float fov{ tan(camera.fovAngle / 2) };
	Matrix camToWorld{ camera.CalculateCameraToWorld() };*/

	//size_t lightsSize{ lights.size() };
	//float offset = 0.0001f;

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;



			float cx = static_cast<float>((((2 * (px + 0.5)) / m_Width) - 1)) * m_AspectRatio * FOV;
			float cy = static_cast<float>(1 - (2 * (py + 0.5)) / m_Height) * FOV;
			Vector3 rayDirection{ cx,cy,1 };
		
			Ray viewRay{ camera.origin,cameraToWorld.TransformVector(rayDirection.Normalized()) };


			ColorRGB finalColor{};

			HitRecord closestHit{};

			


			pScene->GetClosestHit(viewRay, closestHit);


			//if(closestHit.didHit)
			//{
			//	ColorRGB ergb;
			//	ColorRGB BRDFrgb;
			//	for (auto light : pScene->GetLights())
			//	{
			//		Vector3 direction{ LightUtils::GetDirectionToLight(light,closestHit.origin) };
			//		Ray lightRay{ closestHit.origin + (closestHit.normal * 0.0001f), direction.Normalized(), 0.0001f, direction.Magnitude() };

			//		float dot{ Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(light, closestHit.origin).Normalized()) };
			//		if (dot > 0) {


			//			ergb = LightUtils::GetRadiance(light, closestHit.origin);
			//			BRDFrgb += materials[closestHit.materialIndex]->Shade(closestHit,lightRay.direction,camera.forward);
			//			finalColor += ergb * BRDFrgb * dot;
			//			
			//		}

			//		//shadow
			//		if(m_ShadowsEnabled)
			//		{
			//			if (pScene->DoesHit(lightRay))
			//			{
			//			
			//				finalColor *= 0.5f;

			//			}
			//			
			//		}

			//	}

			//}


			if (closestHit.didHit)
			{
				ColorRGB ergb;
				ColorRGB BRDFrgb;
				for (auto light : pScene->GetLights())
				{
					float dot{ Vector3::Dot(closestHit.normal, LightUtils::GetDirectionToLight(light, closestHit.origin).Normalized()) };
					if (dot < 0) {

						continue;

					}
					Vector3 direction{ LightUtils::GetDirectionToLight(light,closestHit.origin) };
					Ray lightRay{ closestHit.origin + (closestHit.normal * 0.0001f), direction.Normalized(), 0.0001f, direction.Magnitude() };

					if(m_ShadowsEnabled)
					{
						if(pScene->DoesHit(lightRay))
						{
							std::cout << "huh";
							continue;
							
						}
					}

					ergb = LightUtils::GetRadiance(light, closestHit.origin);
					BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, camera.forward);
					//BRDFrgb = materials[closestHit.materialIndex]->Shade(closestHit, -lightRay.direction, viewRay.direction.Normalized());

					switch(m_CurrentLightingMode)
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
			


			//ColorRGB finalColor{ hitray.direction.x,hitray.direction.y,hitray.direction.z };
			//Update Color in Buffer

			finalColor.MaxToOne();


			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
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
