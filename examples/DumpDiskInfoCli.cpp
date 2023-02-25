#include <Disk.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>


int main() {
    std::vector<DiskTools::Types::VolumeInfo> volumes;
    std::cout << "Dumping disk info..." << std::endl;
    std::wcout << "listing: " << DiskTools::Utils::CountVolumes() << " volumes" << std::endl;
    try {
        volumes = DiskTools::Utils::ListVolumes(false);
        for (auto &volume: volumes) {
            std::wcout << DiskTools::Types::VolumeInfoToString(volume) << std::endl;
        }
    } catch (DiskTools::Types::DiskToolsException &e) {
        std::wcout << std::endl << "Error: " << e.whatW() << std::endl;
        return 1;
    }

    return 0;
}
