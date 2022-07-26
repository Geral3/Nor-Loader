#pragma once
#include <windows.h>
#include <string_view>
#include <vector>

namespace ph {
	/// <summary>
	/// Contains information about a module a user has access to.
	/// </summary>
	struct module_t {
		int id;
		int method;
		const char* name;
		const char* version;
		const char* status;
		const char* target;
	};

	/// <summary>
	/// Contains information regarding the user's authenticated state.
	/// </summary>
	struct login_request_t {
		const char* message;
		module_t* modules;
		int module_count;
		int success;
	};

	/// <summary>
	/// Contains information regarding the status of the attempted injection.
	/// </summary>
	struct inject_request_t {
		const char* message;
		int success;
	};

	/// <summary>
	/// Contains information regarding the server.
	/// </summary>
	struct server_data_t {
		int id;
		const char* name;
		const char* host;
		const char* port;
	};

	/// <summary>
	/// Sends user information to server and authenticates the client.
	/// </summary>
	/// <param name="login_request">Reference to a login request to output to.</param>
	/// <param name="modules">Reference to a module list to output to.</param>
	/// <param name="mta">Use multithreading instead of apartment state; set to true for most c++ use cases.</param>
	/// <returns>Returns true if authentication was successful, or false otherwise. Output message is stored in login_request.message field.</returns>
	inline bool login(login_request_t& login_request, std::vector<module_t>& modules, bool mta = true) {
		static auto Login = reinterpret_cast<void(__stdcall*)(login_request_t&, bool)>(GetProcAddress(LoadLibraryA("ph.dll"), "Login"));

		if (!modules.empty())
			modules.clear();

		Login(login_request, mta);

		if (login_request.modules && login_request.module_count > 0) {
			for (auto i = 0; i < login_request.module_count; i++)
				modules.push_back(login_request.modules[i]);
		}

		return login_request.success == 1;
	}

	/// <summary>
	/// Retrieves username of currently authenticated user.
	/// </summary>
	/// <returns>String containing retrieved username.</returns>
	inline std::string_view get_username() {
		static auto GetUsername = reinterpret_cast<const char* (__stdcall*)()>(GetProcAddress(LoadLibraryA("ph.dll"), "GetUsername"));
		return GetUsername();
	}

	/// <summary>
	/// Requests loader to inject into the target process with the selected module.
	/// </summary>
	/// <param name="inject_request">Reference to an injection request to output to.</param>
	/// <param name="id">ID of wanted module to inject. This value can be retrieved from the list of subscribed modules; modules[i].id.</param>
	/// <returns>Returns true if injection was successful, or false otherwise. Output message is stored in inject_request.message field.</returns>
	inline bool inject(inject_request_t& inject_request, int id) {
		static auto Inject = reinterpret_cast<void(__stdcall*)(inject_request_t&, int)>(GetProcAddress(LoadLibraryA("ph.dll"), "Inject"));
		Inject(inject_request, id);
		return inject_request.success == 1;
	}

	/// <summary>
	/// Requests loader to send progress of download.
	/// </summary>
	/// <returns>Returns a floating point representing the download progress. This value ranges from 0 to 1</returns>
	inline float get_download_progress() {
		static auto GetDownloadProgress = reinterpret_cast<float(__stdcall*)()>(GetProcAddress(LoadLibraryA("ph.dll"), "GetDownloadProgress"));
		return GetDownloadProgress();
	}

	/// <summary>
	/// Requests loader to send an array of servers.
	/// </summary>
	/// <param name="servers">Reference to a server list to output to.</param>
	/// <returns>Returns number of servers.</returns>
	inline int get_servers(std::vector<server_data_t>& servers) {
		static auto GetServers = reinterpret_cast<server_data_t * (__stdcall*)(int&)>(GetProcAddress(LoadLibraryA("ph.dll"), "GetServers"));

		if (!servers.empty())
			servers.clear();

		int _serversCount = 0;
		const auto _servers = GetServers(_serversCount);

		if (_servers && _serversCount > 0) {
			for (auto i = 0; i < _serversCount; i++)
				servers.push_back(_servers[i]);
		}

		return _serversCount;
	}

	/// <summary>
	/// Requests loader to set the server location.
	/// </summary>
	/// <param name="id">ID of wanted server location to use. This value can be retrieved from the list of servers; servers[i].id.</param>
	/// <returns>Returns true if setting the new server location was successful, or false otherwise.</returns>
	inline bool set_server(int id) {
		static auto SetServer = reinterpret_cast<bool(__stdcall*)(int)>(GetProcAddress(LoadLibraryA("ph.dll"), "SetServer"));
		return SetServer(id);
	}
}