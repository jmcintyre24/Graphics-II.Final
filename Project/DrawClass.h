#pragma once
#include <zmouse.h>
#include "defines.h"
#include "DDSTextureLoader.h"

// Base class for drawing objects
class DrawClass
{
public:
	DrawClass(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win)
	{
		win = _win;
		d3d11 = _d3d11;

		+win.GetWidth(width);
		+win.GetHeight(height);


		+win.GetClientTopLeft(clientTopLeftX, clientTopLeftY);

		+win.GetClientWidth(clientWidth);
		+win.GetClientHeight(clientHeight);
	}

	// Used for compiling shaders
	static HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer())); // Print to output window.
				pErrorBlob->Release();
			}
			return hr;
		}
		if (pErrorBlob) pErrorBlob->Release();

		return S_OK;
	}
protected:
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d11;
	unsigned int width = 0, height = 0;
	UINT clientWidth = 0, clientHeight = 0, clientTopLeftX = 0, clientTopLeftY;
};

class Mesh : DrawClass
{
public:
	struct SimpleVertex
	{
		XMFLOAT4 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 UV;

		inline SimpleVertex* operator=(SimpleVertex v)
		{
			this->Pos = v.Pos;
			this->Normal = v.Normal;
			this->UV = v.UV;

			return this;
		}

		inline bool operator==(SimpleVertex v)
		{
			if(!(this->Pos.x == v.Pos.x) || !(this->Pos.y == v.Pos.y) || !(this->Pos.z == v.Pos.z)) return false;
			if(!(this->Normal.x == v.Normal.x) || !(this->Normal.y == v.Normal.y) || !(this->Normal.z == v.Normal.z)) return false;
			if(!(this->UV.x == v.UV.x) || !(this->UV.y == v.UV.y)) return false;

			return true;
		}
	};

	struct SimpleMesh
	{
		std::vector<SimpleVertex> vertexList;
		std::vector<unsigned int> indicesList;
	};

private:
	struct ConstantBuffer
	{
		XMMATRIX mWorld;
		XMMATRIX mView;
		XMMATRIX mProjection;
		XMFLOAT4 lightDir;
		XMFLOAT4 lightClr;
		XMFLOAT4 vOutputColor;
		bool popped[3];
		float time;
	};

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		renderTargetView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			input = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexshaderwave = nullptr;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>		geoshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			PS_MAIN = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			PS_SPECULAR = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			PS_NOLIGHTS = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			PS_CROSSHAIR = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				constantbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	planeTextureRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	crossbowTextureRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	crosshairTextureRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			samplerLinear = nullptr;
	XMMATRIX											g_World;
	XMMATRIX											g_View;
	XMMATRIX											g_Projection;

	XMFLOAT4 lightDir, lightClr; // Should've used a structure here - Note for 'next' time.

	SimpleMesh* crossbowMesh = nullptr;
	SimpleMesh* balloonMesh = nullptr;

	// -PLANE- //
	Microsoft::WRL::ComPtr<ID3D11Buffer>				p_vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				p_indexbuffer = nullptr;

	// Indicies for Plane
	std::vector<unsigned int> planeIndices;
	XMFLOAT4 plane_pos = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Plane generation.
	void CreatePlane(ID3D11Device* dev, ID3D11DeviceContext* con)
	{
		std::vector<SimpleVertex> verts;
		
		// Generate the simple plane.
		{
			verts.push_back({ XMFLOAT4(-1.0f, 0.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) });
			verts.push_back({ XMFLOAT4(1.0f, 0.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(100.0f, 0.0f) });
			verts.push_back({ XMFLOAT4(-1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 100.0f) });
			verts.push_back({ XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(100.0f, 100.0f) });
		}

		// Create the vertex buffer for the plane.
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * verts.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = verts.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, p_vertexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Generate the simple plane indicies
		{
			planeIndices.push_back(0);
			planeIndices.push_back(2);
			planeIndices.push_back(3);
			planeIndices.push_back(0);
			planeIndices.push_back(3);
			planeIndices.push_back(1);
		}

		// Create the index buffer.
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * planeIndices.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = planeIndices.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, p_indexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
	}
	// Render the plane.
	void RenderPlane(ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ConstantBuffer& cb)
	{
		// Change Topology to Lines
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set vertex buffer
		const UINT stride[] = { sizeof(SimpleVertex) };
		const UINT offset[] = { 0 };
		ID3D11Buffer* const buffs[] = { p_vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, stride, offset);

		// Set Index Buffer
		con->IASetIndexBuffer(p_indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Update the world variable to reflect the current light
		XMMATRIX w_Plane = XMMatrixTranslationFromVector(XMLoadFloat4(&plane_pos)) * XMMatrixScaling(60.0f, 60.0f, 60.0f);
		cb.mWorld = XMMatrixTranspose(w_Plane);
		cb.vOutputColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Update VS and PS
		con->VSSetShader(vertexshader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShader(PS_NOLIGHTS.Get(), nullptr, 0);
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetSamplers(0, 1, samplerLinear.GetAddressOf());

		con->DrawIndexed(planeIndices.size(), 0, 0);
	}
	// -END OF PLANE- //

	// -INVERTED CUBE | SKYBOX- //
	Microsoft::WRL::ComPtr<ID3D11Buffer>				c_vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				c_indexbuffer = nullptr;
	// For Skybox Generation
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			SKBvertexshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			SKBpixelshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				SKBvertex_Buffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			SKBinput = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	SKBtextureRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		depthStencilState = nullptr;
	// Generate a hard-coded inverted cube.
	void CreateInvertedCube(ID3D11Device* dev, ID3D11DeviceContext* con)
	{
		// Create vertex buffer
		SimpleVertex vertices[] =
		{
			{ XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(1.0f, 0.0f)},
			{ XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(0.0f, 0.0f)},
			{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(1.0f, 1.0f)},

			{ XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, -1.0f, 0.0f),	XMFLOAT2(0.0f, 0.0f)},
			{ XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, -1.0f, 0.0f),	XMFLOAT2(1.0f, 0.0f)},
			{ XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, -1.0f, 0.0f),	XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, -1.0f, 0.0f),	XMFLOAT2(0.0f, 1.0f)},

			{ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(1.0f, 0.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(0.0f, 0.0f)},

			{ XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(0.0f, 0.0f)},
			{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(1.0f, 0.0f)},

			{ XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(1.0f, 0.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(0.0f, 0.0f)},

			{ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(0.0f, 0.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(1.0f, 0.0f)},
		};
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * 24;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = vertices;
		if (FAILED(dev->CreateBuffer(&bd, &InitData, c_vertexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
		// Create index buffer
		unsigned int indices[] =
		{
			0,1,3,
			3,1,2,

			5,4,6,
			6,4,7,

			8,9,11,
			11,9,10,

			13,12,14,
			14,12,15,

			16,17,19,
			19,17,18,

			21,20,22,
			22,20,23
		};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * 36;        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;
		if (FAILED(dev->CreateBuffer(&bd, &InitData, c_indexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
	}
	// Render the skybox out.
	void RenderSkybox(ID3D11DeviceContext* con, ConstantBuffer& cb)
	{
		// Set vertex buffer
		const UINT c_stride[] = { sizeof(SimpleVertex) };
		const UINT c_offset[] = { 0 };
		ID3D11Buffer* const c_buffs[] = { c_vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(c_buffs), c_buffs, c_stride, c_offset);

		// Set Index Buffer
		con->IASetIndexBuffer(c_indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR camPos = g_View.r[3];
		XMFLOAT4 skyPos = { XMVectorGetX(camPos), XMVectorGetY(camPos), XMVectorGetZ(camPos), 1.0f };
		XMMATRIX mSky = XMMatrixTranslationFromVector(XMLoadFloat4(&skyPos));
		XMMATRIX mScaleSky = XMMatrixScaling(50.0f, 50.0f, 50.0f);
		mSky = mScaleSky * mSky;

		// Update world variable for skybox
		cb.mWorld = XMMatrixTranspose(mSky);
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Update vertex and pixel shader for skybox.
		con->VSSetShader(SKBvertexshader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShader(SKBpixelshader.Get(), nullptr, 0);
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());

		// Set input layout.
		con->IASetInputLayout(SKBinput.Get());
		con->OMSetDepthStencilState(depthStencilState.Get(), 0);
		con->DrawIndexed(36, 0, 0);
		con->OMSetDepthStencilState(NULL, 0);

		// Reset the input layout.
		con->IASetInputLayout(input.Get());
	}
	// -END OF INVERTED CUBE | SKYBOX- //

	// -CROSSBOW & BALLON MESH- //
	// Crossbow Variables
	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				indexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				b_vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				b_indexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		depthStencilStateFront = nullptr;


	void CreateMesh(ID3D11Device* dev, ID3D11DeviceContext* con, std::vector<SimpleVertex>* verticies, std::vector<unsigned int>* indicies, Microsoft::WRL::ComPtr<ID3D11Buffer>& _vertexbuffer, Microsoft::WRL::ComPtr<ID3D11Buffer>& _indexbuffer)
	{
		// Create Vertex Buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * verticies->size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = verticies->data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, _vertexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Create Index Buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int) * indicies->size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indicies->data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, _indexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
	}
	void RenderMesh(ID3D11DeviceContext* con, ConstantBuffer& cb, SimpleMesh* mesh, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>* textureResourceView = nullptr, Microsoft::WRL::ComPtr<ID3D11PixelShader>* pixelShader = nullptr)
	{
		// Render the mesh
		// Set vertex buffer
		const UINT stride[] = { sizeof(SimpleVertex) };
		const UINT offset[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, stride, offset);

		// If it's the crossbow, attach it to the camera.
		if (crossbowMesh == mesh)
		{
			// World for crossbow
			XMMATRIX crossWorld = XMMatrixIdentity();
			crossWorld = XMMatrixMultiply(crossWorld, g_View);
			crossWorld = XMMatrixTranslation(0.8f, -0.85f, 1.0f) * crossWorld;
			crossWorld = XMMatrixRotationY(1.5708f) * crossWorld; // Rotate 90 degrees
			crossWorld = XMMatrixScaling(0.75f, 0.75f, 0.75f) * crossWorld;
			// Update world variable for skybox
			cb.mWorld = XMMatrixTranspose(crossWorld);
			con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);
			con->OMSetDepthStencilState(depthStencilStateFront.Get(), 0);
		}

		// Set Index Buffer
		con->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set Vertex Shader
		con->VSSetShader(vertexshader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());

		// Set Pixel Shader
		if(pixelShader == nullptr)
			con->PSSetShader(PS_MAIN.Get(), nullptr, 0);
		else
			con->PSSetShader(pixelShader->Get(), nullptr, 0);

		if(textureResourceView == nullptr)
			con->PSSetShaderResources(1, 1, crossbowTextureRV.GetAddressOf());
		else
			con->PSSetShaderResources(1, 1, textureResourceView->GetAddressOf());
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetSamplers(0, 1, samplerLinear.GetAddressOf());

		// Draw out the mesh
		con->DrawIndexed(mesh->indicesList.size(), 0, 0);
		con->OMSetDepthStencilState(NULL, 0);
		con->GSSetShader(nullptr, 0, 0);
	}
	// -END OF CROSSBOW MESH- //

	// -CROSSHAIR GENERATION- //
	Microsoft::WRL::ComPtr<ID3D11Buffer>				cross_vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				cross_indexbuffer = nullptr;

	// Indicies for Plane
	std::vector<unsigned int> crossIndices;
	XMFLOAT4 cross_pos = { 0.0f, 0.0f, 12.0f, 1.0f };

	void CreateNDCPlane(ID3D11Device* dev, ID3D11DeviceContext* con)
	{
		std::vector<SimpleVertex> verts;

		// Generate the simple plane.
		{
			verts.push_back({ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2 (0.0f, 0.0f) });
			verts.push_back({ XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2 (1.0f, 0.0f) });
			verts.push_back({ XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2 (0.0f, 1.0f) });
			verts.push_back({ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2 (1.0f, 1.0f) });
		}

		// Create the vertex buffer for the plane.
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * verts.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = verts.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, cross_vertexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Generate the simple plane indicies
		{
			crossIndices.push_back(0);
			crossIndices.push_back(2);
			crossIndices.push_back(3);
			crossIndices.push_back(0);
			crossIndices.push_back(3);
			crossIndices.push_back(1);
		}

		// Create the index buffer.
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * crossIndices.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = crossIndices.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, cross_indexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
	}
	void RenderCrosshair(ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ConstantBuffer& cb)
	{
		// Change Topology to Lines
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Set vertex buffer
		const UINT stride[] = { sizeof(SimpleVertex) };
		const UINT offset[] = { 0 };
		ID3D11Buffer* const buffs[] = { cross_vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, stride, offset);

		// Set Index Buffer
		con->IASetIndexBuffer(cross_indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Update the world variable
		XMMATRIX w_Plane = XMMatrixTranslationFromVector(XMLoadFloat4(&cross_pos)) * XMMatrixScaling(0.1f, 0.1f, 0.1f);
		cb.mWorld = XMMatrixTranspose(w_Plane);
		cb.vOutputColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Update VS and PS
		con->VSSetShader(vertexshaderwave.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShader(PS_CROSSHAIR.Get(), nullptr, 0);
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetSamplers(0, 1, samplerLinear.GetAddressOf());
		con->OMSetDepthStencilState(depthStencilStateFront.Get(), 0);
		con->DrawIndexed(crossIndices.size(), 0, 0);
		con->OMSetDepthStencilState(nullptr, 0);
	}
	// -END OFCROSSHAIR GENERATION- //

	// -BALLOON RENDERING- //
	XMFLOAT4 balloonPosOne = { -5.0f, 4.0f, -2.0f, 1.0f };
	XMFLOAT4 balloonPosTwo = { 0.0f, 4.0f, -2.0f, 1.0f };
	XMFLOAT4 balloonPosThree = { 5.0f, 4.0f, -2.0f, 1.0f };
	XMMATRIX b_World[3] = {};

	void RenderBalloons(ID3D11DeviceContext* con, ConstantBuffer& cb, SimpleMesh* mesh, float& time)
	{
		// Render the mesh
		// Set vertex buffer
		const UINT stride[] = { sizeof(SimpleVertex) };
		const UINT offset[] = { 0 };
		ID3D11Buffer* const buffs[] = { b_vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, stride, offset);

		// Update Position of Balloon One
		balloonPosOne.x -= sin(time * 2.0f) / 10.0f;

		// Update the world variable & color for the balloon
		b_World[0] = XMMatrixTranslationFromVector(XMLoadFloat4(&balloonPosOne)) * XMMatrixScaling(0.3f, 0.3f, 0.3f);
		cb.mWorld = XMMatrixTranspose(b_World[0]);
		cb.vOutputColor = { 1.0f, 0.0f, 0.0f, 1.0f };
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Set Index Buffer
		con->IASetIndexBuffer(b_indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set Vertex Shader
		con->VSSetShader(vertexshader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());

		// Set Pixel Shader
		con->PSSetShader(PS_SPECULAR.Get(), nullptr, 0);
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());

		// Draw out the mesh - Balloon One
		con->DrawIndexed(mesh->indicesList.size(), 0, 0);
		

		// Update the world variable & color
		// Update Position of Balloon One
		balloonPosTwo.y += sin(time * 2.0f) / 10.0f;
		b_World[1] = XMMatrixTranslationFromVector(XMLoadFloat4(&balloonPosTwo)) * XMMatrixScaling(0.3f, 0.3f, 0.3f);
		cb.mWorld = XMMatrixTranspose(b_World[1]);
		cb.vOutputColor = { 0.0f, 1.0f, 0.0f, 1.0f };
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);
		// Draw out the mesh - Balloon Two
		con->DrawIndexed(mesh->indicesList.size(), 0, 0);
		
		// Update the world variable & color
		// Update Position of Balloon One
		balloonPosThree.x += sin(time * 2.0f) / 10.0f;
		b_World[2] = XMMatrixTranslationFromVector(XMLoadFloat4(&balloonPosThree)) * XMMatrixScaling(0.3f, 0.3f, 0.3f);
		cb.mWorld = XMMatrixTranspose(b_World[2]);
		cb.vOutputColor = { 0.0f, 0.0f, 1.0f, 1.0f };
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);
		// Draw out the mesh - Balloon Three
		con->DrawIndexed(mesh->indicesList.size(), 0, 0);
	}
	// -END OF BALLOON GENERATION- //
public:

	Mesh(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win, SimpleMesh* _mesh, SimpleMesh* _meshtwo, const wchar_t* texturePath, const wchar_t* textureTwoPath) : DrawClass(_d3d11, _win)
	{
		if (_mesh == nullptr)
		{
			std::cout << "Mesh was nullptr/Invalid\n";
		}

		crossbowMesh = _mesh;
		balloonMesh = _meshtwo;
		ID3D11Device* dev = nullptr;
		ID3D11DeviceContext* con = nullptr;
		ID3D11DepthStencilView* depthview = nullptr;
		+d3d11.GetDevice((void**)&dev);
		+d3d11.GetImmediateContext((void**)(&con));
		+d3d11.GetDepthStencilView((void**)&depthview);

		// Back Buffer setup
		{
			IDXGISwapChain* swp = nullptr;
			+d3d11.GetSwapchain((void**)(&swp));
			// Create a render target view
			ID3D11Texture2D* pBackBuffer = nullptr;
			if (FAILED(swp->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
				return;

			if (FAILED(dev->CreateRenderTargetView(pBackBuffer, nullptr, renderTargetView.GetAddressOf())))
			{
				pBackBuffer->Release();
				return;
			}
			pBackBuffer->Release();
			swp->Release();

			con->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthview);
			depthview->Release();
		}

		// Creation of DEPTH stencil desc
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		if (FAILED(dev->CreateDepthStencilState(&desc, depthStencilState.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		if (FAILED(dev->CreateDepthStencilState(&desc, depthStencilStateFront.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// -VERTEX SHADERS- //
#pragma region VERTSHADERS
		// Compile the vertex shader
		ID3DBlob* pVSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "VS", "vs_4_0", &pVSBlob)))
		{
			DebugBreak();
			return;
		}

		// Create the vertex shader
		if (FAILED(dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, vertexshader.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		// Compile the vertex shader for the wave.
		pVSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "VSWave", "vs_4_0", &pVSBlob)))
		{
			DebugBreak();
			return;
		}

		// Create the vertex shader for the wave.
		if (FAILED(dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, vertexshaderwave.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		// Create the input layout
		if (FAILED(dev->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), input.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		// Skybox VS
		// Compile the Skybox vertex shader
		pVSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "SKYBOX_VS", "vs_4_0", &pVSBlob)))
		{
			DebugBreak();
			return;
		}

		// Create the Skybox vertex shader
		if (FAILED(dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, SKBvertexshader.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC SKBlayout[] =
		{
			{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		numElements = ARRAYSIZE(SKBlayout);

		// Create the input layout
		if (FAILED(dev->CreateInputLayout(SKBlayout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), SKBinput.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		pVSBlob->Release();
#pragma endregion
		// - END OF VERTEX SHADERS- //

		// -GEOMETRY SHADERS- //
#pragma region GEOSHADERS
		// Create the Base Geometry Shader
		ID3DBlob* pGSBlob = nullptr;

		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "GS", "gs_4_0", &pGSBlob)))
		{
			DebugBreak();
			pGSBlob->Release();
			return;
		}

		if (FAILED(dev->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &geoshader)))
		{
			DebugBreak();
			pGSBlob->Release();
			return;
		}

		pGSBlob->Release();
#pragma endregion
		// -END OF GEOMETRY SHADERS- //

		// -PIXEL SHADERS- //
#pragma region PIXELSHADERS
		// Compile the pixel shaders
		ID3DBlob* pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "PS", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return;
		}

		// Create the pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, PS_MAIN.GetAddressOf())))
		{
			pPSBlob->Release();
			return;
		}

		// Compile the lighting pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "PS_Specular", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return;
		}

		// Create the pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, PS_SPECULAR.GetAddressOf())))
		{
			pPSBlob->Release();
			return;
		}

		// Compile the solidtexture pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "PS_SolidTexture", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			DebugBreak();
			return;
		}

		// Create the solidtexture pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, PS_NOLIGHTS.GetAddressOf())))
		{
			pPSBlob->Release();
			DebugBreak();
			return;
		}

		//Compile skybox pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "SKYBOX_PS", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			DebugBreak();
			return;
		}
		// Create skybox pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, SKBpixelshader.GetAddressOf())))
		{
			pPSBlob->Release();
			DebugBreak();
			return;
		}

		// Compile crosshair pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "PS_Crosshair", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			DebugBreak();
			return;
		}
		// Create skybox pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, PS_CROSSHAIR.GetAddressOf())))
		{
			pPSBlob->Release();
			DebugBreak();
			return;
		}
		pPSBlob->Release();
#pragma endregion
		// -END OF PIXEL SHADERS- //

		// Create the constant buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		if (FAILED(dev->CreateBuffer(&bd, nullptr, constantbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// CREATION OF OBJECTS //
		// Create the plane.
		CreatePlane(dev, con);
		// Create the cube for the skybox.
		CreateInvertedCube(dev, con);
		// Create the crossbow mesh.
		CreateMesh(dev, con, &crossbowMesh->vertexList, &crossbowMesh->indicesList, vertexbuffer, indexbuffer);
		// Create the crosshair
		CreateNDCPlane(dev, con);
		// Create Ballon Mesh;
		CreateMesh(dev, con, &balloonMesh->vertexList, &balloonMesh->indicesList, b_vertexbuffer, b_indexbuffer);

		// TEXTURE LOADING //
		// Load the grass texture
		if (FAILED(CreateDDSTextureFromFile(dev, texturePath, nullptr, planeTextureRV.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Load Mesh Texture
		if (FAILED(CreateDDSTextureFromFile(dev, textureTwoPath, nullptr, crossbowTextureRV.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// SKYBOX Texture
		if (FAILED(CreateDDSTextureFromFile(dev, L"Textures\\LostValley.dds", nullptr, SKBtextureRV.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Load Crosshair Texture
		if (FAILED(CreateDDSTextureFromFile(dev, L"Textures\\crosshair.dds", nullptr, crosshairTextureRV.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Set the textures that won't change.
		con->PSSetShaderResources(0, 1, planeTextureRV.GetAddressOf());
		con->PSSetShaderResources(2, 1, SKBtextureRV.GetAddressOf());
		con->PSSetShaderResources(3, 1, crosshairTextureRV.GetAddressOf());

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		if (FAILED(dev->CreateSamplerState(&sampDesc, samplerLinear.GetAddressOf())))
			return;

		// ~~~~~~~~~~~ //

		// Initialize the world matrix
		g_World = XMMatrixIdentity();

		// Initialize the view matrix
		XMVECTOR Eye = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
		XMVECTOR At = XMVectorSet(0.0f, 2.0f, 0.0f, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		g_View = XMMatrixLookAtLH(Eye, At, Up);

		// Invert the view matrix so we can do user input.
		XMVECTOR det;
		g_View = XMMatrixInverse(&det, g_View);

		// Initialize the projection matrix
		g_Projection = XMMatrixPerspectiveFovLH(1.309f, DrawClass::width / (FLOAT)DrawClass::height, 0.1f, 600.0f);

		// Set-up Lighting Variables
		{
			// Directional Lighting
			lightDir = { 1.0f, -1.0f, 1.0f, 1.0f };
			lightClr = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		con->Release();
		dev->Release();
		return;
	}

	void ResetDeviceContext(ID3D11DeviceContext* con)
	{
		con->VSSetShader(nullptr, nullptr, 0);
		con->VSSetConstantBuffers(0, 1, nullptr);
		con->PSSetShader(nullptr , nullptr, 0);
		con->PSSetConstantBuffers(0, 1, nullptr);
	}

	void Render(UINT flag = 1)
	{
		if (crossbowMesh == nullptr)
			return;

		// Update time
		static float time = 0.0f;

		static ULONGLONG timePerFrame = 0, timeStart = 0;

		ULONGLONG timeCur = GetTickCount64();
		if (timePerFrame == 0)
			timePerFrame = timeCur;
		if (timeStart == 0)
			timeStart = timeCur;
		time = (timeCur - timeStart) / 1000.0f;

		if (time > 6.28)
		{
			timeStart = timeCur;
		}

		// Grab the context and view.
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		d3d11.GetImmediateContext((void**)&con);
		d3d11.GetRenderTargetView((void**)&view);

		// Set Primitive Topology
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		con->IASetInputLayout(input.Get());

		// Constant Buffer to communicate with the shader's values on the GPU
		ConstantBuffer cb;
		cb.mWorld = XMMatrixTranspose(g_World);
		XMVECTOR det;
		cb.mView = XMMatrixTranspose(XMMatrixInverse(&det, g_View));
		cb.mProjection = XMMatrixTranspose(g_Projection);
		// Directional Light [0]
		cb.lightDir = lightDir;
		cb.lightClr = lightClr;
		cb.vOutputColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		cb.time = time;

		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Render the plane
		RenderPlane(con, view, cb);

		// Render the balloons
		RenderBalloons(con, cb, balloonMesh, time);

		// Reset Geometry Shader so it doesn't affect everything else.
		con->GSSetShader(nullptr, 0, 0);

		// Render the Skybox
		{
			RenderSkybox(con, cb);
		}

		// Render the crossbow
		RenderMesh(con, cb, crossbowMesh);

		// Render the crosshair
		RenderCrosshair(con, view, cb);


		timePerFrame = timeCur;

		con->Release();
		view->Release();
	}

	void UserInput()
	{
		if ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
		{
			std::cout << "Pew\n";

			//XMMATRIX w_One = XMMatrixTranslationFromVector(XMLoadFloat4(&balloonPosOne)) * g_View;

			//XMMATRIX w_Two = XMMatrixTranslationFromVector(XMLoadFloat4(&balloonPosTwo));
			//w_One = XMMatrixMultiply(XMMatrixMultiply(w_Two, g_View),g_Projection);
			//XMMATRIX w_Three = XMMatrixTranslationFromVector(XMLoadFloat4(&balloonPosThree)) * XMMatrixScaling(0.3f, 0.3f, 0.3f);

			//float ndcX = ((width / 2.0f)) / (float)(width * 0.5f) - 1.0f;
			//float ndcY = ((height / 2.0f) / (float) (height * 0.5f)) + 1.0f;

			//std::cout << ndcX << " || " << ndcY << '\n';

			//XMFLOAT4 f = (XMFLOAT4&)XMVector4Transform(XMLoadFloat4(&balloonPosOne), b_World[0]);

			//std::cout << f.x << " || " << f.y << '\n';

			//std::cout << XMVectorGetX(w_One.r[0]) << " || " << XMVectorGetX(w_One.r[3]) << "\n";

			//float xNDCB1 = (balloonPosOne.x) / (float)(width * 0.5f) - 1.0f;
			//float yNDCB1 = (balloonPosOne.y) / (float)(height * 0.5f) + 1.0f;
			//float xNDCB2 = (XMVectorGetX(w_Two.r[3])) / (float)(width * 0.5f) - clientTopLeftX - 1;
			//float xNDCB3 = (XMVectorGetX(w_Three.r[3])) / (float)(width * 0.5f) - clientTopLeftX - 1;

			//float distanceX1 = sqrt(pow((xNDCB1 - ndcX), 2) + pow((yNDCB1 - ndcY), 2));
			//float distanceX2 = -xNDCB2 - ndcX;
			//float distanceX3 = distanceX1 + distanceX2;

			//float distanceX1 = sqrt(pow((XMVectorGetX(w_One.r[3]) - XMVectorGetX(g_View.r[3])), 2) + pow((XMVectorGetY(w_One.r[3]) - XMVectorGetY(g_View.r[3])), 2));

			//if (distanceX1 < 0.75f)
			//{
			//	std::cout << "POP 1\n";
			//}

			//if (distanceX2 < -33.65 && distanceX2 > -33.75)
			//{
			//	std::cout << "POP 2\n";
			//}

			//if (distanceX3 < -33.75 && distanceX3 > -33.90)
			//{
			//	std::cout << "POP 3\n";
			//}

			//std::cout << distanceX1 << " && " << "c" << "\n";
		}

		// Look around movement
		if ((GetKeyState(VK_RBUTTON) & 0x100) != 0)
		{
			POINT cursorPos;
			GetCursorPos(&cursorPos);

			unsigned int clientPosX, clientPosY;
			win.GetX(clientPosX);
			win.GetY(clientPosY);
			unsigned int cosX = clientPosX + (width / 2);
			unsigned int cosY = clientPosY + (height / 2);

			int diffX = (cosX - cursorPos.x);
			int diffY = (cosY - cursorPos.y);

			// Block input outside of 125 pixels away from center.
			if (abs(diffX) < 125 && abs(diffY) < 125)
			{
				// Create the rotation matrix based on mouse input.
				XMMATRIX rot = XMMatrixRotationRollPitchYaw(-diffY / 150.0f, -diffX / 150.0f, 0);

				g_View = XMMatrixMultiply(rot, g_View);

				XMVECTOR vExistingZ = g_View.r[2];
				// Parallel to the world's horizon 
				XMVECTOR vNewX = XMVector3Cross(g_World.r[1], vExistingZ);
				XMVECTOR vNewY = XMVector3Cross(vExistingZ, vNewX);
				vExistingZ = XMVector3Normalize(vExistingZ);
				vNewY = XMVector3Normalize(vNewY);
				vNewX = XMVector3Normalize(vNewX);

				XMMATRIX newView = {
					XMVectorGetX(vNewX), XMVectorGetY(vNewX), XMVectorGetZ(vNewX), XMVectorGetW(g_View.r[0]),
					XMVectorGetX(vNewY), XMVectorGetY(vNewY), XMVectorGetZ(vNewY), XMVectorGetW(g_View.r[1]),
					XMVectorGetX(vExistingZ), XMVectorGetY(vExistingZ), XMVectorGetZ(vExistingZ), XMVectorGetW(g_View.r[2]),
					XMVectorGetX(g_View.r[3]), XMVectorGetY(g_View.r[3]), XMVectorGetZ(g_View.r[3]), XMVectorGetW(g_View.r[3])
				};

				g_View = newView;
			}

			//Set it back to the center
			SetCursorPos(cosX, cosY);
		}

		// Since the view matrix is yet to be transposed until it is rendered, I can use it for moving the camera.
		if (GetAsyncKeyState('W'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0.05f, 1
			};
			g_View = XMMatrixMultiply(translate, g_View);
		}

		if (GetAsyncKeyState('S'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, -0.05f, 1
			};
			g_View = XMMatrixMultiply(translate, g_View);
		}

		if (GetAsyncKeyState('A'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			-0.05f, 0, 0, 1
			};
			g_View = XMMatrixMultiply(translate, g_View);
		}

		if (GetAsyncKeyState('D'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0.05f, 0, 0, 1
			};
			g_View = XMMatrixMultiply(translate, g_View);
		}

		XMMATRIX newView = {
			XMVectorGetX(g_View.r[0]), XMVectorGetY(g_View.r[0]), XMVectorGetZ(g_View.r[0]), XMVectorGetW(g_View.r[0]),
			XMVectorGetX(g_View.r[1]), XMVectorGetY(g_View.r[1]), XMVectorGetZ(g_View.r[1]), XMVectorGetW(g_View.r[1]),
			XMVectorGetX(g_View.r[2]), XMVectorGetY(g_View.r[2]), XMVectorGetZ(g_View.r[2]), XMVectorGetW(g_View.r[2]),
			XMVectorGetX(g_View.r[3]), 1.0f, XMVectorGetZ(g_View.r[3]), XMVectorGetW(g_View.r[3])
		};

		g_View = newView;
	}
};