#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <iostream>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <stdlib.h>

namespace LilSpheres {
	extern const int maxParticles;
	extern void updateParticles(int startIdx, int count, float* array_data);
	extern void setupParticles(int numTotalParticles, float radius = 0.05f);
	extern void cleanupParticles();
	extern int end ;
	extern int startu ;
}
namespace Sphere {
	extern int GetRadius();
	extern glm::vec3 GetPos();
	extern bool renderSphere;
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
}

namespace Capsule {
	extern int GetRadius();
	extern glm::vec3 GetPosA();
	extern glm::vec3 GetPosB();
	extern bool renderCapsule;
	extern void updateCapsule(glm::vec3 posA, glm::vec3 posB, float radius = 1.f);
}

bool show_test_window = false;
bool PlaySimulation = false;
bool Show_Emitter = true;
bool usegravity = true;

float rate = 1000;
float lifetime = 5;
float startpos[3] = { -4,2,4 };
float endpos[3] = { -4,2,-4 };
float vel[3] = { -3.2,4,0 };
float coefelastic = 0.000;
float coefricion = 0.000;
float possphere[3] = { 0,1,0 };
float poscapsuleA[3] = { 0, 2, 0 };
float poscapsuleB[3] = {3, 2, 0};
float agrav[3]= { 0,-9.81,0 };
float gravedad[3] = { 0,-9.81,0 };
float dir[3]{ 0,1,0 };
float rSphere = 1;
float angle=45;
float fontpos[3] = { 0,3,0 };
void GUI() {
	bool show = true;
	
	ImGui::Begin("Physics Parameters", &show, 0);

	// Do your GUI code here....
	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		ImGui::Checkbox("Play Simulation", &PlaySimulation);
		
		if (ImGui::CollapsingHeader("Emiter"))
		{
			ImGui::DragFloat("Rate", &rate, 1, 100, LilSpheres::maxParticles);
			ImGui::DragFloat("Liftime", &lifetime, 1,1,30);
			ImGui::DragFloat3("Cascade Pos A", startpos, 0.1f, -5.0f,10.0f);
			if (startpos[1] < 0)
			{
				startpos[1] = 0;
			}
			if (startpos[0] > 5)
			{
				startpos[0] = 5;
			}
			if (startpos[2] > 5)
			{
				startpos[2] = 5;
			}
			ImGui::DragFloat3("Cascade Pos B", endpos, 0.1f, -5.0f, 10.0f);
			if (endpos[1] < 0)
			{
				endpos[1] = 0;
			}
			if (endpos[0] > 5)
			{
				endpos[0] = 5;
			}
			if (endpos[2] > 5)
			{
				endpos[2] = 5;
			}
			ImGui::DragFloat3("Vel", vel, 0.1f, -5, 10);
			
		}
		if (ImGui::CollapsingHeader("Elasticity and Friction"))
		{
			ImGui::DragFloat("Elasticity", &coefelastic, 0.01, 0, 1);
			ImGui::DragFloat("Friction", &coefricion, 0.01, 0, 1);
		}
		if (ImGui::CollapsingHeader("Coliders"))
		{
			ImGui::Checkbox("Play Sphere", &Sphere::renderSphere);

			ImGui::DragFloat3("Pos Sphere", possphere, 0.1f, -5.0f+rSphere, 10.0f-rSphere);
			if (possphere[1] < 0+ rSphere)
			{
				possphere[1] = 0+rSphere;
			}
			if (possphere[0] > 5-rSphere)
			{
				possphere[0] = 5 - rSphere;
			}
			if (possphere[2] > 5 - rSphere)
			{
				possphere[2] = 5 - rSphere;
			}

			ImGui::DragFloat("Radius", &rSphere, 0.1, 1, 5);
			Sphere::updateSphere({ possphere[0],possphere[1],possphere[2] }, rSphere);
		}
		if (ImGui::CollapsingHeader("Forces"))
		{
			ImGui::Checkbox("Use gravity", &usegravity);
			if (!usegravity)
			{
				for (int i = 0; i < 3; i++)
				{
					gravedad[i] = 0;
				}
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					gravedad[i] = agrav[i];
				}
			}
			ImGui::DragFloat3("Gravity", agrav, 0.1f, -20, 20.0f );
		}
	
	}
	// .........................
	
	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		
		ImGui::SetNextWindowPos(ImVec2(750, 20), ImGuiSetCond_FirstUseEver);
		
		ImGui::ShowTestWindow(&show_test_window);
	}
}
//variables





float *PPos = new float[LilSpheres::maxParticles * 3];
float *PVel = new float[LilSpheres::maxParticles * 3];
float *Ptime = new float[LilSpheres::maxParticles];
bool colisions = false;
const int dsuelo = 0;
float normalsuelo[3] = { 0,-1, 0 };

void PhysicsInit() {
	
	// Do your initialization code here...
	for (int i = 0; i < LilSpheres::maxParticles; i++)
	{
		PPos[i] = 0;
		PPos[i+1] = 0;
		PPos[i+2] = 0;
		Ptime[i] = 0;
	}
	// ...................................
	LilSpheres::updateParticles(LilSpheres::startu,LilSpheres::maxParticles,PPos);
}

bool derecha = false;
bool delante = false;
glm::vec3 normalderecha = { -1,0,0 };
glm::vec3 normaldelante = { 0,0,-1 };

bool ColisionSphere(float dt, int i)
{
	float newPos[3];
	for (int j = 0; j < 3; j++)
	{
		newPos[j] = PPos[j + i * 3] + PVel[j + i * 3] * dt;
	}
	float vec[3] = { newPos[0] - Sphere::GetPos().x,newPos[1] - Sphere::GetPos().y,newPos[2] - Sphere::GetPos().z };
	float distance = sqrt(pow(vec[0], 2) + pow(vec[1], 2) + pow(vec[2], 2));
	float radius = Sphere::GetRadius();
	if (distance < radius)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool t1istheright(float VectorPPf[3], float Point[3], float t1, float t2)
{
	float distnacet1 = 0;
	float distancet2=0;
	float XYZ1[3];
	float XYZ2[3];
	for (int i = 0; i < 3; i++)
	{
		float aux2 = Point[i];
		float aux3 = VectorPPf[i];
		XYZ1[i] = Point[i] + VectorPPf[i] * t1;
		XYZ2[i] = Point[i] + VectorPPf[i] * t2;
		distnacet1 += pow((XYZ1[i] - Point[i]), 2);
		distancet2 += pow((XYZ2[i] - Point[i]), 2);
	}
	
	distnacet1=sqrt(distnacet1);
	
	distancet2 = sqrt(distancet2);

	if (distnacet1 < distancet2)
	{
		return true;
	}
	return false;
}
int  DEGREE = 3.1415926/180;
void PhysicsUpdate(float dt) {
	// Do your   here...
	if (PlaySimulation)
	{
		LilSpheres::end = LilSpheres::end + rate*dt;
		LilSpheres::end = (LilSpheres::end) % LilSpheres::maxParticles;
		float Uvector[3];
		float modul = 0;
		for (int i = 0; i < 3; i++)
		{
			Uvector[i] = endpos[i] - startpos[i];
			modul += Uvector[i] * Uvector[i];
		}
		modul = sqrt(modul);
		for (int j = 0; j < 3; j++)
		{
			Uvector[j] = Uvector[j] / modul;
		}
		for (int i = 0; i <= LilSpheres::maxParticles; i++)
		{
			if ((i >= LilSpheres::startu && i <= LilSpheres::end) || (LilSpheres::end < LilSpheres::startu && (i < LilSpheres::end || i>LilSpheres::startu)))
			{
				if (Ptime[i] == 0)
				{
					/*float r = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX / modul));
					for (int j = 0; j < 3; j++)
					{
						float randomY;
						if (j == 1)
						{
							randomY = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (0.4 - 0.1)));
						}
						else
						{
							randomY = 0;
						}
						PPos[j + i * 3] = Uvector[j] * r + startpos[j] - randomY;
						PVel[j + i * 3] = vel[j];
					}
					Ptime[i] += dt;*/
					float ranfodZ = static_cast<float>(rand() % 361);
					float initspeed[3];
					float aux = (ranfodZ*3.14)/180;

					aux = static_cast<float>(sin(aux));
					initspeed[0] = cos(aux)*dir[0]* cos(angle*DEGREE)*3;
					initspeed[1] = static_cast<float>(sin(aux))*dir[1]*3;
					initspeed[2] = cos(aux)*dir[2]*sin(angle*DEGREE)*3;
					for (int j = 0; j < 3; j++)
					{
						PPos[j + i * 3] = PPos[j + i * 3] + initspeed[j] * dt;
						PVel[j + i * 3] = initspeed[j ] + dt*gravedad[j];
					}
					Ptime[i] += dt;

				}
				else if (Ptime[i] >= lifetime)
				{

					for (int j = 0; j < 3; j++)
					{
						PPos[j + i * 3] = 0;
					}

					Ptime[i] = 0;
					LilSpheres::startu = (LilSpheres::startu + 1) % LilSpheres::maxParticles;
				}
				else
				{

					if (PPos[i * 3 + 1] + PVel[i * 3 + 1] * dt <= 0)
					{

						for (int j = 0; j < 3; j++)
						{
							PVel[j + i * 3] = (PVel[j + i * 3] + dt*gravedad[j]) - (1 + coefelastic-coefricion)*(normalsuelo[j] * (PVel[j + i * 3] + dt*gravedad[j]))*normalsuelo[j];
						}


					}

					else if (PPos[i * 3] + PVel[i * 3] * dt > 5 || PPos[i * 3] + PVel[i * 3] * dt < -5)
					{
						for (int j = 0; j < 3; j++)
						{
							PVel[j + i * 3] = (PVel[j + i * 3] + dt*gravedad[j]) - (1 + coefelastic-coefricion)*(normalderecha[j] * (PVel[j + i * 3] + dt*gravedad[j]))*normalderecha[j];
						}
					}
					else if (PPos[i * 3 + 2] + PVel[i * 3 + 2] * dt > 5 || PPos[i * 3 + 2] + PVel[i * 3 + 2] * dt < -5)
					{
						for (int j = 0; j < 3; j++)
						{
							PVel[j + i * 3] = (PVel[j + i * 3] + dt*gravedad[j]) - (1 + coefelastic-coefricion)*(normaldelante[j] * (PVel[j + i * 3] + dt*gravedad[j]))*normaldelante[j];
						}
					}
					else if (ColisionSphere(dt, i)&&Sphere::renderSphere)
					{
						float VectorPPf[3] = { PPos[i * 3] - (PPos[i * 3] + PVel[i * 3] * dt),PPos[i * 3 + 1] - (PPos[i * 3 + 1] + PVel[i * 3 + 1] * dt),PPos[i * 3 + 2] - (PPos[i * 3 + 2] + PVel[i * 3 + 2] * dt) };
						float PofRect[3] = { PPos[i * 3],PPos[i * 3 + 1],PPos[i * 3 + 2] };
						float a, b, c;
						float PmenosC[3] = { PofRect[0] - Sphere::GetPos().x,PofRect[1] - Sphere::GetPos().y,PofRect[2] - Sphere::GetPos().z };
						a = pow(VectorPPf[0], 2) + pow(VectorPPf[1], 2) + pow(VectorPPf[2], 2);
						b = (2 * PmenosC[0] * VectorPPf[0]) + (2 * PmenosC[1] * VectorPPf[1]) + (2 * PmenosC[2] * VectorPPf[2]);
						c = pow(PmenosC[0], 2) + pow(PmenosC[1], 2) + pow(PmenosC[2], 2) - pow(Sphere::GetRadius(), 2);
						float t1 = -b - sqrt(pow(b, 2) - 4 * a*c);
						t1 = t1 / (2 * a);
						float t2 = -b + sqrt(pow(b, 2) - 4 * a*c);
						t2 = t2 / (2 * a);
						float PColision[3];
						if (t1istheright(VectorPPf, PofRect, t1, t2))
						{
							for (int j = 0; j < 3; j++)
							{
								PColision[j] = PofRect[j] + VectorPPf[j] * t1;
							}
							float aux1 = sqrt(pow(PColision[0], 2) + pow(PColision[1], 2) + pow(PColision[2], 2));
							float aux2 = Sphere::GetRadius();

						}
						else
						{
							for (int j = 0; j < 3; j++)
							{
								PColision[j] = PofRect[j] + VectorPPf[j] * t2;

							}
							float aux1 = sqrt(pow(PColision[0], 2) + pow(PColision[1], 2) + pow(PColision[2], 2));
							float aux2 = Sphere::GetRadius();
						}
						float VectorPcC[3] = { Sphere::GetPos().x - PColision[0],Sphere::GetPos().y - PColision[1],Sphere::GetPos().z - PColision[2] };
						float d = 0;
						for (int j = 0; j < 3; j++)
						{
							d -= VectorPcC[j] * PColision[j];
						}
						for (int z = 0; z < 3; z++)
						{
							PPos[z + i * 3] = (PPos[z + i * 3] + PVel[z + i * 3] * dt) - (1 + coefelastic-coefricion)*(VectorPcC[z] * (PPos[z + i * 3] + PVel[z + i * 3] * dt) + d)*VectorPcC[z];
							float aux = PPos[z + i * 3];
							PVel[z + i * 3] = (PVel[z + i * 3] + dt*gravedad[z]) - (2)*(VectorPcC[z] * (PVel[z + i * 3] + dt*gravedad[z]))*VectorPcC[z];
						}

					}
					else
					{
						for (int j = 0; j < 3; j++)
						{

							PPos[j + i * 3] = PPos[j + i * 3] + PVel[j + i * 3] * dt;
							PVel[j + i * 3] = PVel[j + i * 3] + dt*gravedad[j];

						}
					}


					Ptime[i] += dt;
				}
			}
		}

		LilSpheres::updateParticles(0, LilSpheres::maxParticles, PPos);

	}
	else
	{
		
		LilSpheres::cleanupParticles();
		LilSpheres::setupParticles(LilSpheres::maxParticles);
	}

	
	// ...........................
}

void PhysicsCleanup() {
	// Do your cleanup code here...
	
	// ............................
}