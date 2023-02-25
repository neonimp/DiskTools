#include <Disk.hpp>
#include <Utils.hpp>
#include <iostream>
#include <string>

int main() {
    std::cout << "Dumping disk info..." << std::endl;
//    auto c_disk_info = DiskTools::Disk(L"\\\\.\\PhysicalDrive0");
//    if (c_disk_info.HasError()) {
//        std::wcout << "Error: " << c_disk_info.GetLastNTErrorStringW(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT))
//                   << c_disk_info.GetLastNTError() << std::endl;
//        return 1;
//    }
//    std::cout << "Disk type: " << DiskTools::Utils::GetDiskTypeString(c_disk_info.GetDiskType()) << std::endl;
//    std::cout << "Total size: " << c_disk_info.GetTotalSize() << std::endl;
//    std::cout << "Free size: " << c_disk_info.GetFreeSize() << std::endl;
//    std::cout << "Used size: " << c_disk_info.GetUsedSize() << std::endl;
    std::wcout << "listing: " << DiskTools::Utils::CountVolumes() << " volumes" << std::endl;
    try {
        auto volumes = DiskTools::Utils::ListVolumes(false);
        for (auto &volume: volumes) {
            std::wcout << DiskTools::Types::VolumeInfoToString(volume) << std::endl;
        }
    } catch (DiskTools::DiskToolsException &e) {
        std::wcout << std::endl << "Error: " << e.whatW() << std::endl;
    }
    return 0;
}
