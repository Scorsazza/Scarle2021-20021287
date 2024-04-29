//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include <time.h>

#include <iostream>

//Scarle Headers
#include "GameData.h"
#include "GameState.h"
#include "DrawData.h"
#include "DrawData2D.h"
#include "ObjectList.h"

#include "CMOGO.h"
#include <DirectXCollision.h>
#include <random>

#include "Collision.h"
#include "Enemy.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
	m_window(nullptr),
	m_outputWidth(1080),
	m_outputHeight(1920),
	m_featureLevel(D3D_FEATURE_LEVEL_11_0)
{
}
void Game::CreateWall(const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& scale, float rotationY, const std::string& modelName)
{
	CMOGO* wall = new CMOGO(modelName, m_d3dDevice.Get(), m_fxFactory);
	wall->SetPos(position);
	wall->SetScale(scale);
	wall->SetYaw(rotationY);

	m_GameObjects.push_back(wall);
	m_ColliderObjects.push_back(wall);
}

void Game::CreateFloor(const DirectX::SimpleMath::Vector3& center, float width, float length, float wallThickness)
{
	DirectX::SimpleMath::Vector3 floorScale = DirectX::SimpleMath::Vector3(width, wallThickness, length);
	DirectX::SimpleMath::Vector3 floorPosition = center - DirectX::SimpleMath::Vector3(0, wallThickness / 2, 0);

	// Create the floor object similar to the walls, but with different scaling.
	FileVBGO* floor = new FileVBGO("cube", m_d3dDevice.Get());

	floor->SetPos(floorPosition);
	floor->SetScale(floorScale);
	floor->SetYaw(0);

	m_GameObjects.push_back(floor);
}

void Game::GenerateMap() {
	// Create the main room floor
	CreateFloor(Vector3(0, 0, 0), mainRoomSize, mainRoomSize, wallThickness);

	// Create walls around the main room
	CreateWall(Vector3(0, 0, -(mainRoomSize / 2 + wallThickness / 2)), Vector3(mainRoomSize, wallHeight, wallThickness), 0, "wallModel");
	CreateWall(Vector3(0, 0, mainRoomSize / 2 + wallThickness / 2), Vector3(mainRoomSize, wallHeight, wallThickness), 0, "wallModel");

	// Create walls for the left and right without connecting passages
	float doorWidth = 50.0f;
	float sideWallLength = (mainRoomSize - doorWidth) / 2;
	CreateWall(Vector3(-(mainRoomSize / 2 + wallThickness / 2), 0, -sideWallLength / 2 - doorWidth / 2), Vector3(wallThickness, wallHeight, sideWallLength), 0, "wallModel");
	CreateWall(Vector3(-(mainRoomSize / 2 + wallThickness / 2), 0, sideWallLength / 2 + doorWidth / 2), Vector3(wallThickness, wallHeight, sideWallLength), 0, "wallModel");
	CreateWall(Vector3(mainRoomSize / 2 + wallThickness / 2, 0, -sideWallLength / 2 - doorWidth / 2), Vector3(wallThickness, wallHeight, sideWallLength), 0, "wallModel");
	CreateWall(Vector3(mainRoomSize / 2 + wallThickness / 2, 0, sideWallLength / 2 + doorWidth / 2), Vector3(wallThickness, wallHeight, sideWallLength), 0, "wallModel");

	// Create floors and walls for adjacent smaller rooms
	Vector3 leftRoomOffset = Vector3(-(mainRoomSize + smallRoomSize) / 2 - wallThickness, 0, 0);
	Vector3 rightRoomOffset = Vector3((mainRoomSize + smallRoomSize) / 2 + wallThickness, 0, 0);
	CreateFloor(leftRoomOffset, smallRoomSize, smallRoomSize, wallThickness);
	CreateFloor(rightRoomOffset, smallRoomSize, smallRoomSize, wallThickness);
	CreateWall(leftRoomOffset + Vector3(0, 0, -(smallRoomSize / 2 + wallThickness / 2)), Vector3(smallRoomSize, wallHeight, wallThickness), 0, "wallModel");
	CreateWall(leftRoomOffset + Vector3(0, 0, smallRoomSize / 2 + wallThickness / 2), Vector3(smallRoomSize, wallHeight, wallThickness), 0, "wallModel");
	CreateWall(leftRoomOffset + Vector3(-(smallRoomSize / 2 + wallThickness / 2), 0, 0), Vector3(wallThickness, wallHeight, smallRoomSize), 0, "wallModel");
	CreateWall(leftRoomOffset + Vector3(smallRoomSize / 2 + wallThickness / 2, 0, 0), Vector3(wallThickness, wallHeight, smallRoomSize), 0, "wallModel");
	CreateWall(rightRoomOffset + Vector3(0, 0, -(smallRoomSize / 2 + wallThickness / 2)), Vector3(smallRoomSize, wallHeight, wallThickness), 0, "wallModel");
	CreateWall(rightRoomOffset + Vector3(0, 0, smallRoomSize / 2 + wallThickness / 2), Vector3(smallRoomSize, wallHeight, wallThickness), 0, "wallModel");
	CreateWall(rightRoomOffset + Vector3(-(smallRoomSize / 2 + wallThickness / 2), 0, 0), Vector3(wallThickness, wallHeight, smallRoomSize), 0, "wallModel");
	CreateWall(rightRoomOffset + Vector3(smallRoomSize / 2 + wallThickness / 2, 0, 0), Vector3(wallThickness, wallHeight, smallRoomSize), 0, "wallModel");

	float spacing = 10.0f; // Spacing between each coin
	int numCoinsPerRow = static_cast<int>(mainRoomSize / spacing) - 2; // Number of coins per row, minus 2 for edge padding

	GenerateCoin();
}

void Game::GenerateCoin() {
	float elevation = 10.0f; // Elevation from the floor, adjust as needed

	// Define the area around the player where coins can spawn
	float spawnRadius = 50.0f; // Distance from player within which coins can spawn
	float buffer = wallThickness + 5.0f; // Buffer from the walls

	std::random_device rd; // Non-deterministic random number generator
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

	std::uniform_real_distribution<> disAngle(0, 2 * XM_PI);
	std::uniform_real_distribution<> disRadius(0, spawnRadius);

	float angle = disAngle(gen);
	float radius = disRadius(gen);

	float x = radius * cos(angle);
	float z = radius * sin(angle);

	// Make sure the coin doesn't spawn too close to the walls
	x = std::max(std::min(x, mainRoomSize / 2 - buffer), -mainRoomSize / 2 + buffer);
	z = std::max(std::min(z, mainRoomSize / 2 - buffer), -mainRoomSize / 2 + buffer);

	Coin* newCoin = new Coin("coin", m_d3dDevice.Get(), m_fxFactory);
	newCoin->SetPos(Vector3(x, elevation, z));
	m_GameObjects.push_back(newCoin);
	m_ColliderObjects.push_back(newCoin);
}

// Initialise the Direct3D resources required to run.
void Game::Initialize(HWND _window, int _width, int _height)
{
	m_window = _window;
	m_outputWidth = std::max(_width, 1);
	m_outputHeight = std::max(_height, 1);
	m_score = 0;
	CreateDevice();

	CreateResources();

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/

	//seed the random number generator
	srand((UINT)time(NULL));

	//set up keyboard and mouse system
	//documentation here: https://github.com/microsoft/DirectXTK/wiki/Mouse-and-keyboard-input
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(_window);
	m_mouse->SetMode(Mouse::MODE_RELATIVE);
	//Hide the mouse pointer
	ShowCursor(false);

	//create GameData struct and populate its pointers
	m_GD = new GameData;
	m_GD->m_GS = GS_MAIN_MENU;

	//set up systems for 2D rendering
	m_DD2D = new DrawData2D();
	m_DD2D->m_Sprites.reset(new SpriteBatch(m_d3dContext.Get()));
	m_DD2D->m_Font.reset(new SpriteFont(m_d3dDevice.Get(), L"..\\Assets\\italic.spritefont"));
	m_states = new CommonStates(m_d3dDevice.Get());

	//set up DirectXTK Effects system
	m_fxFactory = new EffectFactory(m_d3dDevice.Get());
	//Tell the fxFactory to look to the correct build directory to pull stuff in from
	((EffectFactory*)m_fxFactory)->SetDirectory(L"..\\Assets");
	//init render system for VBGOs
	VBGO::Init(m_d3dDevice.Get());

	//set audio system
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | AudioEngine_Debug;
#endif
	m_audioEngine = std::make_unique<AudioEngine>(eflags);

	//create a set of dummy things to show off the engine

	//create a base light
	m_light = new Light(Vector3(0.0f, 100.0f, 160.0f), Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.4f, 0.1f, 0.1f, 1.0f));
	m_GameObjects.push_back(m_light);

	//find how big my window is to correctly calculate my aspect ratio
	float AR = (float)_width / (float)_height;

	//create a base camera
	m_cam = new Camera(0.25f * XM_PI, AR, 1.0f, 10000.0f, Vector3::UnitY, Vector3::Zero);
	m_cam->SetPos(Vector3(0.0f, 200.0f, 200.0f));
	m_GameObjects.push_back(m_cam);

	//add Player

	Player* pPlayer = new Player("lowpolyghost", m_d3dDevice.Get(), m_fxFactory);
	pPlayer->Reset();  // Reset and place player at a specific location
	m_GameObjects.push_back(pPlayer);
	m_PhysicsObjects.push_back(pPlayer);

	Enemy* pEnemy = new Enemy("lowpolyghost", m_d3dDevice.Get(), m_fxFactory, pPlayer);
	m_GameObjects.push_back(pEnemy);
	m_PhysicsObjects.push_back(pEnemy);
	pEnemy->Activate(false);

	//add a secondary camera

	m_TPScam = new TPSCamera(0.25f * XM_PI, AR, 1.0f, 10000.0f, pPlayer, Vector3::UnitY, Vector3(0.0f, 1.7f, 0.1f)); // Offset y to approximate eye level, small z forward

	m_TPScam->SetPitch(0.0f);
	m_TPScam->SetYaw(0.0f);
	m_GameObjects.push_back(m_TPScam);

	//create DrawData struct and populate its pointers
	m_DD = new DrawData;
	m_DD->m_pd3dImmediateContext = nullptr;
	m_DD->m_states = m_states;
	m_DD->m_cam = m_cam;
	m_DD->m_light = m_light;

	//example basic 2D stuff

	m_scoreText = new TextGO2D("Score: 0");
	m_scoreText->SetPos(Vector2(100, 10));
	m_scoreText->SetColour(Color((float*)&Colors::Yellow));
	m_GameObjects2D.push_back(m_scoreText);



	GenerateMap();
}

// Executes the basic game loop.
void Game::Tick()
{
	m_timer.Tick([&]()
		{
			Update(m_timer);
		});

	Render();
}

// Updates the world.
void Game::ResetGame() {
	// Reset the score
	m_score = 0;
	m_scoreText->SetText("Score: " + std::to_string(m_score));

	// Remove any game over or win text from the screen
	auto itText = std::remove_if(m_GameObjects2D.begin(), m_GameObjects2D.end(), [](GameObject2D* obj) {
		return dynamic_cast<TextGO2D*>(obj) != nullptr;
		});
	m_GameObjects2D.erase(itText, m_GameObjects2D.end());

	// Reset game state
	m_GD->m_GS = GS_PLAY_TPS_CAM;

	// Remove one coin if available
	auto itCoin = std::find_if(m_GameObjects.begin(), m_GameObjects.end(), [](GameObject* obj) {
		return dynamic_cast<Coin*>(obj) != nullptr; // Identify a coin object
		});
	if (itCoin != m_GameObjects.end()) {
		Coin* coin = dynamic_cast<Coin*>(*itCoin);
		if (coin) {
			delete coin; // Properly delete the coin
			m_GameObjects.erase(itCoin); // Remove the coin from the game objects list
			m_ColliderObjects.erase(std::remove(m_ColliderObjects.begin(), m_ColliderObjects.end(), coin), m_ColliderObjects.end()); // Remove from collider objects
		}
	}

	GenerateMap();

	if (std::find(m_GameObjects2D.begin(), m_GameObjects2D.end(), m_scoreText) == m_GameObjects2D.end()) {
		m_GameObjects2D.push_back(m_scoreText);
	}
}

void Game::Update(DX::StepTimer const& _timer)
{
	m_scoreText->SetText("Score: " + std::to_string(m_score));
	if (m_score >= 10 && m_GD->m_GS != GS_WIN) {
		m_GD->m_GS = GS_WIN; // Set game state to win
		for (auto obj : m_PhysicsObjects) {
			Enemy* pEnemy = dynamic_cast<Enemy*>(obj);
			if (pEnemy) {
				pEnemy->Activate(false);
			}
		}
	}

	if ((m_GD->m_GS == GS_WIN || m_GD->m_GS == GS_GAME_OVER) && m_GD->m_KBS_tracker.pressed.Enter) {
		ResetGame();
	}

	float elapsedTime = float(_timer.GetElapsedSeconds());
	m_GD->m_dt = elapsedTime;

	//this will update the audio engine but give us chance to do somehting else if that isn't working
	if (!m_audioEngine->Update())
	{
		if (m_audioEngine->IsCriticalError())
		{
			// We lost the audio device!
		}
	}
	else
	{
		//update sounds playing
		for (list<Sound*>::iterator it = m_Sounds.begin(); it != m_Sounds.end(); it++)
		{
			(*it)->Tick(m_GD);
		}
	}

	ReadInput();
	//upon space bar switch camera state
	//see docs here for what's going on: https://github.com/Microsoft/DirectXTK/wiki/Keyboard
	if (m_GD->m_KBS_tracker.pressed.Space)
	{
	}

	//update all objects
	for (list<GameObject*>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
	{
		(*it)->Tick(m_GD);
	}
	for (list<GameObject2D*>::iterator it = m_GameObjects2D.begin(); it != m_GameObjects2D.end(); it++)
	{
		(*it)->Tick(m_GD);
	}


	CheckCollision();
}

// Draws the scene.
void Game::Render() {
	if (m_timer.GetFrameCount() == 0) {
		return;  // Skip rendering if the update has not been called yet
	}

	Clear();  // Clear the screen with the appropriate background color

	m_DD->m_pd3dImmediateContext = m_d3dContext.Get();
	m_DD->m_cam = m_cam;  // Default camera
	if (m_GD->m_GS == GS_PLAY_TPS_CAM || m_GD->m_GS == GS_WIN) {
		m_DD->m_cam = m_TPScam; // Third person camera for gameplay and win states
	}

	// Handle different game states for specific text output
	if (m_GD->m_GS == GS_MAIN_MENU || m_GD->m_GS == GS_WIN || m_GD->m_GS == GS_GAME_OVER) {
		m_DD2D->m_Sprites->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

		std::string titleText, instructionText;
		Vector2 titlePosition, instructionPosition;

		if (m_GD->m_GS == GS_MAIN_MENU) {
			titleText = "Press Enter to Play!";
			instructionText = "Collect 10 coins without getting caught to win";
			titlePosition = Vector2(m_outputWidth / 2 - 200, m_outputHeight / 2 - 50);
			instructionPosition = Vector2(m_outputWidth / 2 - 450, m_outputHeight / 2 + 10);
		}
		else if (m_GD->m_GS == GS_WIN) {
			titleText = "You Win!";
			instructionText = "Press Enter To Restart";
			titlePosition = Vector2(m_outputWidth / 2 - 200, m_outputHeight / 2 - 20);
			instructionPosition = Vector2(m_outputWidth / 2 - 190, m_outputHeight / 2 + 40);
		}
		else if (m_GD->m_GS == GS_GAME_OVER) {
			titleText = "You have been caught!";
			instructionText = "Press Enter To Restart";
			titlePosition = Vector2(m_outputWidth / 2 - 200, m_outputHeight / 2 - 20);
			instructionPosition = Vector2(m_outputWidth / 2 - 190, m_outputHeight / 2 + 40);
		}

		// Draw title text
		TextGO2D titleGO(titleText);
		titleGO.SetPos(titlePosition);
		titleGO.SetColour(Color((float*)&Colors::White));
		titleGO.Draw(m_DD2D);

		// Draw instructions text
		TextGO2D instructionsGO(instructionText);
		instructionsGO.SetPos(instructionPosition);
		instructionsGO.SetColour(Color((float*)&Colors::White));
		instructionsGO.Draw(m_DD2D);

		m_DD2D->m_Sprites->End();
	}
	else {
		VBGO::UpdateConstantBuffer(m_DD);
		for (auto& gameObject : m_GameObjects) {
			gameObject->Draw(m_DD);
		}
		m_DD2D->m_Sprites->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());
		for (auto& gameObject2D : m_GameObjects2D) {
			gameObject2D->Draw(m_DD2D);
		}
		m_DD2D->m_Sprites->End();
	}

	m_d3dContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
	// Clear the views.
	m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::Black);
	m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	// Set the viewport.
	CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
	m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = m_swapChain->Present(1, 0);

	// If the device was reset we must completely reinitialize the renderer.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		OnDeviceLost();
	}
	else
	{
		DX::ThrowIfFailed(hr);
	}
}

// Message handlers
void Game::OnActivated()
{
	// TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
	// TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
	// TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
	m_timer.ResetElapsedTime();

	// TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int _width, int _height)
{
	m_outputWidth = std::max(_width, 1);
	m_outputHeight = std::max(_height, 1);

	CreateResources();

	// TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& _width, int& _height) const noexcept
{
	// TODO: Change to desired default window size (note minimum size is 320x200).
	_width = 1920;
	_height = 1080;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
	UINT creationFlags = 0;

#ifdef _DEBUG
	//creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	//something missing on the machines in 2Q28
	//this should work!
#endif

	static const D3D_FEATURE_LEVEL featureLevels[] =
	{
		// TODO: Modify for supported Direct3D feature levels
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// Create the DX11 API device object, and get a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	DX::ThrowIfFailed(D3D11CreateDevice(
		nullptr,                            // specify nullptr to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		static_cast<UINT>(std::size(featureLevels)),
		D3D11_SDK_VERSION,
		device.ReleaseAndGetAddressOf(),    // returns the Direct3D device created
		&m_featureLevel,                    // returns feature level of device created
		context.ReleaseAndGetAddressOf()    // returns the device immediate context
	));

#ifndef NDEBUG
	ComPtr<ID3D11Debug> d3dDebug;
	if (SUCCEEDED(device.As(&d3dDebug)))
	{
		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// TODO: Add more message IDs here as needed.
			};
			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
#endif

	DX::ThrowIfFailed(device.As(&m_d3dDevice));
	DX::ThrowIfFailed(context.As(&m_d3dContext));

	// TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_d3dContext->OMSetRenderTargets(static_cast<UINT>(std::size(nullViews)), nullViews, nullptr);
	m_renderTargetView.Reset();
	m_depthStencilView.Reset();
	m_d3dContext->Flush();

	const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
	const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
	const DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	const DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	constexpr UINT backBufferCount = 2;

	// If the swap chain already exists, resize it, otherwise create one.
	if (m_swapChain)
	{
		HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			OnDeviceLost();

			// Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method
			// and correctly set up the new device.
			return;
		}
		else
		{
			DX::ThrowIfFailed(hr);
		}
	}
	else
	{
		// First, retrieve the underlying DXGI Device from the D3D Device.
		ComPtr<IDXGIDevice1> dxgiDevice;
		DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

		// Identify the physical adapter (GPU or card) this device is running on.
		ComPtr<IDXGIAdapter> dxgiAdapter;
		DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

		// And obtain the factory object that created it.
		ComPtr<IDXGIFactory2> dxgiFactory;
		DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

		// Create a descriptor for the swap chain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = backBufferWidth;
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = backBufferFormat;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = backBufferCount;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed = TRUE;

		// Create a SwapChain from a Win32 window.
		DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
			m_d3dDevice.Get(),
			m_window,
			&swapChainDesc,
			&fsSwapChainDesc,
			nullptr,
			m_swapChain.ReleaseAndGetAddressOf()
		));

		// This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
		DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
	}

	// Obtain the backbuffer for this window which will be the final 3D rendertarget.
	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

	// Create a view interface on the rendertarget to use on bind.
	DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

	// Allocate a 2-D surface as the depth/stencil buffer and
	// create a DepthStencil view on this surface to use on bind.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

	ComPtr<ID3D11Texture2D> depthStencil;
	DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

	// TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.

	m_depthStencilView.Reset();
	m_renderTargetView.Reset();
	m_swapChain.Reset();
	m_d3dContext.Reset();
	m_d3dDevice.Reset();

	CreateDevice();

	CreateResources();
}
void Game::StartGame()
{
	m_GD->m_GS = GS_PLAY_TPS_CAM; // Change game state
	for (auto obj : m_GameObjects) {
		Enemy* pEnemy = dynamic_cast<Enemy*>(obj);
		if (pEnemy) {
			pEnemy->Activate(true); // Activate the enemy
		}
	}
}

void Game::ReadInput() {
	m_GD->m_KBS = m_keyboard->GetState();
	m_GD->m_KBS_tracker.Update(m_GD->m_KBS);

	if (m_GD->m_KBS.Escape) {
		ExitGame();
	}

	if (m_GD->m_GS == GS_MAIN_MENU && m_GD->m_KBS_tracker.pressed.Enter) {
		StartGame();
	}

	m_GD->m_MS = m_mouse->GetState();
	RECT window;
	GetWindowRect(m_window, &window);
	SetCursorPos((window.left + window.right) >> 1, (window.bottom + window.top) >> 1);
}

void Game::CheckCollision() {
	Player* pPlayer = nullptr;
	// Find the player in the list of physics objects
	for (auto obj : m_PhysicsObjects) {
		if (dynamic_cast<Player*>(obj)) {
			pPlayer = dynamic_cast<Player*>(obj);
			break;
		}
	}

	if (!pPlayer) return;

	// Iterate through all physics objects and collider objects to check for intersections
	for (auto physObj : m_PhysicsObjects) {
		// Skip the enemy if it's not active
		Enemy* pEnemy = dynamic_cast<Enemy*>(physObj);
		if (pEnemy && !pEnemy->IsActive()) {
			continue;
		}

		for (auto collObj : m_ColliderObjects) {
			if (physObj->Intersects(*collObj)) {
				Coin* coin = dynamic_cast<Coin*>(collObj);
				if (coin && physObj == pPlayer) { // Check if the colliding object is a coin and the physics object is the player
					m_score++;  // Increment score
					delete coin;  // Properly delete the coin
					m_ColliderObjects.erase(std::remove(m_ColliderObjects.begin(), m_ColliderObjects.end(), coin), m_ColliderObjects.end()); // Remove the coin from collider objects
					m_GameObjects.erase(std::remove(m_GameObjects.begin(), m_GameObjects.end(), coin), m_GameObjects.end()); // Remove from game objects
					GenerateCoin(); // Regenerate a new coin
					continue;  // Skip further checks for this coin
				}

				// Calculate the ejection vector for other objects and apply it
				XMFLOAT3 eject_vect = Collision::ejectionCMOGO(*physObj, *collObj);
				physObj->SetPos(physObj->GetPos() - eject_vect);
			}
		}

		// Additional check for collisions between the player and enemies
		if (pEnemy && pPlayer->Intersects(*pEnemy)) {
			m_GD->m_GS = GS_GAME_OVER; // Change game state to GAME OVER
			break;
		}
	}
}