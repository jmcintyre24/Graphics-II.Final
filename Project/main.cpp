#include "defines.h"

#include "DrawClass.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

//Globals//
unsigned int width, height;

GWindow win;
GEventReceiver msgs;
GDirectX11Surface d3d11;

struct OBJVert
{
	UINT posI;
	UINT uvI;
	UINT normI;
};

void ReadOBJFaceVert(std::string a, OBJVert& v)
{
	std::string::size_type sz;

	v.posI = std::stoi(a, &sz);
	std::string::size_type rem = a.find("/");
	for (std::string::size_type i = 0; i <= rem; i++)
		a.replace(0, 1, "");
	v.uvI = std::stoi(a, &sz);
	rem = a.find("/");
	for (std::string::size_type i = 0; i <= rem; i++)
		a.replace(0, 1, "");
	v.normI = std::stoi(a, &sz);

	// Subtract by 1 since obj starts at 1 instead of 0
	v.posI -= 1;
	v.uvI -= 1;
	v.normI -= 1;
}

void ReadModel(std::string pathToModel, Mesh::SimpleMesh& mesh)
{	
	// Vector to push back verts being read in.
	std::ifstream in(pathToModel);
	std::vector<XMFLOAT4> tempPOSVec;
	std::vector<XMFLOAT3> tempNORMVec;
	std::vector<XMFLOAT2> tempUVVec;

	UINT indice = 0;

	// Read OBJ file in.
	if (in.is_open())
	{
		for (std::string str; std::getline(in, str);)   //read stream line by line
		{
			std::istringstream in(str);

			std::string val;
			in >> val;

			// Vertex
			if (val == "v")
			{
				float x, y, z;
				in >> x >> y >> z;

				tempPOSVec.push_back({ x, y, z * -1.0f, 1.0f }); // Invert to left handed system.
			}
			// Normals
			else if (val == "vn")
			{
				float x, y, z;
				in >> x >> y >> z;

				tempNORMVec.push_back({ x, y, z * -1.0f }); // Invert to left handed system.
			}
			// UVs
			else if (val == "vt")
			{
				float u, v;
				in >> u >> v;
				v = 1.0f - v; // Invert to left handed system.
				tempUVVec.push_back({ u, v });
			}
			// Face
			else if (val == "f")
			{
				std::string a, b, c, d;
				in >> d >> c >> b >> a; // Read in backwards to convert to left handed

				// If it's a quad, break it into two triangles
				if (!a.empty())
				{
					OBJVert av, bv, cv, dv;
					ReadOBJFaceVert(a, av);
					ReadOBJFaceVert(b, bv);
					ReadOBJFaceVert(c, cv);
					ReadOBJFaceVert(d, dv);

					Mesh::SimpleVertex v = {
						{tempPOSVec[av.posI]},
						{tempNORMVec[av.normI]},
						{tempUVVec[av.uvI]}
					};

					Mesh::SimpleVertex v1 = {
						{tempPOSVec[bv.posI]},
						{tempNORMVec[bv.normI]},
						{tempUVVec[bv.uvI]}
					};

					Mesh::SimpleVertex v2 = {
						{tempPOSVec[cv.posI]},
						{tempNORMVec[cv.normI]},
						{tempUVVec[cv.uvI]}
					};

					Mesh::SimpleVertex v3 = {
						{tempPOSVec[dv.posI]},
						{tempNORMVec[dv.normI]},
						{tempUVVec[dv.uvI]}
					};

					int vEX = -1, v1EX = -1, v2EX = -1, v3EX = -1;
					for (int i = 0; i < mesh.vertexList.size(); i++)
					{
						if (v == mesh.vertexList[i])
							vEX = i;

						if (v1 == mesh.vertexList[i])
							v1EX = i;

						if (v2 == mesh.vertexList[i])
							v2EX = i;

						if (v3 == mesh.vertexList[i])
							v3EX = i;
					}

					// Triangle One
					if (vEX == -1)
					{
						mesh.vertexList.push_back(v);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(vEX);

					if (v1EX == -1)
					{
						mesh.vertexList.push_back(v1);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(v1EX);

					if (v2EX == -1)
					{
						mesh.vertexList.push_back(v2);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(v2EX);

					// Triangle Two
					if (vEX == -1)
					{
						mesh.vertexList.push_back(v);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(vEX);

					if (v2EX == -1)
					{
						mesh.vertexList.push_back(v2);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(v2EX);

					if (v3EX == -1)
					{
						mesh.vertexList.push_back(v3);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(v3EX);
				}
				// Read in the triangle
				else
				{
					OBJVert bv, cv, dv;
					ReadOBJFaceVert(b, bv);
					ReadOBJFaceVert(c, cv);
					ReadOBJFaceVert(d, dv);

					Mesh::SimpleVertex v = {
						{tempPOSVec[dv.posI]},
						{tempNORMVec[dv.normI]},
						{tempUVVec[dv.uvI]}
					};

					Mesh::SimpleVertex v1 = {
						{tempPOSVec[bv.posI]},
						{tempNORMVec[bv.normI]},
						{tempUVVec[bv.uvI]}
					};

					Mesh::SimpleVertex v2 = {
						{tempPOSVec[cv.posI]},
						{tempNORMVec[cv.normI]},
						{tempUVVec[cv.uvI]}
					};

					int vEX = -1, v1EX = -1, v2EX = -1;
					for (int i = 0; i < mesh.vertexList.size(); i++)
					{
						if(v == mesh.vertexList[i])
							vEX = i;

						if (v1 == mesh.vertexList[i])
							v1EX = i;

						if (v2 == mesh.vertexList[i])
							v2EX = i;
					}

					if (vEX == -1)
					{
						mesh.vertexList.push_back(v);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(vEX);

					if (v1EX == -1)
					{
						mesh.vertexList.push_back(v1);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(v1EX);

					if (v2EX == -1)
					{
						mesh.vertexList.push_back(v2);
						mesh.indicesList.push_back(indice);
						indice++;
					}
					else
						mesh.indicesList.push_back(v2EX);
				}
			}
		}
	}
	char* e = strerror(errno);
}

// lets pop a window and use D3D11 to clear to a green screen
int main()
{
	if (+win.Create(0, 0, 1280, 768, GWindowStyle::WINDOWEDBORDERED))
	{
		win.SetWindowName("Joseph_McIntyre_DEV4_FinalWOBJLoader");

		+win.GetWidth(width);
		+win.GetHeight(height);

		float clr[] = { 0.2f, 0.2f, 0.4f, 1 }; // start with blue

		msgs.Create(win, [&]() {
			if (+msgs.Find(GWindow::Events::RESIZE, true))
				{
					+win.GetWidth(width);
					+win.GetHeight(height);
				}
			});

		if (+d3d11.Create(win, DEPTH_BUFFER_SUPPORT))
		{
			Mesh::SimpleMesh crossbowMesh;
			Mesh::SimpleMesh balloonMesh;
			ReadModel(".\\Models\\crossbow.obj", crossbowMesh);
			ReadModel(".\\Models\\balloon.obj", balloonMesh);

			Mesh mainScene(d3d11, win, &crossbowMesh, &balloonMesh, L"Textures\\LongMattedGrass.dds", L"Textures\\lowpoly_crossbow.dds");

			while (+win.ProcessWindowEvents())
			{
				IDXGISwapChain* swap = nullptr;
				ID3D11DeviceContext* con = nullptr;
				ID3D11RenderTargetView* view = nullptr;
				ID3D11DepthStencilView* depthview = nullptr;

				if (+d3d11.GetImmediateContext((void**)&con) &&
					+d3d11.GetRenderTargetView((void**)&view) &&
					+d3d11.GetSwapchain((void**)&swap) &&
					+d3d11.GetDepthStencilView((void**)&depthview))
				{
					// Clear the render target view.
					con->ClearRenderTargetView(view, clr);
					// Clear the depth stencil view.
					con->ClearDepthStencilView(depthview, D3D11_CLEAR_DEPTH, 1.0f, 0);

					// Check if window is in focus for user input.
					bool isFocused;
					+win.IsFocus(isFocused);
					if(isFocused)
						mainScene.UserInput();

					// Render the scene out.
					mainScene.Render();
					
					swap->Present(1, 0);
					con->Release();
					view->Release();
					depthview->Release();
					swap->Release();
				}
			}
		}
	}
	return 0;
}