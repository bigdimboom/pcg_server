
//#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <httplib.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include "dimcli/cli.h"
#include "ply/ply_reader.h"

namespace fs = std::filesystem;

bool readPLY(const std::string& path, std::vector<float3>& myverts, std::vector<Color>& myColors)
{
	std::ifstream infile(path);

	if (infile.is_open() && !infile.fail())
	{
		read_ply_file(path, myverts, myColors);
		return true;
	}

	return false;
}

int main(int argc, char** argv)
{
	Dim::Cli cli;

	auto& host_name = cli.opt<std::string>("host <host_name>").desc("like. 192.168.x.x");
	auto& host_port = cli.opt<int>("port <host_port>").desc("range (1:65535)");
	auto& pcg_path = cli.opt<std::string>("pcg <pcg_file>").desc("xxx.ply");

	if (!cli.parse(argc, argv))
	{
		auto ret = cli.printError(std::cerr);
		if (!*cli.helpOpt())
		{
			std::cerr << "--help for more info.\n";
		}
		return ret;
	}

	if (!fs::exists(*pcg_path))
	{
		spdlog::error("unable to find {}", *pcg_path);
		return -1;
	}

	{
		fs::path pcgPath(*pcg_path);
		if (!pcgPath.has_extension() || pcgPath.extension() != ".ply")
		{
			spdlog::error("point cloud file must be ply.");
			return -1;
		}
	}


	httplib::Server svr;

	//if (!svr.set_mount_point("/unity", unity_dir->c_str()))
	//{
	//	spdlog::error("unable to bind Unity WebGl build folder: {}", *unity_dir);
	//	return EXIT_FAILURE;
	//}

	//if (!svr.set_mount_point("/workdir", work_dir->c_str()))
	//{
	//	spdlog::error("unable to bind Unity WebGl build folder: {}", *work_dir);
	//	return EXIT_FAILURE;
	//}

	svr.Options(R"(\*)", [](const auto& req, auto& res)
	{
		res.set_header("Allow", "GET, POST, HEAD, OPTIONS");
	});

	svr.Options("/", [](const auto& req, auto& res)
	{
		res.set_header("Access-Control-Allow-Origin", req.get_header_value("Origin").c_str());
		res.set_header("Allow", "GET, POST, HEAD, OPTIONS");
		res.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Origin, Authorization");
		res.set_header("Access-Control-Allow-Methods", "OPTIONS, GET, POST, HEAD");
	});


	svr.Get("/test", [&pcg_path](const httplib::Request& req, httplib::Response& res)
	{
		res.set_content("hello world", "text/html");
	});

	svr.Get("/download_pcg", [&pcg_path](const httplib::Request& req, httplib::Response& res)
	{
		//std::string ply_text;
		std::string pcg_path(*pcg_path);

		std::vector<float3> myverts;
		std::vector<Color> myColors;
		std::string buffer;

		if (fs::exists(pcg_path) && readPLY(pcg_path, myverts, myColors))
		{
			auto bytes1 = myverts.size() * sizeof(float3);
			auto bytes2 = myColors.size() * sizeof(Color);
			buffer.resize(4 + bytes1 + bytes2);

			const int32_t size = myverts.size();
			spdlog::info("size: {}", size);

			memcpy((char*)buffer.c_str(), &size, sizeof(int32_t));
			memcpy(((char*)buffer.c_str()) + 4, myverts.data(), bytes1);
			memcpy(((char*)buffer.c_str()) + 4 + bytes1, myColors.data(), bytes2);
			res.set_content(buffer, "text/plain");
			return;
		}
		res.set_content("pcg does not exist", "text/html");
	});

	spdlog::info("server is running @{}:{}", *host_name, *host_port);
	svr.listen((*host_name).c_str(), (*host_port));

	return 0;
}