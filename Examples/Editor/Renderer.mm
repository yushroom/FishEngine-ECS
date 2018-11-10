#include "Renderer.h"
#include <FishEditor/OSXGameApp.hpp>
#include <FishEngine.hpp>
#include <FishEditor.hpp>
using namespace FishEngine;
using namespace FishEditor;

extern id<MTLDevice> g_device;
extern MTKView* g_MTKView;
extern GameApp* mainApp;

inline std::string GetglTFSample(const std::string& name)
{
#ifdef __APPLE__
	return "/Users/yushroom/program/github/glTF-Sample-Models/2.0/"
	
#else
	return R"(D:\program\glTF-Sample-Models\2.0\)"
#endif
	+ name + "/glTF-Binary/" + name + ".glb";
}


class TestEditor : public FishEditor::GameApp
{
public:
	void Start() override
	{
		auto path = GetglTFSample("Buggy");
		{
			auto go = m_Scene->CreateGameObject();
			m_Scene->GameObjectAddComponent<Camera>(go);
			go->GetTransform()->SetLocalPosition(0, 0, -10);
			//m_Scene->GameObjectAddComponent<FreeCamera>(go);
			go->name = "Main Camera";
		}
		{
			auto go = m_Scene->CreateGameObject();
			auto t = go->GetTransform();
			auto light = m_Scene->GameObjectAddComponent<Light>(go);
			t->SetLocalEulerAngles(50, -30, 0);
			t->SetLocalPosition(0, 3, 0);
			go->name = "Directional Light";
		}
		GLTFLoadFlags flags;
		flags.loadMateirals = true;
		flags.loadPrimitiveAsSubMesh = true;
		auto rootGO = ModelUtil::FromGLTF(path, flags, m_Scene);
		rootGO->GetTransform()->SetLocalEulerAngles(0, 180, 0);
		rootGO->GetTransform()->SetLocalScale(0.01f);
		{
			auto s = m_Scene->AddSystem<AnimationSystem>();
			s->m_Priority = 999;
			s->m_Enabled = false;
		}
//		m_Scene->AddSystem<DrawSkeletonSystem>();
		
		auto selection = m_EditorScene->GetSingletonComponent<SingletonSelection>();
		selection->selected = rootGO;
		{
			auto cam = Camera::GetEditorCamera();
			assert(cam != nullptr);
			cam->GetGameObject()->GetTransform()->SetLocalPosition(0, 0.756, -2.56);
		}
	}
	
	void Update() override
	{
		m_Scene->time->deltaTime = 0.01666f;
		m_Scene->time->deltaTime = m_Scene->time->deltaTime;
//		NSPoint pos = [NSEvent mouseLocation];
		NSPoint pos = [[g_MTKView window] mouseLocationOutsideOfEventStream];
		float x = (float)pos.x / m_WindowWidth;
		float y = (float)pos.y / m_WindowHeight;
//		y = 1.0f - y;
//		printf("mouse pos: %f %f\n", x, y);
		m_EditorScene->GetSystem<InputSystem>()->SetMousePosition(x, y);
		
		if (m_EditorScene->input->IsButtonReleased(KeyCode::Escape))
		{
			m_GameOver = true;
		}
		
		GameApp::Update();
	}
};

constexpr int MAX_KEYCODES = 256;
static KeyCode keycodes[MAX_KEYCODES];

// see cocoa_init.m and cocoa_window.m in GLFW
static void createKeyTables()
{
	keycodes[0x1D] = KeyCode::Alpha0;
	keycodes[0x12] = KeyCode::Alpha1;
	keycodes[0x13] = KeyCode::Alpha2;
	keycodes[0x14] = KeyCode::Alpha3;
	keycodes[0x15] = KeyCode::Alpha4;
	keycodes[0x17] = KeyCode::Alpha5;
	keycodes[0x16] = KeyCode::Alpha6;
	keycodes[0x1C] = KeyCode::Alpha7;
	keycodes[0x19] = KeyCode::Alpha8;
	keycodes[0x1D] = KeyCode::Alpha9;
	
	keycodes[0x00] = KeyCode::A;
	keycodes[0x0B] = KeyCode::B;
	keycodes[0x08] = KeyCode::C;
	keycodes[0x02] = KeyCode::D;
	keycodes[0x0E] = KeyCode::E;
	keycodes[0x03] = KeyCode::F;
	keycodes[0x05] = KeyCode::G;
	keycodes[0x04] = KeyCode::H;
	keycodes[0x22] = KeyCode::I;
	keycodes[0x26] = KeyCode::J;
	keycodes[0x28] = KeyCode::K;
	keycodes[0x25] = KeyCode::L;
	keycodes[0x2E] = KeyCode::M;
	keycodes[0x2D] = KeyCode::N;
	keycodes[0x1F] = KeyCode::O;
	keycodes[0x23] = KeyCode::P;
	keycodes[0x0C] = KeyCode::Q;
	keycodes[0x0F] = KeyCode::R;
	keycodes[0x01] = KeyCode::S;
	keycodes[0x11] = KeyCode::T;
	keycodes[0x20] = KeyCode::U;
	keycodes[0x09] = KeyCode::V;
	keycodes[0x0D] = KeyCode::W;
	keycodes[0x07] = KeyCode::X;
	keycodes[0x10] = KeyCode::Y;
	keycodes[0x06] = KeyCode::Z;
	
//	keycodes[0x27] = KeyCode::APOSTROPHE;
	keycodes[0x2A] = KeyCode::Backslash;
	keycodes[0x2B] = KeyCode::Comma;
	keycodes[0x18] = KeyCode::Equals;
//	keycodes[0x32] = KeyCode::GRAVE_ACCENT;
	keycodes[0x21] = KeyCode::LeftBracket;
	keycodes[0x1B] = KeyCode::Minus;
	keycodes[0x2F] = KeyCode::Period;
	keycodes[0x1E] = KeyCode::RightBracket;
	keycodes[0x29] = KeyCode::Semicolon;
	keycodes[0x2C] = KeyCode::Slash;
//	keycodes[0x0A] = KeyCode::WORLD_1;
	keycodes[0x33] = KeyCode::Backspace;
	keycodes[0x39] = KeyCode::CapsLock;
	keycodes[0x75] = KeyCode::Delete;
	keycodes[0x7D] = KeyCode::DownArrow;
	keycodes[0x77] = KeyCode::End;
	keycodes[0x24] = KeyCode::Return;
	keycodes[0x35] = KeyCode::Escape;
	
	keycodes[0x7A] = KeyCode::F1;
	keycodes[0x78] = KeyCode::F2;
	keycodes[0x63] = KeyCode::F3;
	keycodes[0x76] = KeyCode::F4;
	keycodes[0x60] = KeyCode::F5;
	keycodes[0x61] = KeyCode::F6;
	keycodes[0x62] = KeyCode::F7;
	keycodes[0x64] = KeyCode::F8;
	keycodes[0x65] = KeyCode::F9;
	keycodes[0x6D] = KeyCode::F10;
	keycodes[0x67] = KeyCode::F11;
	keycodes[0x6F] = KeyCode::F12;
	keycodes[0x69] = KeyCode::F13;
	keycodes[0x6B] = KeyCode::F14;
	keycodes[0x71] = KeyCode::F15;
//	keycodes[0x6A] = KeyCode::F16;
//	keycodes[0x40] = KeyCode::F17;
//	keycodes[0x4F] = KeyCode::F18;
//	keycodes[0x50] = KeyCode::F19;
//	keycodes[0x5A] = KeyCode::F20;
	keycodes[0x73] = KeyCode::Home;
	keycodes[0x72] = KeyCode::Insert;
	keycodes[0x7B] = KeyCode::LeftArrow;
	keycodes[0x3A] = KeyCode::LeftAlt;
	keycodes[0x3B] = KeyCode::LeftControl;
	keycodes[0x38] = KeyCode::LeftShift;
	keycodes[0x37] = KeyCode::LeftCommand;	// left supert
	keycodes[0x6E] = KeyCode::Menu;
	keycodes[0x47] = KeyCode::Numlock;
	keycodes[0x79] = KeyCode::PageDown;
	keycodes[0x74] = KeyCode::PageUp;
	keycodes[0x7C] = KeyCode::RightArrow;
	keycodes[0x3D] = KeyCode::RightAlt;
	keycodes[0x3E] = KeyCode::RightControl;
	keycodes[0x3C] = KeyCode::RightShift;
	keycodes[0x36] = KeyCode::RightCommand;
	keycodes[0x31] = KeyCode::Space;
	keycodes[0x30] = KeyCode::Tab;
	keycodes[0x7E] = KeyCode::UpArrow;
	
	keycodes[0x53] = KeyCode::Keypad1;
	keycodes[0x54] = KeyCode::Keypad2;
	keycodes[0x55] = KeyCode::Keypad3;
	keycodes[0x56] = KeyCode::Keypad4;
	keycodes[0x57] = KeyCode::Keypad5;
	keycodes[0x58] = KeyCode::Keypad6;
	keycodes[0x59] = KeyCode::Keypad7;
	keycodes[0x5B] = KeyCode::Keypad8;
	keycodes[0x5C] = KeyCode::Keypad9;
	keycodes[0x45] = KeyCode::KeypadPlus;	// add
//	keycodes[0x41] = KeyCode::KeypadDECIMAL;
	keycodes[0x4B] = KeyCode::KeypadDivide;
	keycodes[0x4C] = KeyCode::KeypadEnter;
	keycodes[0x51] = KeyCode::KeypadEquals;
	keycodes[0x43] = KeyCode::KeypadMultiply;
	keycodes[0x4E] = KeyCode::KeypadMinus;	// subtract
}

static KeyCode translateKey(unsigned int key)
{
	if (key >= MAX_KEYCODES)
		return KeyCode::None;
	return keycodes[key];
}

// from glfw
// Translates macOS key modifiers into GLFW ones
//
//static int translateFlags(NSUInteger flags)
//{
//	int mods = 0;
//
//	if (flags & NSEventModifierFlagShift)
//	mods |= GLFW_MOD_SHIFT;
//	if (flags & NSEventModifierFlagControl)
//	mods |= GLFW_MOD_CONTROL;
//	if (flags & NSEventModifierFlagOption)
//	mods |= GLFW_MOD_ALT;
//	if (flags & NSEventModifierFlagCommand)
//	mods |= GLFW_MOD_SUPER;
//	if (flags & NSEventModifierFlagCapsLock)
//	mods |= GLFW_MOD_CAPS_LOCK;
//
//	return mods;
//}


@interface Renderer ()
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;

@property TestEditor* gameApp;
@end



@implementation Renderer

-(nonnull instancetype)initWithView:(nonnull MTKView *)view;
{
	self = [super init];
	
	if (self)
	{
		_device = view.device;
		_commandQueue = [_device newCommandQueue];
	}
	
	createKeyTables();

	_gameApp = new TestEditor();
	g_device = _device;
	g_MTKView = view;
	
	_gameApp->Init();
	_gameApp->Resize(800, 600);
	FishEngine::ResetGraphicsAPI(800*2, 600*2);
	_gameApp->Start();
	
	return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view {
	_gameApp->Update();
	if (_gameApp->m_GameOver)
	{
		[view.window close];
	}
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
	_gameApp->Resize(size.width/2, size.height/2);
	FishEngine::ResetGraphicsAPI(size.width, size.height);
}

- (void)handleEvent:(nonnull NSEvent*)event{

}

- (void)handleKeyEvent:(nonnull NSEvent*)event
{
	Scene* scene = _gameApp->GetEditorScene();
	auto s = scene->GetSystem<InputSystem>();
	
//	NSString* characters = [event characters];
//	NSLog(characters);
	
	KeyEvent e;
	if (event.type == NSEventTypeKeyDown)
	{
		e.action = KeyAction::Pressed;
	}
	else if (event.type == NSEventTypeKeyUp)
	{
		e.action = KeyAction::Released;
	}
	e.key = translateKey(event.keyCode);
	s->PostKeyEvent(e);
}

-(void)handleMouseButtonEvent:(nonnull NSEvent*)event
{
	Scene* scene = _gameApp->GetEditorScene();
	auto s = scene->GetSystem<InputSystem>();
	
	KeyEvent e;
	switch (event.type)
	{
		case NSEventTypeLeftMouseDown:
		{
			e.key = KeyCode::MouseLeftButton;
			e.action = KeyAction::Pressed;
			break;
		}
		case NSEventTypeLeftMouseUp:
		{
			e.key = KeyCode::MouseLeftButton;
			e.action = KeyAction::Released;
			break;
		}
		case NSEventTypeRightMouseDown:
		{
			e.key = KeyCode::MouseRightButton;
			e.action = KeyAction::Pressed;
		}
		case NSEventTypeRightMouseUp:
		{
			e.key = KeyCode::MouseRightButton;
			e.action = KeyAction::Released;
		}
		case NSEventTypeOtherMouseDown:
		{
			e.key = KeyCode::MouseMiddleButton;
			e.action = KeyAction::Pressed;
		}
		case NSEventTypeOtherMouseUp:
		{
			e.key = KeyCode::MouseMiddleButton;
			e.action = KeyAction::Released;
		}
		default:
		abort(); break;
	}
	s->PostKeyEvent(e);

}

- (void)handleMouseScrollEvent:(nonnull NSEvent*)event
{
	Scene* scene = _gameApp->GetEditorScene();
	auto s = scene->GetSystem<InputSystem>();
	NSAssert(event.type==NSEventTypeScrollWheel, @"");
	double wheel_dx = 0.0;
	double wheel_dy = 0.0;
	
	wheel_dx = [event scrollingDeltaX];
	wheel_dy = [event scrollingDeltaY];
	if ([event hasPreciseScrollingDeltas])
	{
		wheel_dx *= 0.1;
		wheel_dy *= 0.1;
	}
	s->UpdateAxis(FishEngine::Axis::MouseScrollWheel, wheel_dy);
}

@end
