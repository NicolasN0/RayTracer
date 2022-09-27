#pragma once
#include <cassert>
#include <iostream>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <SDL.h>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		//Vector3 forward{0.266f,-0.453f,0.860f };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		float rotspeed{ 0.01f };

		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			Vector3 right = Vector3::Cross(up, forward).Normalized();
			Vector4 rightMatrix{ right,0 };
			Vector3 up = Vector3::Cross(forward, right).Normalized();
			Vector4 upMatrix{ up,0 };
			Vector4 forwardMatrix{ forward,0 };
			Vector4 positionMatrix{ origin,1 };

			Matrix obn {rightMatrix,upMatrix,forwardMatrix,positionMatrix};
			
			//assert(false && "Not Implemented Yet");
			return obn;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if(pKeyboardState[SDL_SCANCODE_W])
			{
				origin.z += 1 * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin.z -= 1 * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin.x -= 1 * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin.x += 1 * deltaTime;
			}
			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

		
			//todo: W2
			if(SDL_BUTTON(mouseState) == 1)
			{
				/*totalPitch -= mouseY * rotspeed * deltaTime;
				totalYaw -= mouseX * rotspeed * deltaTime;*/
				totalPitch -= mouseX * rotspeed * deltaTime;
				totalYaw -= mouseY * rotspeed * deltaTime;
			}
			/*totalPitch -= 0.05f * deltaTime;
			totalYaw -= 0.05f * deltaTime;*/
			

			Matrix finalRot = Matrix::CreateRotation(totalPitch, totalYaw, 0);
			forward = finalRot.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			//Matrix finalRot = forward*
		}
	};
}
