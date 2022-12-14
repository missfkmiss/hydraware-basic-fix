#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include "valve_sdk/Misc/Color.hpp"

#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}
#define CONFIGURATION(type, var, val) type var = val
template <typename T>
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_)
	{
		category = category_;
		name = name_;
		value = value_;
	}

	std::string category, name;
	T* value;
};

struct legitbot_s
{
	bool enabled = false;
	bool deathmatch = false;
	//bool silent = false;
	int silent2 = false;

	bool flash_check = false;
	bool smoke_check = false;
	bool autopistol = false;

	float fov = 0.f;
	float silent_fov = 0.f;
	float smooth = 1.f;

	int shot_delay = 0;
	int kill_delay = 0;

	struct
	{
		bool head = true;
		bool chest = true;
		bool hands = true;
		bool legs = true;
	} hitboxes;

	struct
	{
		bool enabled = false;
		int start = 1;
		int type = 0;
		int x = 100;
		int y = 100;
	} rcs;

	struct
	{
		bool enabled = false;
		int min_damage = 1;
	} autowall;

	struct
	{
		bool enabled = false;
		int hotkey = 0;
	} autofire;
};

struct weapons
{
	legitbot_s legit;
};
struct statrack_setting
{
	int definition_index = 1;
	struct
	{
		int counter = 0;
	}statrack_new;
};
struct item_setting
{
	char name[32] = "Default";
	//bool enabled = false;
	int stickers_place = 0;
	int definition_vector_index = 0;
	int definition_index = 0;
	bool   enabled_stickers = 0;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool stat_trak = 0;
	float wear = FLT_MIN;
	char custom_name[32] = "";
};
class Options
{
public:
	std::map<short, weapons> weapons;
	struct
	{
		/*struct
		{
			std::map<int, profilechanger_settings> profile_items = { };
		}profile;*/
		struct
		{
			bool skin_preview = false;
			bool show_cur = true;

			std::map<int, statrack_setting> statrack_items = { };
			std::map<int, item_setting> m_items = { };
			std::map<std::string, std::string> m_icon_overrides = { };
		}skin;
	}changers;
		// 
		// ESP
		// 
	//	bool esp_enabled = false;
		bool esp_enemies_only = false;
		bool esp_player_boxes = false;
		bool esp_player_boxesOccluded = false;
		bool esp_player_names = false;
		bool esp_player_health = false;
		bool esp_player_armour = false;
		bool esp_player_weapons = false;
		bool esp_player_snaplines = false;
		bool esp_dropped_weapons = false;
		bool esp_defuse_kit = false;
		bool esp_planted_c4 = false;
		bool esp_items = false;

		// 
		// GLOW
		// 
		bool glow_enabled = false;
		bool glow_enemies_only = false;
		bool glow_players = false;
		bool glow_chickens = false;
		bool glow_c4_carrier = false;
		bool glow_planted_c4 = false;
		bool glow_defuse_kits = false;
		bool glow_weapons = false;
		bool glow_enemiesOC = false;

		//
		// CHAMS
		//
		bool chams_player_enabled = false;
		bool player_enemies_shine = false;

		bool chams_player_enemies_only = false;
		bool chams_player_wireframe = false;
		int chams_player_flat = false;
		bool chams_player_ignorez = false;
		bool chams_player_glass = false;
		bool chams_arms_enabled = false;
		bool chams_arms_wireframe = false;
		bool chams_arms_flat = false;
		bool chams_arms_ignorez = false;
		bool chams_arms_glass = false;

		//
		// MISC
		//
		bool misc_bhop = false;
		bool misc_bhop2 = false;
		int playerModelT{ 0 };
		int playerModelCT{ 0 };

		bool misc_showranks = false;
		bool misc_watermark = false;
		bool Velocity = false;
		bool outline = false;
		bool lastjump = false;
		bool lastjumpoutline = false;
		bool autoaccept = false;
		bool no_flash = false;
		bool no_smoke = false;
		bool spectator_list = false;
		int AutoStafe_key;
		bool autostrafe = false;

		struct
		{
			bool enabled = false;
			bool edge_jump_duck_in_air = false;

			int hotkey = 0;
		} edgejump;
		bool edge_bug;
		int edge_bug_key;
		bool jump_bug = false;
		int jump_bug_key;
		int glow_enemies_type;
		bool sniper_xhair = false;
		CONFIGURATION(bool, misc_thirdperson, false);
		CONFIGURATION(float, misc_thirdperson_dist, 130.f);

		// 
		// COLORS
		// 
		Color color_esp_ally_visible= {0, 0, 0};
		Color color_esp_enemy_visible= {0, 0, 0};
		Color color_esp_ally_occluded= {0, 0, 0};
		Color color_esp_enemy_occluded= {0, 0, 0};
		Color color_esp_weapons= {0, 0, 0};
		Color color_esp_defuse= {0, 0, 0};
		Color color_esp_c4= {255, 255, 255};
		Color color_esp_item= {0, 0, 0};
		Color Velocitycol = { 0, 0, 0 };

		Color color_glow_ally= {0, 0, 0};
		Color color_glow_allyOC= {0, 0, 0};

		Color color_glow_enemy= {0, 0, 0};
		Color color_glow_enemyOC= {0, 0, 0};

		Color color_glow_chickens= {0, 0, 0};
		Color color_glow_c4_carrier= {0, 0, 0};
		Color color_glow_planted_c4= {0, 0, 0};
		Color color_glow_defuse= {0, 0, 0};
		Color color_glow_weapons= {0, 0, 0};

		Color color_chams_player_ally_visible= {0, 0, 0};
		Color color_chams_player_ally_occluded= {0, 0, 0};
		Color color_chams_player_enemy_visible= {0, 0, 0};
		Color color_chams_player_enemy_occluded= {0, 0, 0};
		Color color_chams_arms_visible= {0, 0, 0};
		Color color_chams_arms_occluded= {0, 0, 0};
		Color color_watermark= {0, 0, 0}; // no menu config cuz its useless
		Color player_enemy_visible_shine = { 0, 0, 0 };


protected:
	//std::vector<ConfigValue<char>*> chars;
	std::vector<ConfigValue<int>*> ints;
	std::vector<ConfigValue<bool>*> bools;
	std::vector<ConfigValue<float>*> floats;
private:
//	void SetupValue(char value, std::string category, std::string name);
	void SetupValue(int& value, std::string category, std::string name);
	void SetupValue(bool& value, std::string category, std::string name);
	void SetupValue(float& value, std::string category, std::string name);
	void SetupColor(Color& value, const std::string& name);
	void SetupWeapons();
	void SetupVisuals();
	void SetupMisc();
	void SetupColors();
public:
	void Initialize();
	void LoadSettings(const std::string& szIniFile);
	void SaveSettings(const std::string& szIniFile);
	void DeleteSettings(const std::string& szIniFile);

	std::string folder;
};

inline Options g_Options;
inline bool   g_Unload;
