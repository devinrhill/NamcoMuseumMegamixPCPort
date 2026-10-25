#include "wii/Nand.hpp"

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <system_error>

#include "WiiSystem.hpp"

namespace fs = std::filesystem;

namespace wii::nand
{
namespace
{
fs::path Path(const std::string& name)
{
	return fs::path(WiiSystem::SaveDir()) / name;
}

int FromError(const std::error_code& ec)
{
	if (!ec) return OK;
	if (ec == std::errc::no_such_file_or_directory) return NOEXISTS;
	if (ec == std::errc::no_space_on_device) return MAXBLOCKS;
	if (ec == std::errc::file_exists) return EXISTS;
	if (ec == std::errc::permission_denied || ec == std::errc::read_only_file_system) return ACCESS;
	return UNKNOWN;
}
}

int ChangeDir()
{
	std::error_code ec;
	fs::create_directories(WiiSystem::SaveDir(), ec);
	return FromError(ec);
}

int GetLength(const std::string& name, std::uint32_t& length)
{
	std::error_code ec;
	auto size = fs::file_size(Path(name), ec);
	if (ec) return FromError(ec);
	length = static_cast<std::uint32_t>(size);
	return OK;
}

int ReadFile(const std::string& name, std::uint8_t* buf, std::uint32_t size, std::uint32_t& read)
{
	read = 0;
	std::ifstream f(Path(name), std::ios::binary);
	if (!f) return fs::exists(Path(name)) ? ACCESS : NOEXISTS;
	f.read(reinterpret_cast<char*>(buf), size);
	read = static_cast<std::uint32_t>(f.gcount());
	return OK;
}

int WriteFile(const std::string& name, const std::uint8_t* buf, std::uint32_t size)
{
	std::error_code ec;
	const fs::path tmp = Path(name + ".tmp");
	{
		std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
		if (!f) return ACCESS;
		f.write(reinterpret_cast<const char*>(buf), size);
		if (!f) return MAXBLOCKS;
	}
	fs::rename(tmp, Path(name), ec); // replaces an existing file
	return FromError(ec);
}

int Delete(const std::string& name)
{
	std::error_code ec;
	if (!fs::remove(Path(name), ec) && !ec) return NOEXISTS;
	return FromError(ec);
}

int Check(std::uint32_t blocks, std::uint32_t inodes, std::uint32_t& answer)
{
	answer = 0;
	std::error_code ec;
	auto info = fs::space(WiiSystem::SaveDir(), ec);
	if (ec) return OK; // cannot tell: assume there is room
	if (info.available < static_cast<std::uintmax_t>(blocks) * 16384u) answer |= 1;
	(void)inodes; // a host file system has no inode limit that matters here
	return OK;
}
}
