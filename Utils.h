#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

		

			Vector3 TC{ sphere.origin - ray.origin };
			float dp{ Vector3::Dot(TC, ray.direction) };
			float tclSquare{ TC.SqrMagnitude() };
			float odSquare{ tclSquare - Square(dp) };

			float radiusSquared{ Square(sphere.radius) };
			if (odSquare > radiusSquared) {
				return false;
			}
			float tca{ sqrtf(radiusSquared - odSquare) };

			float t{ dp - tca };
			if (t > ray.min && t < ray.max) {
				if (ignoreHitRecord) return true;
				Vector3 I{ ray.origin + t * ray.direction };
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.t = t;
				hitRecord.origin = I;
				hitRecord.normal = (I - sphere.origin).Normalized();
				return true;

			}

			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			float t = Vector3::Dot((plane.origin - ray.origin) , plane.normal) / Vector3::Dot(ray.direction , plane.normal);
			if(t > ray.min && t < ray.max)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.normal = plane.normal;
				hitRecord.origin = ray.origin + t * ray.direction;
				hitRecord.materialIndex = plane.materialIndex;
				return true;
			}
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			Vector3 a = triangle.v1 - triangle.v0;
			Vector3 b = triangle.v2 - triangle.v0;
			Vector3 normal = Vector3::Cross(a, b);
			if(Vector3::Dot(ray.direction,normal) == 0)
			{
				return false;
			}

			TriangleCullMode currentCulling = triangle.cullMode;
		

			Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3 };
			Vector3 L = center - ray.origin;
			float t = Vector3::Dot(L, normal) / Vector3::Dot(ray.direction,normal);
			if(t < ray.min || t > ray.max)
			{
				return false;
			}
			Vector3 p = ray.origin + (t * ray.direction);

			Vector3 edgeA = triangle.v1 - triangle.v0;
			Vector3 pointToSide = p - triangle.v0;
			if(Vector3::Dot(normal,Vector3::Cross(edgeA,pointToSide)) < 0)
			{
				return false;
			}

			Vector3 edgeB = triangle.v2 - triangle.v1;
			pointToSide = p - triangle.v1;
			if (Vector3::Dot(normal, Vector3::Cross(edgeB, pointToSide)) < 0)
			{
				return false;
			}

			Vector3 edgeC = triangle.v0 - triangle.v2;
			pointToSide = p - triangle.v2;
			if (Vector3::Dot(normal, Vector3::Cross(edgeC, pointToSide)) < 0)
			{
				return false;
			}


			if (ignoreHitRecord) {
				if (currentCulling == TriangleCullMode::BackFaceCulling) {
					currentCulling = TriangleCullMode::FrontFaceCulling;
				}
				else if (currentCulling == TriangleCullMode::FrontFaceCulling) {
					currentCulling = TriangleCullMode::BackFaceCulling;
				}
			}


			switch (currentCulling)
			{
			case TriangleCullMode::BackFaceCulling:
				if (Vector3::Dot(normal, ray.direction) > 0)
				{


					return false;

				}
				break;
			case TriangleCullMode::FrontFaceCulling:
				if (Vector3::Dot(normal, ray.direction) < 0)
				{


					return false;

				}
				break;

			}

			

			if(ignoreHitRecord)
			{
				return true;
			}

			hitRecord.normal = normal;
			hitRecord.origin = p;
			hitRecord.didHit = true;
			hitRecord.t = t;
			hitRecord.materialIndex = triangle.materialIndex;
			return true;


			
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;

		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5

			//slabtest
			if(!SlabTest_TriangleMesh(mesh,ray))
			{
				return false;
			}

		
			float dist{ FLT_MAX };
			HitRecord temp{};
			for(int i{}; i < mesh.indices.size();i++)
			{
				if( i % 3 == 0 || i == 0)
				{
					
					Triangle t;

					t.v0 = mesh.positions[mesh.indices[i]];
					t.v1 = mesh.positions[mesh.indices[i+1]];
					t.v2 = mesh.positions[mesh.indices[i+2]];
					t.normal = mesh.normals[i%3];
					t.cullMode = mesh.cullMode;
					t.materialIndex = mesh.materialIndex;
				
					if(HitTest_Triangle(t,ray,temp,ignoreHitRecord))
					{
						if(ignoreHitRecord)
						{
							return true;
						}
						if(temp.didHit && temp.t < dist)
						{
							dist = temp.t;
							hitRecord = temp;
						}
					}
				}
			}
			return hitRecord.didHit;

		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			if(light.type == LightType::Directional)
			{
				return Vector3(light.direction);
			} 

			return Vector3(light.origin - origin);
			////todo W3
		
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			Vector3 dist;

			switch(light.type)
			{
			case LightType::Directional:
				return light.color * light.intensity;
				break;
			case LightType::Point:
				dist = light.origin - target ;
			
				return light.color * light.intensity / Square(dist.Magnitude());

				break;
			}

		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}

		
#pragma warning(pop)
	}

	
}