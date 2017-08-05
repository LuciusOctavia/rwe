#include <rwe/Gaf.h>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

int listCommand(const std::string& filename)
{
    std::cout << "GAF archive: " << filename << std::endl;
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    std::cout << "Opening..." << std::endl;
    rwe::GafArchive archive(&file);

    std::cout << "Enumerating contents..." << std::endl;

    for (auto& entry : archive.entries())
    {
        std::cout << entry.name << ", " << entry.frameOffsets.size() << " frames" << std::endl;
    }

    return 0;
}

class GafAdapter : public rwe::GafReaderAdapter
{
private:
    std::size_t frameCount;
    std::unique_ptr<char[]> currentFrame;
    rwe::GafFrameData currentFrameHeader;
    fs::path destPath;
public:
    explicit GafAdapter(const std::string& destPath) : frameCount(0), currentFrame(), destPath(destPath) {}
    void beginFrame(const rwe::GafFrameData& header) override
    {
        std::cout << "Beginning frame " << frameCount << std::endl;
        currentFrameHeader = header;
        currentFrame = std::make_unique<char[]>(header.width * header.height);
    }

    void frameLayer(const LayerData& data) override
    {
        // copy the layer onto the frame
        for (std::size_t y = 0; y < data.height; ++y)
        {
            for (std::size_t x = 0; x < data.width; ++x)
            {
                auto outPosX = static_cast<int>(x) - (data.x - currentFrameHeader.posX);
                auto outPosY = static_cast<int>(y) - (data.y - currentFrameHeader.posY);

                assert(outPosX >= 0);
                assert(outPosX < currentFrameHeader.width);
                assert(outPosY >= 0);
                assert(outPosY < currentFrameHeader.height);

                currentFrame[(outPosY * currentFrameHeader.width) + outPosX] = data.data[(y * data.width) + x];
            }
        }
    }

    void endFrame() override
    {
        fs::path fullPath = destPath / std::to_string(frameCount);
        std::ofstream out(fullPath.string(), std::ios::binary);
        out.write(currentFrame.get(), currentFrameHeader.width * currentFrameHeader.height);
        std::cout << "Finished frame " << frameCount << std::endl;
        ++frameCount;
    }
};

int extractCommand(const std::string& gafPath, const std::string& entryName, const std::string& destinationPath)
{
    std::cout << "GAF archive: " << gafPath << std::endl;
    std::ifstream file(gafPath, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    std::cout << "Opening..." << std::endl;
    rwe::GafArchive archive(&file);

    std::cout << "Finding file..." << std::endl;
    auto entry = archive.findEntry(entryName);
    if (!entry)
    {
        std::cerr << "Could not find entry '" << entryName << "' inside archive." << std::endl;
        return 1;
    }

    std::cout << "Extracting..." << std::endl;

    GafAdapter adapter(destinationPath);
    archive.extract(*entry, adapter);

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Specify a command" << std::endl;
        return 1;
    }

    std::string command(argv[1]);

    if (command == "list")
    {
        if (argc < 3)
        {
            std::cerr << "Specify a GAF file to list" << std::endl;
            return 1;
        }

        return listCommand(argv[2]);
    }

    if (command == "extract")
    {
        if (argc < 5)
        {
            std::cerr << "Specify GAF file, file to extract and destination" << std::endl;
            return 1;
        }

        return extractCommand(argv[2], argv[3], argv[4]);
    }

    std::cerr << "Unrecognised command: " << command << std::endl;
    return 1;
}