#include <iostream>
#include <string>
#include <stack>

#include <Flinty.h>
#include <Windows.h>

#include "Compressor.h"

enum class Mode
{
	NONE, DECOMPRESS, COMPRESS, CONFIG
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

	Mode mode = Mode::NONE;
	String m = argv[1];
	if (m == "decompress")
		mode = Mode::DECOMPRESS;
	else if (m == "compress")
		mode = Mode::COMPRESS;
	else if (m == "-f")
		mode = Mode::CONFIG;

	Metadata metadata;
	bool hasMetadata = false;

	FL_ASSERT(mode != Mode::NONE);
	// Defaults
	String input, output;
	byte compression = 0;
	byte quality = 0;
	int windowSize = 8;
	if (mode == Mode::CONFIG)
	{
		String file = argv[2];

		fl::XMLDocument document(file);
		fl::XMLNode* rootNode = document.FindNode("Config");
		FL_ASSERT(rootNode);
		fl::XMLNode* modeNode = rootNode->FindChild("Mode");
		bool compress = false;
		if (modeNode)
			compress = String(modeNode->value) == "Compress";

		if (compress)
		{
			fl::XMLNode* settingsNode = rootNode->FindChild("Compressor");
			if (settingsNode)
			{
				fl::XMLNode* inputNode = settingsNode->FindChild("Input");
				if (inputNode)
					input = inputNode->value;
				fl::XMLNode* outputNode = settingsNode->FindChild("Output");
				if (outputNode)
					output = outputNode->value;
				fl::XMLNode* qualityNode = settingsNode->FindChild("Quality");
				if (qualityNode)
					quality = atoi(qualityNode->value.c_str());
				fl::XMLNode* compressionNode = settingsNode->FindChild("Compression");
				if (compressionNode)
					compression = atoi(compressionNode->value.c_str());
				fl::XMLNode* windowSizeNode = settingsNode->FindChild("WindowSize");
				if (windowSizeNode)
					windowSize = atoi(windowSizeNode->value.c_str());
			}
		}
	}
	else
	{
		input = String(argv[2]);
		if (argc > 3)
			output = String(argv[3]);

		if (argc > 4)
			quality = (byte)atoi(argv[4]);

		if (argc > 5)
			compression = atoi(argv[5]);

		if (argc > 6)
			windowSize = atoi(argv[6]);

		if (argc > 7)
		{
			String metadataFile = argv[7];

			fl::XMLDocument document(metadataFile);
			fl::XMLNode* rootNode = document.FindNode("Metadata");
			FL_ASSERT(rootNode);

			for (fl::XMLNode& e : rootNode->FindChild("Events")->children)
			{
				Metadata::Event ev;
				ev.name = e.FindAttribute("name")->value;
				ev.startFrame = atoi(e.FindChild("StartFrame")->value.c_str());
				ev.endFrame = atoi(e.FindChild("EndFrame")->value.c_str());
				metadata.events.push_back(ev);
			}

			for (fl::XMLNode& a : rootNode->FindChild("Animations")->children)
			{
				Metadata::Animation anim;
				anim.name = a.FindAttribute("name")->value;
				anim.startFrame = atoi(a.FindChild("StartFrame")->value.c_str());
				anim.endFrame = atoi(a.FindChild("EndFrame")->value.c_str());
				anim.mode = (Header::AnimationMode)atoi(a.FindChild("Mode")->value.c_str());
				metadata.animations.push_back(anim);
			}
			hasMetadata = true;
		}
	}

	std::vector<String> files = GetAllFiles(input, "*");

	std::cout << "Reading " << files.size() << " images... ";
	std::vector<Sprite> sprites;
	for (int i = 0; i < files.size(); i++)
		sprites.push_back(Sprite(files[i]));
	std::cout << "Done." << std::endl;

	Compressor compressor(sprites, metadata, quality, (Header::CompressionType)compression, windowSize);
	compressor.Compress(output);

	return 0;
}