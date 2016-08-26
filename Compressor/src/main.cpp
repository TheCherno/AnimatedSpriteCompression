#include <iostream>
#include <string>
#include <stack>

#include <Flinty.h>
#include <Windows.h>

#include "Compressor.h"

enum class Mode
{
	DECOMPRESS, COMPRESS
};

static std::vector<String> GetAllFiles(String path, const String& mask)
{
	std::vector<String> results;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA ffd;
	String spec;
	std::stack<String> directories;

	directories.push(path);

	while (!directories.empty()) {
		path = directories.top();
		spec = path + "/" + mask;
		directories.pop();

		hFind = FindFirstFile(spec.c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE) {
			return results;
		}

		do {
			if (strcmp(ffd.cFileName, ".") != 0 &&
				strcmp(ffd.cFileName, "..") != 0) {
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					directories.push(path + "/" + ffd.cFileName);
				}
				else {
					results.push_back(path + "/" + ffd.cFileName);
				}
			}
		} while (FindNextFile(hFind, &ffd) != 0);

		if (GetLastError() != ERROR_NO_MORE_FILES) {
			FindClose(hFind);
			return results;
		}

		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
	}

	return results;
}

static void PrintUsage()
{
	std::cout << "\tUsage: sc mode(compress|decompress) input-path output-path [quality]" << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		PrintUsage();
		return 1;
	}

	String output = "";
	if (argc > 3)
		output = String(argv[3]);

	byte quality = 0;
	if (argc > 4)
		quality = (byte)atoi(argv[4]);

	int windowSize = 16;
	if (argc > 5)
		windowSize = atoi(argv[5]);


	Mode mode = argv[1] == "Decompress" ? Mode::DECOMPRESS : Mode::COMPRESS;
	std::vector<String> files = GetAllFiles(argv[2], "*");

	std::cout << "Reading " << files.size() << " images... ";
	std::vector<Sprite> sprites;
	for (int i = 0; i < files.size(); i++)
		sprites.push_back(Sprite(files[i]));
	std::cout << "Done." << std::endl;

	Compressor compressor(sprites, quality, windowSize);
	compressor.Compress(output);

	system("PAUSE");
	return 0;
}