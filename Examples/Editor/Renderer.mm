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
		GameApp::Update();
	}
};


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
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
	_gameApp->Resize(size.width/2, size.height/2);
	FishEngine::ResetGraphicsAPI(size.width, size.height);
}

- (void)handleEvent:(nonnull NSEvent*)event{
	Scene* scene = _gameApp->GetEditorScene();
	auto input = scene->input;
	auto s = scene->GetSystem<InputSystem>();
	
	
	if (event.type == NSEventTypeKeyDown)
	{
		
	}
	else if (event.type == NSEventTypeKeyUp)
	{
		
	}
}

-(void)handleMouseButtonEvent:(nonnull NSEvent*)event
{
	Scene* scene = _gameApp->GetEditorScene();
	auto input = scene->input;
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
