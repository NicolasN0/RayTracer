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


		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};
		Vector3 rightLocal{ Vector3::UnitX };

		float totalYaw{0.f};
		float totalPitch{0.f};

		Matrix cameraToWorld{};

		float rotspeed{ 0.005f };

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
			
			
			cameraToWorld = obn; //test to not have to call it in RenderPixel
			return obn;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if(pKeyboardState[SDL_SCANCODE_W])
			{
				
				origin += forward * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				
				origin -= forward * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				
				origin -= rightLocal * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				
				origin += rightLocal * deltaTime;
			}
			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			
		
			//Middle mouse is 2
			//Right mouse is 8


			//MovementMouse
			if (SDL_BUTTON(mouseState) == 1)
			{
				//forward movement
				origin -= forward * float(mouseY) * deltaTime;
				//CameraMovement
				totalYaw -= mouseX * rotspeed;
			}

			if(SDL_BUTTON(mouseState) == 16)
			{
				
				origin.y -=  mouseY * deltaTime;
			}

			//RotationMouse
			if(SDL_BUTTON(mouseState) == 8)
			{
			

				totalYaw -= mouseX * rotspeed ;
				totalPitch -= mouseY * rotspeed ;
			}
			
			
		
			Matrix finalRot = Matrix::CreateRotation(totalPitch, totalYaw, 0);
			forward = finalRot.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			
			rightLocal = finalRot.TransformVector(Vector3::UnitX);
			rightLocal.Normalize();

			
		}
	};
}
