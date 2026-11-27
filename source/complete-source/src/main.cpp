#include "version.h"
#include "init.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace
{
	constexpr SKSE::PluginVersionData GetPluginVersion() {
		SKSE::PluginVersionData version;

		version.PluginVersion({ Version::MAJOR, Version::MINOR, Version::PATCH, Version::BUILD });
		version.PluginName(Project::NAME);
		// CommonLibSSE-NG 6.7.0 dropped the bool parameter. PluginVersionData also defaults
		// versionIndependenceEx to kVersionIndependentEx_AddressLibraryV5, which is what
		// Skyrim 1.7.99 requires before it will load the plugin at all.
		version.UsesAddressLibrary();
		// The shipped 2.0 DLL declared versionIndependence = 5 (AddressLibraryPostAE plus
		// StructsPost629). Keep that identical so SKSE sees the same plugin.
		version.UsesUpdatedStructs();

		return version;
	}

	void InitLog() {
		auto path = SKSE::log::log_directory();

		if (path) {
			*path /= Project::NAME;
			*path += ".log";
			spdlog::set_default_logger(spdlog::basic_logger_mt("default", path->string(), true));
		}

		spdlog::set_level(spdlog::level::info);
		spdlog::flush_on(spdlog::level::info);
	}
}

extern "C" __declspec(dllexport) constinit SKSE::PluginVersionData SKSEPlugin_Version{ GetPluginVersion() };

extern "C" __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
	InitLog();
	SKSE::Init(a_skse);
	Gotobed::Init();

	return true;
}