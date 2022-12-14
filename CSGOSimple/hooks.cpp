#include "hooks.hpp"
#include <intrin.h>  

#include "render.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/bhop.hpp"
#include "features/chams.hpp"
#include "features/visuals.hpp"
#include "features/glow.hpp"
#include "features/skins.h"

#pragma intrinsic(_ReturnAddress)  

namespace Hooks {

	void anti_cheat_fix()
	{
		const char* modules[]{ "client.dll", "engine.dll", "server.dll", "studiorender.dll", "materialsystem.dll", "shaderapidx9.dll", "vstdlib.dll", "vguimatsurface.dll" };
		long long long_long = 0x69690004C201B0;
		for (auto test : modules)
			WriteProcessMemory(GetCurrentProcess(), (LPVOID)Utils::PatternScan(GetModuleHandleA(test), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &long_long, 7, 0);
	}

	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		stdrender_hook.setup(g_StudioRender);
		viewrender_hook.setup(g_ViewRender);

		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);

		gameevents_hook.setup(g_GameEvents);
		gameevents_hook.hook_index(index::FireEvent, hkFireEvent);
		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);
		stdrender_hook.hook_index(index::DrawModelExecute2, hkDrawModelExecute2);
		viewrender_hook.hook_index(index::RenderSmokeOverlay, RenderSmokeOverlay);

		sequence_hook = new recv_prop_hook(C_BaseViewModel::m_nSequence(), RecvProxy);
		anti_cheat_fix();
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();

		Glow::Get().Shutdown();
		sequence_hook->~recv_prop_hook();

	}
	bool __fastcall send_net_msg(void* ecx, void* edx, INetMessage* msg, bool reliable, bool voice)
	{
		static auto oFireEvent = hk_netchannel.get_original<sendnetmsg_fn>(40);

		if (!msg)
			return original_sendnetmsg(ecx, msg, reliable, voice);


		if (msg->GetType() == 14)
			return false;

		return oFireEvent(ecx, msg, reliable, voice);
	}

	bool __stdcall hkFireEvent(IGameEvent* pEvent) {
		static auto oFireEvent = gameevents_hook.get_original<FireEvent>(index::FireEvent);

		if (!pEvent)
			return oFireEvent(g_GameEvents, pEvent);

		const char* szEventName = pEvent->GetName();


		if (!strcmp(szEventName, "server_spawn"))
		{

			const auto net_channel = g_EngineClient->GetNetChannelInfo();

			if (net_channel != nullptr)
			{
				const auto index = 40;
				Hooks::hk_netchannel.setup(net_channel);
				Hooks::hk_netchannel.hook_index(index, send_net_msg);
			}
		}

		if (!strcmp(szEventName, "cs_game_disconnected"))
		{
			if (hk_netchannel.is_hooked())
			{
				hk_netchannel.unhook_all();
			}
		}

		return oFireEvent(g_GameEvents, pEvent);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);

		DWORD colorwrite, srgbwrite;
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vert_dec);
		pDevice->GetVertexShader(&vert_shader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		auto esp_drawlist = Render::Get().RenderScene();

		Menu::Get().Render();
	

		ImGui::Render(esp_drawlist);

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vert_dec);
		pDevice->SetVertexShader(vert_shader);

		return oEndScene(pDevice);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);

		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
			Menu::Get().OnDeviceReset();

		return hr;
	}
	//--------------------------------------------------------------------------------
	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);

		oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!cmd || !cmd->command_number)
			return;
		
		if (Menu::Get().IsVisible())
			cmd->buttons &= ~IN_ATTACK;

		if (g_Options.misc_bhop)
			BunnyHop::OnCreateMove(cmd);
		if (GetAsyncKeyState(g_Options.AutoStafe_key))
			BunnyHop::AutoStafe(cmd);

		static auto prediction = new PredictionSystem();
		auto flags = g_LocalPlayer->m_fFlags();

		prediction->StartPrediction(cmd);

		g_Legitbot->Run(cmd);
		float max_radias = D3DX_PI * 2;
		float step = max_radias / 128;
		float xThick = 23;
		if (g_Options.jump_bug && GetAsyncKeyState(g_Options.jump_bug_key)) {
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
				g_Options.misc_bhop2 = false;
				bool unduck = cmd->buttons &= ~in_duck;
				if (unduck) {
					cmd->buttons &= ~in_duck; // duck
					cmd->buttons |= in_jump; // jump
					unduck = false;
				}
				Vector pos = g_LocalPlayer->abs_origin();
				for (float a = 0.f; a < max_radias; a += step) {
					Vector pt;
					pt.x = (xThick * cos(a)) + pos.x;
					pt.y = (xThick * sin(a)) + pos.y;
					pt.z = pos.z;


					Vector pt2 = pt;
					pt2.z -= 8192;

					trace_t fag;

					Ray_t ray;
					ray.Init(pt, pt2);

					CTraceFilter flt;
					flt.pSkip = g_LocalPlayer;
					g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &fag);

					if (fag.fraction != 1.f && fag.fraction != 0.f) {
						cmd->buttons |= in_duck; // duck
						cmd->buttons &= ~in_jump; // jump
						unduck = true;
					}
				}
				for (float a = 0.f; a < max_radias; a += step) {
					Vector pt;
					pt.x = ((xThick - 2.f) * cos(a)) + pos.x;
					pt.y = ((xThick - 2.f) * sin(a)) + pos.y;
					pt.z = pos.z;

					Vector pt2 = pt;
					pt2.z -= 8192;

					trace_t fag;

					Ray_t ray;
					ray.Init(pt, pt2);

					CTraceFilter flt;
					flt.pSkip = g_LocalPlayer;
					g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &fag);

					if (fag.fraction != 1.f && fag.fraction != 0.f) {
						cmd->buttons |= in_duck; // duck
						cmd->buttons &= ~in_jump; // jump
						unduck = true;
					}
				}
				for (float a = 0.f; a < max_radias; a += step) {
					Vector pt;
					pt.x = ((xThick - 20.f) * cos(a)) + pos.x;
					pt.y = ((xThick - 20.f) * sin(a)) + pos.y;
					pt.z = pos.z;

					Vector pt2 = pt;
					pt2.z -= 8192;

					trace_t fag;

					Ray_t ray;
					ray.Init(pt, pt2);

					CTraceFilter flt;
					flt.pSkip = g_LocalPlayer;
					g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &flt, &fag);

					if (fag.fraction != 1.f && fag.fraction != 0.f) {
						cmd->buttons |= in_duck; // duck
						cmd->buttons &= ~in_jump; // jump
						unduck = true;
					}
				}
			}
		}
		else g_Options.misc_bhop2 = true;

		if ((g_LocalPlayer->m_fFlags() & FL_ONGROUND) && g_LocalPlayer->IsAlive()) if (g_Options.edge_bug && GetAsyncKeyState(g_Options.edge_bug_key)) cmd->buttons = 4;
		prediction->EndPrediction();
		if (g_Options.edgejump.enabled && GetAsyncKeyState(g_Options.edgejump.hotkey))
		{
			if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && (flags & FL_ONGROUND))
				cmd->buttons |= IN_JUMP;

			if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND) && g_Options.edgejump.edge_jump_duck_in_air && !(cmd->buttons |= IN_DUCK))
				cmd->buttons |= IN_DUCK;
		}

		// https://github.com/spirthack/CSGOSimple/issues/69
		if (g_Options.misc_showranks && cmd->buttons & IN_SCORE) // rank revealer will work even after unhooking, idk how to "hide" ranks  again
			g_CHLClient->DispatchUserMessage(CS_UM_ServerRankRevealAll, 0, 0, nullptr);

		if (g_Options.sniper_xhair && !g_LocalPlayer->m_bIsScoped())
			g_CVar->FindVar("weapon_debug_spread_show")->SetValue(3);
		else
			g_CVar->FindVar("weapon_debug_spread_show")->SetValue(0);

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
	}
	//--------------------------------------------------------------------------------
	__declspec(naked) void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx; not sure if we need this
			push esp
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel) 
		{
			//Ignore 50% cuz it called very often
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			Render::Get().BeginScene();
		}
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (g_Options.autoaccept && !strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				//This will flash the CSGO window on the taskbar
				//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	}
	//--------------------------------------------------------------------------------
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && g_Options.glow_enabled)
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);
		if (g_EngineClient->IsInGame()) {
			//if (stage != ClientFrameStage_t::FRAME_RENDER_START && stage != ClientFrameStage_t::FRAME_RENDER_END)
			//	return;
			if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
				skins::on_frame_stage_notify(false);
			else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
				skins::on_frame_stage_notify(true);

			static int originalIdx = 0;

			if (!g_LocalPlayer) {
				originalIdx = 0;
				return;
			}

			constexpr auto getModel = [](int team) constexpr noexcept -> const char* {
				constexpr std::array models{
				"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
				"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
				"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
				"models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
				"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
				"models/player/custom_player/legacy/ctm_st6_variante.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
				"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
				"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
				"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
				"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
				"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
				"models/player/custom_player/legacy/tm_leet_variantf.mdl",
				"models/player/custom_player/legacy/tm_leet_variantg.mdl",
				"models/player/custom_player/legacy/tm_leet_varianth.mdl",
				"models/player/custom_player/legacy/tm_leet_varianti.mdl",
				"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
				"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
				"models/player/custom_player/legacy/tm_phoenix_varianth.mdl"
				};

				switch (team) {
				case 2: return static_cast<std::size_t>(g_Options.playerModelT - 1) < models.size() ? models[g_Options.playerModelT - 1] : nullptr;
				case 3: return static_cast<std::size_t>(g_Options.playerModelCT - 1) < models.size() ? models[g_Options.playerModelCT - 1] : nullptr;
				default: return nullptr;
				}
			};

			if (const auto model = getModel(g_LocalPlayer->m_iTeamNum())) {
				if (stage == FRAME_RENDER_START)
					originalIdx = g_LocalPlayer->m_nModelIndex();

				const auto idx = stage == FRAME_RENDER_END && originalIdx ? originalIdx : g_MdlInfo->GetModelIndex(model);

				g_LocalPlayer->setModelIndex(idx);

				if (const auto ragdoll = g_LocalPlayer->get_entity_from_handle(g_LocalPlayer->m_hRagdoll()))
					ragdoll->setModelIndex(idx);
			}
		}
		ofunc(g_CHLClient, edx, stage);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

		if (g_EngineClient->IsInGame() && vsView)
			Visuals::Get().ThirdPerson();

		ofunc(g_ClientMode, edx, vsView);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}


		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}

	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 87");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}

	void RecvProxy(const CRecvProxyData* pData, void* entity, void* output)
	{
		static auto ofunc = sequence_hook->get_original_function();

		if (g_LocalPlayer && g_LocalPlayer->IsAlive()) {
			const auto proxy_data = const_cast<CRecvProxyData*>(pData);
			const auto view_model = static_cast<C_BaseViewModel*>(entity);

			if (view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid()) {
				const auto owner = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(view_model->m_hOwner()));
				if (owner == g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer())) {
					const auto view_model_weapon_handle = view_model->m_hWeapon();
					if (view_model_weapon_handle.IsValid()) {
						const auto view_model_weapon = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
						if (view_model_weapon) {
							if (k_weapon_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex())) {
								auto original_sequence = proxy_data->m_Value.m_Int;
								const auto override_model = k_weapon_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
								proxy_data->m_Value.m_Int = skins::GetNewAnimation(override_model, proxy_data->m_Value.m_Int);
							}
						}
					}
				}
			}

		}

		ofunc(pData, entity, output);
	}

	void __fastcall hkDrawModelExecute2(void* _this, int, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags)
	{
		static auto ofunc = stdrender_hook.get_original<decltype(&hkDrawModelExecute2)>(index::DrawModelExecute2);

		if (g_StudioRender->IsForcedMaterialOverride())
			return ofunc(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		static auto flash = g_MatSystem->FindMaterial("effects/flashbang", TEXTURE_GROUP_CLIENT_EFFECTS);
		static auto flash_white = g_MatSystem->FindMaterial("effects/flashbang_white", TEXTURE_GROUP_CLIENT_EFFECTS);
		flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Options.no_flash);
		flash_white->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, g_Options.no_flash);
		std::vector<const char*> vistasmoke_mats =

		{
				"particle/vistasmokev1/vistasmokev1_fire",
				"particle/vistasmokev1/vistasmokev1_smokegrenade",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		};

		for (auto mat_s : vistasmoke_mats)
		{
			IMaterial* mat = g_MatSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, g_Options.no_smoke);
		}

		Chams::Get().OnDrawModelExecute(pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		ofunc(g_StudioRender, 0, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		g_StudioRender->ForcedMaterialOverride(nullptr);
	}

	void __fastcall Hooks::RenderSmokeOverlay(void* _this, int, const bool unk)
	{
		static auto ofunc = viewrender_hook.get_original<decltype(&RenderSmokeOverlay)>(index::RenderSmokeOverlay);

		if (!g_Options.no_smoke)
			ofunc(g_ViewRender, 0, unk);
	}
}
