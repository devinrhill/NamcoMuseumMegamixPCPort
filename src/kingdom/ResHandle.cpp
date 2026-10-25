#include "kingdom/ResHandle.hpp"

#include <cstdio>
#include <set>
#include "wii/Archive.hpp"

namespace kingdom
{
ResHandle* MakeResHandle(const std::string& key)
{
	std::size_t size = 0;
	std::string entry = key;
	if (wii::ArchiveStore::Find(entry, size) == nullptr)
	{
		entry = key + ".brres";
		if (wii::ArchiveStore::Find(entry, size) == nullptr) { static std::set<std::string> warned; if (warned.insert(key).second) std::fprintf(stderr, "ResHandle: '%s' is not in the attached archives\n", key.c_str()); return nullptr; }
	}
	ResHandle* h = new ResHandle();
	h->entry = entry;
	h->name = key;
	return h;
}
}
