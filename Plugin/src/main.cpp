#include "main.h"
ptrdiff_t offset1, offset2, offset3;

namespace DisableDialogueCamera
{
	std::int32_t camera;
	bool firstPerson = false;
	bool thirdPerson = false;
	std::int32_t dialogueMenu = 0;
	bool shipMenu = false;
	
	REL::Relocation<std::uintptr_t> ShowMenu{ REL::ID(891126) };
	
	REL::Relocation<std::uintptr_t> Addr1{ REL::ID(817115) };
	REL::Relocation<std::uintptr_t> Addr2{ REL::ID(166169), 0x313 };
	
	REL::Relocation<std::uintptr_t> HookAddr1{ REL::ID(166169), 0x1AD};
	REL::Relocation<std::uintptr_t> HookAddr2{ REL::ID(165937), 0x3F};
	
	REL::Relocation<std::uintptr_t> CallAddr1{ REL::ID(179039) };
	REL::Relocation<std::uintptr_t> CallAddr2{ REL::ID(81121) };
	
	REL::Relocation<std::uintptr_t> AlwaysShow{ REL::ID(145943), 0x2E7};
	
	
	RE::CameraState CameraState()
	{
		return *SFSE::stl::adjust_pointer<RE::CameraState>(RE::PlayerCamera::GetSingleton()->currentState, 0x50);
	}

	bool FirstPerson()
	{
		return CameraState() == RE::CameraState::kFirstPerson;
	}

	void Install()
	{		
		RE::UI::GetSingleton()->RegisterSink(EventHandler::GetSingleton());
		
	   	const auto settings = Settings::Main::GetSingleton();
		
		camera = *settings->cameraType;
		if (newver) {
			offset1 = -0xCA;
			offset2 = 0xC5;
			offset3 = 0xD6;
		}
		else {
			offset1 = -0xCB;	
			offset2 = 0x5F;
			offset3 = 0x70;	
		}
		REL::Relocation<std::uintptr_t> NoBlock3{ REL::ID(120109), 0x8B};
		REL::Relocation<std::uintptr_t> DialogueCamera1{ REL::ID(100146), offset1};
		REL::Relocation<std::uintptr_t> DialogueCamera2{ REL::ID(153784), 0x2A};	
		if (newver) {
			REL::Relocation<std::uintptr_t> NoBlock5{ REL::ID(137819), 0x4C};
			
			constexpr std::uint8_t jmp[] = { 0xE9, 0x99, 0x07, 0x00, 0x00, 0x90, 0x90 };
			
			REL::safe_write(NoBlock5.address(), &REL::NOP7, sizeof(REL::NOP7));	
			REL::safe_write(DialogueCamera2.address(), &jmp, sizeof(jmp));	
		}
		else {		
			constexpr std::uint8_t jmp[] = { 0xE9, 0x8D, 0x07, 0x00, 0x00, 0x90, 0x90 };			
			
			REL::safe_write(DialogueCamera2.address(), &jmp, sizeof(jmp));	
		}
		
		REL::safe_write(DialogueCamera1.address(), &REL::NOP6, sizeof(REL::NOP6));	
		REL::safe_write(AlwaysShow.address(), &REL::JMP8, sizeof(REL::JMP8));		
		REL::safe_write(NoBlock3.address(), &REL::NOP5, sizeof(REL::NOP5));
		
		if (*settings->enableMovement) {
			static REL::Relocation<std::uintptr_t> NoBlock1{ REL::ID(137819), offset2};
			REL::Relocation<std::uintptr_t> NoBlock2{ REL::ID(137819), offset3};
			REL::Relocation<std::uintptr_t> NoBlock4{ REL::ID(171169), 0xD5};
				
			REL::safe_write(NoBlock2.address(), &REL::NOP5, sizeof(REL::NOP5));			
			REL::safe_write(NoBlock4.address(), &REL::NOP5, sizeof(REL::NOP5));
			{
				struct Gamepad_Code : Xbyak::CodeGenerator
				{
					Gamepad_Code()
					{
						Xbyak::Label retnLabel;
						Xbyak::Label callLabel;
						Xbyak::Label addrLabel;
					
						mov(rax, ptr[rip + addrLabel]);
						cmp(byte[rax], 1);
						je("KeyBoard");
						call(ptr[rip + callLabel]);
					
						L("KeyBoard");
						jmp(ptr[rip + retnLabel]);					
					
						L(addrLabel);
						dq(Addr1.address());
					
						L(callLabel);
						dq(CallAddr1.address());					

						L(retnLabel);
						dq(NoBlock1.address() + 0x5);
					}
				};

				Gamepad_Code code;
				code.ready();

				auto& trampoline = SFSE::GetTrampoline();
				trampoline.write_branch<5>(NoBlock1.address(), trampoline.allocate(code));
			}
		}
		
		if (*settings->disablePOVChange) {
			{
				struct disableZoomIn_Code : Xbyak::CodeGenerator
				{
					disableZoomIn_Code()
					{
						Xbyak::Label retnLabel;
						Xbyak::Label menuLabel;
						Xbyak::Label jmpLabel;
						
						je("Skip");
						
						mov(rcx, (uintptr_t)&dialogueMenu);
						cmp(dword[rcx], 00);
						je("Original");
						
						mov(rcx, ptr[rip + menuLabel]);
					    cmp(dword[rcx + 0x84], 00);
						je("Original");
						
						jmp("Skip");
						
						L("Original");
						jmp(ptr[rip + retnLabel]);
						
						L("Skip");
						jmp(ptr[rip + jmpLabel]);
						
						L(jmpLabel);
						dq(Addr2.address());
						
						L(menuLabel);
						dq(ShowMenu.address());

						L(retnLabel);
						dq(HookAddr1.address() + 0x6);
					}
				};

				disableZoomIn_Code code;
				code.ready();

				auto& trampoline = SFSE::GetTrampoline();
				trampoline.write_branch<6>(HookAddr1.address(), trampoline.allocate(code));
			}
			{
				struct disableZoomOut_Code : Xbyak::CodeGenerator
				{
					disableZoomOut_Code()
					{
						Xbyak::Label retnLabel;
						Xbyak::Label callLabel;
						Xbyak::Label menuLabel;
						
						mov(rcx, (uintptr_t)&dialogueMenu);
						cmp(dword[rcx], 00);
						je("Original");
						
						mov(rcx, ptr[rip + menuLabel]);
					    cmp(dword[rcx + 0x84], 00);
						je("Original");
						
						jmp("Skip");
						
						L("Original");
						call(ptr[rip + callLabel]);
						
						L("Skip");
						jmp(ptr[rip + retnLabel]);
						
						L(menuLabel);
						dq(ShowMenu.address());

						L(callLabel);
						dq(CallAddr2.address());

						L(retnLabel);
						dq(HookAddr2.address() + 0x5);
					}
				};

				disableZoomOut_Code code;
				code.ready();

				auto& trampoline = SFSE::GetTrampoline();
				trampoline.write_branch<5>(HookAddr2.address(), trampoline.allocate(code));
			}
		}

		INFO("Installed");
	}

	RE::BSEventNotifyControl EventHandler::ProcessEvent(const RE::MenuOpenCloseEvent& a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
	{
		{
			if (a_event.menuName == "SpaceshipHudMenu" && a_event.opening) {
				shipMenu = true;
			}
			if (a_event.menuName == "SpaceshipHudMenu" && !a_event.opening) {
				shipMenu = false;
			}
			if (a_event.menuName == "DialogueMenu" && !shipMenu) 
			{
				if (a_event.opening)
				{
					dialogueMenu = 1;
					if (FirstPerson()) 
					{
						if (camera == 2)
						{
							RE::PlayerCamera::GetSingleton()->ForceThirdPerson();
							firstPerson = true;
						}
					}
					else
					{
						if (camera == 1)
						{
							RE::PlayerCamera::GetSingleton()->ForceFirstPerson();
							thirdPerson = true;
						}
					}
				}
				else 
				{
					dialogueMenu = 0;			
					if (FirstPerson())
					{
						if (camera == 1 && thirdPerson)
						{
							RE::PlayerCamera::GetSingleton()->ForceThirdPerson();
							thirdPerson = false;
						}
					}
					else
					{
						if (camera == 2 && firstPerson)
						{
							RE::PlayerCamera::GetSingleton()->ForceFirstPerson();
							firstPerson = false;
						}
					}
				}			
			}

			return RE::BSEventNotifyControl::kContinue;
		}
	}
}

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type) {
		case SFSE::MessagingInterface::kPostLoad:
			{
				Settings::Main::GetSingleton()->Load();
				DisableDialogueCamera::Install();
			}
			break;
		default:
			break;
		}
	}
}

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
#ifndef NDEBUG
	MessageBoxA(NULL, "Loaded. You can now attach the debugger or continue execution.", Plugin::NAME.data(), NULL);
#endif

	SFSE::Init(a_sfse, false);
	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));
	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);
	const REL::Version ver = a_sfse->RuntimeVersion();
	
	if (ver >= SFSE::RUNTIME_SF_1_11_36) {
		newver = true;
	}

	// do stuff
	// this allocates 1024 bytes for development builds, you can
	// adjust the value accordingly with the log result for release builds
	SFSE::AllocTrampoline(1 << 10);

	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
