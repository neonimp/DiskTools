#pragma once
#if !defined(TYPES_H_)
#define TYPES_H_
#define DLLExport __declspec(dllexport)

#include <cstdint>
#include <string>
#include <vector>
#include <gsl/gsl>
#include <Windows.h>
#include <Disk.hpp>

namespace DiskTools::Types {
    struct DLLExport DiskExtent {
        uint64_t diskNumber;
        uint64_t startingOffset;
        uint64_t extentLength;
    };

    struct DLLExport VolumeInfo {
        std::wstring volumeName;
        std::wstring volumePath;
        std::wstring drivePath;
        uint32_t extentCount{};
        DiskExtent *extents{};
    };

    struct DLLExport PartitionInfo {
        uint64_t partitionNumber;
        uint64_t startingOffset;
        uint64_t partitionLength;
        uint32_t partitionType;
        bool bootIndicator;
        bool recognizedPartition;
        bool rewritePartition;
    };

    struct DLLExport DiskInfo {
        std::wstring diskPath;
        bool isRemovable{};
        bool isReadOnly{};
        bool isGpt{};
        bool isMbr{};
        uint32_t partitionCount{};
        PartitionInfo *partitions{};
    };

    DLLExport std::wstring VolumeInfoToString(VolumeInfo &volumeInfo);

    DLLExport std::wstring DiskExtentToString(DiskExtent &diskExtent);

    DLLExport std::wstring DiskExtentToString(std::vector<DiskExtent> diskExtents);
}

#endif //TYPES_H_