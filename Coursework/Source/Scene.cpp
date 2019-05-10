
//
// Scene.cpp
//

#include <stdafx.h>
#include <string.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <System.h>
#include <DirectXTK\DDSTextureLoader.h>
#include <DirectXTK\WICTextureLoader.h>
#include <CGDClock.h>
#include <Scene.h>
#include <ctime> 
#include <Effect.h>
#include <VertexStructures.h>
#include <Texture.h>

using namespace std;
using namespace DirectX;
using namespace DirectX::PackedVector;

//
// Methods to handle initialisation, update and rendering of the scene
HRESULT Scene::rebuildViewport(){
	// Binds the render target view and depth/stencil view to the pipeline.
	// Sets up viewport for the main window (wndHandle) 
	// Called at initialisation or in response to window resize
	ID3D11DeviceContext *context = system->getDeviceContext();
	if (!context)
		return E_FAIL;
	// Bind the render target view and depth/stencil view to the pipeline.
	ID3D11RenderTargetView* renderTargetView = system->getBackBufferRTV();
	context->OMSetRenderTargets(1, &renderTargetView, system->getDepthStencil());
	// Setup viewport for the main window (wndHandle)
	RECT clientRect;
	GetClientRect(wndHandle, &clientRect);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(clientRect.right - clientRect.left);
	viewport.Height = static_cast<FLOAT>(clientRect.bottom - clientRect.top);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//Set Viewport
	context->RSSetViewports(1, &viewport);
	return S_OK;
}

// Main resource setup for the application.  These are setup around a given Direct3D device.
HRESULT Scene::initialiseSceneResources() {

	srand(time(NULL));
	ID3D11DeviceContext *context = system->getDeviceContext();
	ID3D11Device *device = system->getDevice();
	if (!device)
		return E_FAIL;
	// Set up viewport for the main window (wndHandle) 
	rebuildViewport();

	// Setup main effects (pipeline shaders, states etc)

	// The Effect class is a helper class similar to the depricated DX9 Effect. It stores pipeline shaders, pipeline states  etc and binds them to setup the pipeline to render with a particular Effect. The constructor requires that at least shaders are provided along a description of the vertex structure.
	Effect *basicColourEffect = new Effect(device, "Shaders\\cso\\basic_colour_vs.cso", "Shaders\\cso\\basic_colour_ps.cso", basicVertexDesc, ARRAYSIZE(basicVertexDesc));
	Effect *basicLightingEffect = new Effect(device, "Shaders\\cso\\basic_lighting_vs.cso", "Shaders\\cso\\basic_colour_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	Effect *perPixelLightingEffect = new Effect(device, "Shaders\\cso\\per_pixel_lighting_vs.cso", "Shaders\\cso\\per_pixel_lighting_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	Effect *waterEffect = new Effect(device, "Shaders\\cso\\ocean_vs.cso", "Shaders\\cso\\ocean_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	Effect *reflectionEffect = new Effect(device, "Shaders\\cso\\reflection_map_VS.cso", "Shaders\\cso\\reflection_map_PS.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	Effect *skyBoxEffect = new Effect(device, "Shaders\\cso\\sky_box_vs.cso", "Shaders\\cso\\sky_box_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	Effect *grassEffect = new Effect(device, "Shaders\\cso\\grass_vs.cso", "Shaders\\cso\\grass_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	Effect* treeEffect = new Effect(device, "Shaders\\cso\\tree_vs.cso", "Shaders\\cso\\tree_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	Effect* fireEffect = new Effect(device, "Shaders\\cso\\fire_vs.cso", "Shaders\\cso\\fire_ps.cso", particleVertexDesc, ARRAYSIZE(particleVertexDesc));
	Effect* smokeEffect = new Effect(device, "Shaders\\cso\\fire_vs.cso", "Shaders\\cso\\fire_ps.cso", particleVertexDesc, ARRAYSIZE(particleVertexDesc));
	Effect* normalMapEffect = new Effect(device, "Shaders\\cso\\normal_map_ppl_vs.cso", "Shaders\\cso\\normal_map_ppl_ps.cso", extVertexDesc, ARRAYSIZE(extVertexDesc));
	// Add Code Here ( Load reflection_map_vs.cso and reflection_map_ps.cso  )

	// The Effect class constructor sets default depth/stencil, rasteriser and blend states
	// The Effect binds these states to the pipeline whenever an object using the effect is rendered
	// We can customise states if required
	ID3D11DepthStencilState *skyBoxDSState = skyBoxEffect->getDepthStencilState();
	D3D11_DEPTH_STENCIL_DESC skyBoxDSDesc;
	skyBoxDSState->GetDesc(&skyBoxDSDesc);
	skyBoxDSDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	skyBoxDSState->Release(); device->CreateDepthStencilState(&skyBoxDSDesc, &skyBoxDSState);
	skyBoxEffect->setDepthStencilState(skyBoxDSState);

	//grass blending
	ID3D11BlendState* transparencyBlendState = grassEffect->getBlendState();
	D3D11_BLEND_DESC transparencyBlendDesc;
	ZeroMemory(&transparencyBlendDesc, sizeof(D3D11_BLEND_DESC));
	transparencyBlendState->GetDesc(&transparencyBlendDesc);

	transparencyBlendDesc.AlphaToCoverageEnable = TRUE;
	transparencyBlendDesc.IndependentBlendEnable = FALSE;
	transparencyBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	transparencyBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparencyBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparencyBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	transparencyBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparencyBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparencyBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	transparencyBlendState->Release(); device->CreateBlendState(&transparencyBlendDesc, &transparencyBlendState);

	grassEffect->setBlendState(transparencyBlendState);
	treeEffect->setBlendState(transparencyBlendState);
	waterEffect->setBlendState(transparencyBlendState);

	ID3D11DepthStencilState* grassDSstate = grassEffect->getDepthStencilState();
	D3D11_DEPTH_STENCIL_DESC	dsDesc;// Setup default depth-stencil descriptor
	grassDSstate->GetDesc(&dsDesc);
	//dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	// Create custom grass depth-stencil state object
	grassDSstate->Release(); device->CreateDepthStencilState(&dsDesc, &grassDSstate);
	grassEffect->setDepthStencilState(grassDSstate);


	//Fire Customisation
	ID3D11BlendState* fireBlendState = fireEffect->getBlendState();
	D3D11_BLEND_DESC fireBlendDesc;
	fireBlendState->GetDesc(&fireBlendDesc);
	fireBlendDesc.AlphaToCoverageEnable = FALSE; // Use pixel coverage info from rasteriser 
	//Enable Alpha Blending
	fireBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	fireBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	fireBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	device->CreateBlendState(&fireBlendDesc, &fireBlendState);

	fireBlendState->Release(); device->CreateBlendState(&fireBlendDesc, &fireBlendState);
	fireEffect->setBlendState(fireBlendState);

	// Customise fireEffect depth stencil state
	// (depth-stencil state is initialised to default in effect constructor)
	// Get the default depth-stencil state object 
	ID3D11DepthStencilState* fireDSstate = fireEffect->getDepthStencilState();
	D3D11_DEPTH_STENCIL_DESC	fireDsDesc;// Setup default depth-stencil descriptor
	fireDSstate->GetDesc(&fireDsDesc);

	// Disable Depth Writing 
	fireDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	device->CreateDepthStencilState(&fireDsDesc, &fireDSstate);

	// Create custom grass depth-stencil state object
	fireDSstate->Release(); device->CreateDepthStencilState(&fireDsDesc, &fireDSstate);
	fireEffect->setDepthStencilState(fireDSstate);

	//Smoke Customisation
	ID3D11BlendState* smokeBlendState = fireEffect->getBlendState();
	D3D11_BLEND_DESC smokeBlendDesc;
	smokeBlendState->GetDesc(&smokeBlendDesc);
	smokeBlendDesc.AlphaToCoverageEnable = FALSE; // Use pixel coverage info from rasteriser 
	//Enable Alpha Blending
	smokeBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	smokeBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	smokeBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	device->CreateBlendState(&smokeBlendDesc, &smokeBlendState);

	smokeBlendState->Release(); device->CreateBlendState(&smokeBlendDesc, &smokeBlendState);
	smokeEffect->setBlendState(smokeBlendState);

	// Customise fireEffect depth stencil state
	// (depth-stencil state is initialised to default in effect constructor)
	// Get the default depth-stencil state object 
	ID3D11DepthStencilState* smokeDSstate = smokeEffect->getDepthStencilState();
	D3D11_DEPTH_STENCIL_DESC	smokeDsDesc;// Setup default depth-stencil descriptor
	smokeDSstate->GetDesc(&smokeDsDesc);

	// Disable Depth Writing 
	smokeDsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	device->CreateDepthStencilState(&smokeDsDesc, &smokeDSstate);

	// Create custom grass depth-stencil state object
	smokeDSstate->Release(); device->CreateDepthStencilState(&smokeDsDesc, &smokeDSstate);
	smokeEffect->setDepthStencilState(smokeDSstate);


	// Setup Textures
	// The Texture class is a helper class to load textures
	Texture* cubeDayTexture = new Texture(device, L"Resources\\Textures\\grassenvmap1024.dds");
	Texture* wavesTexture = new Texture(device, L"Resources\\Textures\\Waves.dds");
	Texture* heightMap = new Texture(device, L"Resources\\Textures\\heightmap2.bmp");
	Texture* normalMap = new Texture(device, L"Resources\\Textures\\normalmap.bmp");
	Texture* grassTexture = new Texture(device, L"Resources\\Textures\\grass.png");
	Texture* grassAlpha = new Texture(device, L"Resources\\Textures\\grassAlpha.tif");
	Texture* treeTexture = new Texture(device, L"Resources\\Textures\\tree.tif");
	Texture* castleTexture = new Texture(device, L"Resources\\Textures\\castle.jpg");
	Texture* castleNormalTexture = new Texture(device, L"Resources\\Textures\\castle_normal.jpg");
	Texture* fireTexture = new Texture(device, L"Resources\\Textures\\Fire.tif");
	Texture* smokeTexture = new Texture(device, L"Resources\\Textures\\smoke.tif");
	Texture* logsTexture = new Texture(device, L"Resources\\Textures\\logs.jpg");
	Texture* logsNormalTexture = new Texture(device, L"Resources\\Textures\\logs_normal.jpg");
	// The BaseModel class supports multitexturing and the constructor takes a pointer to an array of shader resource views of textures. 
	// Even if we only need 1 texture/shader resource view for an effect we still need to create an array.
	ID3D11ShaderResourceView *skyBoxTextureArray[] = { cubeDayTexture->getShaderResourceView()};
	ID3D11ShaderResourceView *waterTextureArray[] = { wavesTexture->getShaderResourceView(), cubeDayTexture->getShaderResourceView()};
	ID3D11ShaderResourceView *grassTextureArray[] = { grassTexture->getShaderResourceView(), grassAlpha->getShaderResourceView() };
	ID3D11ShaderResourceView *treeTextureArray[] = { treeTexture->getShaderResourceView() };
	ID3D11ShaderResourceView* castleTextureArray[] = { castleTexture->getShaderResourceView(), castleNormalTexture->getShaderResourceView() };
	ID3D11ShaderResourceView* fireTextureArray[] = { fireTexture->getShaderResourceView() };
	ID3D11ShaderResourceView* smokeTextureArray[] = { smokeTexture->getShaderResourceView() };
	ID3D11ShaderResourceView* logsTextureArray[] = { logsTexture->getShaderResourceView() , logsNormalTexture->getShaderResourceView()};

	// Setup Objects - the object below are derived from the Base model class
	// The constructors for all objects derived from BaseModel require at least a valid pointer to the main DirectX device
	// And a valid effect with a vertex shader input structure that matches the object vertex structure.
	// In addition to the Texture array pointer mentioned above an additional optional parameters of the BaseModel class are a pointer to an array of Materials along with an int that contains the number of Materials
	// The baseModel class now manages a CBuffer containing model/world matrix properties. It has methods to update the cbuffers if the model/world changes 
	// The render methods of the objects attatch the world/model Cbuffers to the pipeline at slot b0 for vertex and pixel shaders
	
	//MATERIALS
	Material mattMaterial;
	mattMaterial.setSpecular(XMCOLOR(0, 0, 0, 0));
	Material* mattMaterialArr[] = { &mattMaterial };


	//OBJECTS
	box = new Box(device, skyBoxEffect, NULL, 0, skyBoxTextureArray,1);
	box->setWorldMatrix(box->getWorldMatrix()*XMMatrixScaling(4000, 4000, 4000));
	box->update(context);

	orb = new  Model(device, wstring(L"Resources\\Models\\sphere.3ds"), reflectionEffect, NULL, 0, skyBoxTextureArray, 1);
	orb->setWorldMatrix(orb->getWorldMatrix()*XMMatrixTranslation(0, 2, 0));
	orb->update(context);

	water = new Grid(500, 500, device, waterEffect, NULL, 0, waterTextureArray, 2);
	water->setWorldMatrix(orb->getWorldMatrix()*XMMatrixTranslation(-250, -2, -250));
	water->update(context);

	castle = new  Model(device, wstring(L"Resources\\Models\\castle.3ds"), normalMapEffect, mattMaterialArr, 1, castleTextureArray, 2);
	castle->setWorldMatrix(castle->getWorldMatrix()* XMMatrixScaling(10, 10, 10)* XMMatrixRotationY(XMConvertToRadians(rand() % 360))* XMMatrixTranslation(-40, 9, 0));
	castle->update(context);

	terrain = new Terrain(device, context, 500, 500, heightMap->getTexture(), normalMap->getTexture(), grassEffect, NULL, 0, grassTextureArray, 2);
	terrain->setWorldMatrix(terrain->getWorldMatrix() * XMMatrixScaling(1, 20, 1) * XMMatrixTranslation(-250, -2, -250));
	terrain->update(context);

	logs = new  Model(device, wstring(L"Resources\\Models\\logs.obj"), normalMapEffect, mattMaterialArr, 1, logsTextureArray, 2);
	logs->setWorldMatrix(logs->getWorldMatrix()* XMMatrixScaling(0.005, 0.005, 0.005)* XMMatrixRotationX(XMConvertToRadians(-90))* XMMatrixTranslation(-40, 9, 20));
	logs->update(context);

	fire = new ParticleSystem(device, fireEffect, NULL, 0, fireTextureArray, 1);
	fire->setWorldMatrix(fire->getWorldMatrix()* XMMatrixScaling(4, 5, 4)* XMMatrixTranslation(-40, 11, 20));
	fire->update(context);

	smoke = new ParticleSystem(device, smokeEffect, NULL, 0, smokeTextureArray, 1);
	smoke->setWorldMatrix(smoke->getWorldMatrix()* XMMatrixScaling(7, 10, 7)* XMMatrixTranslation(-40, 14, 20));
	smoke->update(context);

	int tempRandomCounter = 0;
	while (tempRandomCounter < numOfTrees) {
		XMFLOAT3 temp;
		temp.x = rand() % 480 - 240;
		temp.z = rand() % 480 - 240;
		temp.y = terrain->CalculateYValueWorld(temp.x, temp.z);
		while((temp.y < treeHeight) || ((temp.x > -90 && temp.x < 3 ) && (temp.z > -60 && temp.z < 70)))
		{
				temp.x = rand() % 480 - 240;
				temp.y = rand() % 480 - 240;
				temp.y = terrain->CalculateYValueWorld(temp.x, temp.z);
		}
		trees[tempRandomCounter] = new  Model(device, wstring(L"Resources\\Models\\tree.3ds"), treeEffect, NULL, 0, treeTextureArray, 1);
		float scale = rand() % 6 + 1;
		trees[tempRandomCounter]->setWorldMatrix(trees[tempRandomCounter]->getWorldMatrix()* XMMatrixScaling(scale, scale, scale)* XMMatrixRotationY(XMConvertToRadians(rand() % 360))* XMMatrixTranslation(temp.x, temp.y + 0.1f, temp.z));
		trees[tempRandomCounter]->update(context);
		tempRandomCounter++;
	}

	// Setup a camera
	// The LookAtCamera is derived from the base Camera class. The constructor for the Camera class requires a valid pointer to the main DirectX device
	// and and 3 vectors to define the initial position, up vector and target for the camera.
	// The camera class  manages a Cbuffer containing view/projection matrix properties. It has methods to update the cbuffers if the camera moves changes  
	// The camera constructor and update methods also attaches the camera CBuffer to the pipeline at slot b1 for vertex and pixel shaders
	mainCamera =  new FirstPersonCamera(device, XMVectorSet(0.0, 9.0, -10.0, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), XMVectorZero());


	// Add a CBuffer to store light properties - you might consider creating a Light Class to manage this CBuffer
	// Allocate 16 byte aligned block of memory for "main memory" copy of cBufferLight
	cBufferLightCPU = (CBufferLight *)_aligned_malloc(sizeof(CBufferLight), 16);

	// Fill out cBufferLightCPU
	cBufferLightCPU->lightVec = XMFLOAT4(-5.0, 2.0, 5.0, 1.0);
	cBufferLightCPU->lightAmbient = XMFLOAT4(0.2, 0.2, 0.2, 1.0);
	cBufferLightCPU->lightDiffuse = XMFLOAT4(0.7, 0.7, 0.7, 1.0);
	cBufferLightCPU->lightSpecular = XMFLOAT4(0.5, 0.5, 0.5, 0.5);

	// Create GPU resource memory copy of cBufferLight
	// fill out description (Note if we want to update the CBuffer we need  D3D11_CPU_ACCESS_WRITE)
	D3D11_BUFFER_DESC cbufferDesc;
	D3D11_SUBRESOURCE_DATA cbufferInitData;
	ZeroMemory(&cbufferDesc, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&cbufferInitData, sizeof(D3D11_SUBRESOURCE_DATA));

	cbufferDesc.ByteWidth = sizeof(CBufferLight);
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInitData.pSysMem = cBufferLightCPU;// Initialise GPU CBuffer with data from CPU CBuffer

	HRESULT hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData,
		&cBufferLightGPU);

	// We dont strictly need to call map here as the GPU CBuffer was initialised from the CPU CBuffer at creation.
	// However if changes are made to the CPU CBuffer during update the we need to copy the data to the GPU CBuffer 
	// using the mapCbuffer helper function provided the in Util.h   

	mapCbuffer(context, cBufferLightCPU, cBufferLightGPU, sizeof(CBufferLight));
	context->VSSetConstantBuffers(2, 1, &cBufferLightGPU);// Attach CBufferLightGPU to register b2 for the vertex shader. Not strictly needed as our vertex shader doesnt require access to this CBuffer
	context->PSSetConstantBuffers(2, 1, &cBufferLightGPU);// Attach CBufferLightGPU to register b2 for the Pixel shader.

	// Add a Cbuffer to stor world/scene properties
	// Allocate 16 byte aligned block of memory for "main memory" copy of cBufferLight
	cBufferSceneCPU = (CBufferScene *)_aligned_malloc(sizeof(CBufferScene), 16);

	// Fill out cBufferSceneCPU
	cBufferSceneCPU->windDir = XMFLOAT4(1, 0, 0, 1);
	cBufferSceneCPU->Time = 0.0;
	cBufferSceneCPU->grassHeight = 0.0;
	
	cbufferInitData.pSysMem = cBufferSceneCPU;// Initialise GPU CBuffer with data from CPU CBuffer
	cbufferDesc.ByteWidth = sizeof(CBufferScene);

	hr = device->CreateBuffer(&cbufferDesc, &cbufferInitData, &cBufferSceneGPU);

	mapCbuffer(context, cBufferSceneCPU, cBufferSceneGPU, sizeof(CBufferScene));
	context->VSSetConstantBuffers(3, 1, &cBufferSceneGPU);// Attach CBufferSceneGPU to register b3 for the vertex shader. Not strictly needed as our vertex shader doesnt require access to this CBuffer
	context->PSSetConstantBuffers(3, 1, &cBufferSceneGPU);// Attach CBufferSceneGPU to register b3 for the Pixel shader
	
	return S_OK;
}

// Update scene state (perform animations etc)
HRESULT Scene::updateScene(ID3D11DeviceContext *context,Camera *camera) {

	// mainClock is a helper class to manage game time data
	mainClock->tick();
	double dT = mainClock->gameTimeDelta();
	double gT = mainClock->gameTimeElapsed();
	//cout << "Game time Elapsed= " << gT << " seconds" << endl;

	// If the CPU CBuffer contents are changed then the changes need to be copied to GPU CBuffer with the mapCbuffer helper function
	mainCamera->update(context);

	// Update the scene time as it is needed to animate the water
	cBufferSceneCPU->Time = gT;
	mapCbuffer(context, cBufferSceneCPU, cBufferSceneGPU, sizeof(CBufferScene));
	cout << "AVG SPF - " << mainClock->averageSPF() << endl;
	cout << "AVG FPS - " << mainClock->averageFPS() << endl; 
	return S_OK;
}

// Render scene
HRESULT Scene::renderScene() {

	ID3D11DeviceContext *context = system->getDeviceContext();
	
	// Validate window and D3D context
	if (isMinimised() || !context)
		return E_FAIL;
	
	// Clear the screen
	static const FLOAT clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	context->ClearRenderTargetView(system->getBackBufferRTV(), clearColor);
	context->ClearDepthStencilView(system->getDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render Scene objects
	
	// Render SkyBox
	if (box)
		box->render(context);

	// Render Orb
	if (orb)
		orb->render(context);

	// Render Orb
	if (water)
		water->render(context);
	if (castle)
		castle->render(context);

	//Render Terrain
	if (terrain)
		DrawGrass(context);
	// Render tree
	for (int i = 0; i < numOfTrees; i++) {
		if (trees[i])
			trees[i]->render(context);
	}
	// Render logs
	if (logs)
		logs->render(context);

	// Render fire
	if (fire)
		fire->render(context);

	if (smoke)
	{
		cBufferSceneCPU->Time *= 0.5;
		mapCbuffer(context, cBufferSceneCPU, cBufferSceneGPU, sizeof(CBufferScene));
		smoke->render(context);
	}

	updateCameraInput();
	// Present current frame to the screen
	HRESULT hr = system->presentBackBuffer();

	return S_OK;
}

//
// Event handling methods
//
// Process mouse move with the left button held down

void Scene::DrawGrass(ID3D11DeviceContext* context)
{
	// Draw the Grass
	if (terrain) {

		// Render grass layers from base to tip
		for (int i = 0; i < numGrassPasses; i++)
		{
			cBufferSceneCPU->grassHeight = (grassLength / numGrassPasses) * i;
			mapCbuffer(context, cBufferSceneCPU, cBufferSceneGPU, sizeof(CBufferScene));
			terrain->render(context);
		}
	}
}
void Scene::handleMouseLDrag(const POINT& disp) {
	mainCamera->updateRotation((float)+disp.x * 0.007f, (float)+disp.y * 0.007f);
}
// Process mouse wheel movement
void Scene::handleMouseWheel(const short zDelta) {
	//LookAtCamera

	//if (zDelta<0)
	//	mainCamera->zoomCamera(1.2f);
	//else if (zDelta>0)
	//	mainCamera->zoomCamera(0.9f);
	//cout << "zoom" << endl;
	//FirstPersonCamera
	//mainCamera->move(zDelta*0.01);
}

// Process key down event.  keyCode indicates the key pressed while extKeyFlags indicates the extended key status at the time of the key down event (see http://msdn.microsoft.com/en-gb/library/windows/desktop/ms646280%28v=vs.85%29.aspx).


void Scene::updateCameraInput() {

	float spf = mainClock->averageSPF();
	float speed = 0.0f;
	if (spf < 1000) {
		speed = 15.0f* spf;
	}

	if (aKey)
	{
		mainCamera->moveLeftRight -= speed;
	}
	if (dKey)
	{
		mainCamera->moveLeftRight += speed;
	}
	if (wKey)
	{
		mainCamera->moveBackForward += speed;
	}
	if (sKey)
	{
		mainCamera->moveBackForward -= speed;
	}
	if (cKey)
	{
		mainCamera->moveUp -= speed;
	}
	if (SpaceKey)
	{
		mainCamera->moveUp += speed;
	}
	mainCamera->UpdateCamera();
}
void Scene::handleKeyDown(const WPARAM keyCode, const LPARAM extKeyFlags) {
	if (keyCode == 87) wKey = true;
	if (keyCode == 83) sKey = true;
	if (keyCode == 65) aKey = true;
	if (keyCode == 68) dKey = true;
	if (keyCode == 32) SpaceKey = true;
	if (keyCode == 67) cKey = true;
}
// Process key up event.  keyCode indicates the key released while extKeyFlags indicates the extended key status at the time of the key up event (see http://msdn.microsoft.com/en-us/library/windows/desktop/ms646281%28v=vs.85%29.aspx).
void Scene::handleKeyUp(const WPARAM keyCode, const LPARAM extKeyFlags) {

	if (keyCode == 87) wKey = false;
	if (keyCode == 83) sKey = false;
	if (keyCode == 65) aKey = false;
	if (keyCode == 68) dKey = false;
	if (keyCode == 32) SpaceKey = false;
	if (keyCode == 67) cKey = false;

}

// Clock handling methods
void Scene::startClock() {
	mainClock->start();
}

void Scene::stopClock() {
	mainClock->stop();
}

void Scene::reportTimingData() {

	cout << "Actual time elapsed = " << mainClock->actualTimeElapsed() << endl;
	cout << "Game time elapsed = " << mainClock->gameTimeElapsed() << endl << endl;
	mainClock->reportTimingData();
}

// Private constructor
Scene::Scene(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc) {
	try
	{
		// 1. Register window class for main DirectX window
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_CROSS);
		wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = wndClassName;
		wcex.hIconSm = NULL;
		if (!RegisterClassEx(&wcex))
			throw exception("Cannot register window class for Scene HWND");
		// 2. Store instance handle in our global variable
		hInst = hInstance;
		// 3. Setup window rect and resize according to set styles
		RECT		windowRect;
		windowRect.left = 0;
		windowRect.right = _width;
		windowRect.top = 0;
		windowRect.bottom = _height;
		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);
		// 4. Create and validate the main window handle
		wndHandle = CreateWindowEx(dwExStyle, wndClassName, wndTitle, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 500, 500, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, hInst, this);
		if (!wndHandle)
			throw exception("Cannot create main window handle");
		ShowWindow(wndHandle, nCmdShow);
		UpdateWindow(wndHandle);
		SetFocus(wndHandle);
		// 5. Create DirectX host environment (associated with main application wnd)
		system = System::CreateDirectXSystem(wndHandle);
		if (!system)
			throw exception("Cannot create Direct3D device and context model");
		// 6. Setup application-specific objects
		HRESULT hr = initialiseSceneResources();
		if (!SUCCEEDED(hr))
			throw exception("Cannot initalise scene resources");
		// 7. Create main clock / FPS timer (do this last with deferred start of 3 seconds so min FPS / SPF are not skewed by start-up events firing and taking CPU cycles).
		mainClock = CGDClock::CreateClock(string("mainClock"), 3.0f);
		if (!mainClock)
			throw exception("Cannot create main clock / timer");
	}
	catch (exception &e)
	{
		cout << e.what() << endl;
		// Re-throw exception
		throw;
	}
}

// Helper function to call updateScene followed by renderScene
HRESULT Scene::updateAndRenderScene() {
	ID3D11DeviceContext *context = system->getDeviceContext();
	HRESULT hr = updateScene(context, (Camera*)mainCamera);
	if (SUCCEEDED(hr))
		hr = renderScene();
	return hr;
}

// Return TRUE if the window is in a minimised state, FALSE otherwise
BOOL Scene::isMinimised() {

	WINDOWPLACEMENT				wp;

	ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	wp.length = sizeof(WINDOWPLACEMENT);

	return (GetWindowPlacement(wndHandle, &wp) != 0 && wp.showCmd == SW_SHOWMINIMIZED);
}

//
// Public interface implementation
//
// Method to create the main Scene instance
Scene* Scene::CreateScene(const LONG _width, const LONG _height, const wchar_t* wndClassName, const wchar_t* wndTitle, int nCmdShow, HINSTANCE hInstance, WNDPROC WndProc) {
	static bool _scene_created = false;
	Scene *system = nullptr;
	if (!_scene_created) {
		system = new Scene(_width, _height, wndClassName, wndTitle, nCmdShow, hInstance, WndProc);
		if (system)
			_scene_created = true;
	}
	return system;
}

// Destructor
Scene::~Scene() {
	//Clean Up
	if (wndHandle)
		DestroyWindow(wndHandle);
}

// Call DestoryWindow on the HWND
void Scene::destoryWindow() {
	if (wndHandle != NULL) {
		HWND hWnd = wndHandle;
		wndHandle = NULL;
		DestroyWindow(hWnd);
	}
}

//
// Private interface implementation
//
// Resize swap chain buffers and update pipeline viewport configurations in response to a window resize event
HRESULT Scene::resizeResources() {
	if (system) {
		// Only process resize if the System *system exists (on initial resize window creation this will not be the case so this branch is ignored)
		HRESULT hr = system->resizeSwapChainBuffers(wndHandle);
		rebuildViewport();
		RECT clientRect;
		GetClientRect(wndHandle, &clientRect);
		if (!isMinimised())
			renderScene();
	}
	return S_OK;
}

