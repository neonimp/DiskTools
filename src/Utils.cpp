#include <Utils.hpp>
#include <iostream>
#include <utility>
#include <vector>
#include <format>

namespace DiskTools {
    std::vector<Types::VolumeInfo> Utils::ListVolumes(bool stopOnException) {
        // Get the logical drives
        auto logicalDrives = GetLogicalDrives();
        // Create a string to store the disk paths
        auto volumes = std::vector<Types::VolumeInfo>();
        // Loop through the logical drives
        for (uint32_t i = 0; i < 26; i++) {
            // Check if the drive exists
            if (logicalDrives & (1 << i)) {
                // Create a buffer to store the drive path
                auto drivePath = std::make_unique<std::wstring>(L"");
                auto volumeName = std::make_unique<std::wstring>(L"");
                auto diskPath = std::make_unique<std::wstring>(L"\\\\.\\PhysicalDrive");
                // Create the drive path
                drivePath->append(1, 'A' + (wchar_t) i);
                drivePath->append(L":\\");
                if (auto volumeNameBuff = std::array<wchar_t, MAX_PATH + 1>();
                        GetVolumeNameForVolumeMountPointW(drivePath->c_str(), volumeNameBuff.data(), MAX_PATH + 1) !=
                        0) {
                    volumeName->append(volumeNameBuff.data());

                } else {
                    throw Types::DiskToolsException(std::wstring(L"Failed to get volume name from drive path"), GetLastError(),
                                             *drivePath);
                }

                // Get disk path from volume name
                if (auto drivePathBuff = std::array<wchar_t, MAX_PATH + 1>();
                        GetVolumePathNamesForVolumeNameW(volumeName->c_str(), drivePathBuff.data(), MAX_PATH + 1,
                                                         nullptr) != 0) {
                    drivePath->append(drivePathBuff.data());
                } else {
                    throw Types::DiskToolsException(std::wstring(L"Failed to get drive path from volume name"), GetLastError(),
                                             *volumeName);
                }
                // Create the disk path
                if (stopOnException) {
                    auto volume = GetVolumeInfo(volumeName.get());
                    volumes.push_back(volume);
                } else {
                    try {
                        auto volume = GetVolumeInfo(volumeName.get());
                        volumes.push_back(volume);
                    } catch (Types::DiskToolsException &e) {
                        continue;
                    }
                }
            }
        }
        return volumes;
    }

    Types::VolumeInfo Utils::GetVolumeInfo(const std::wstring *volumeName) {
        // Strip the \ from the volume name
        auto volumeNameCopy = std::make_unique<std::wstring>(*volumeName);
        if (volumeNameCopy->ends_with(L"\\")) {
            volumeNameCopy->pop_back();
        }
        // Get a handle to the volume
        auto hVolume = CreateFileW(volumeNameCopy->c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0,
                                   nullptr);
        if (hVolume == INVALID_HANDLE_VALUE) {
            throw Types::DiskToolsException(std::wstring(L"Failed to open volume"), GetLastError(), *volumeNameCopy);
        }
        auto singleExtent = VOLUME_DISK_EXTENTS{};
        auto extents = std::unique_ptr<VOLUME_DISK_EXTENTS>(nullptr);
        auto workingBuffer = std::make_unique<uint8_t[]>(sizeof(VOLUME_DISK_EXTENTS));
        auto extentCount = 0U;

        // Get the disk extents
        if (auto bytesReturned = 0;
                DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                nullptr,
                                0,
                                &singleExtent,
                                sizeof(singleExtent),
                                (LPDWORD) &bytesReturned,
                                nullptr)
                ) {
            // Single extent
            extents = std::make_unique<VOLUME_DISK_EXTENTS>(singleExtent);
            extentCount = extents->NumberOfDiskExtents;
        } else {
            // Multiple extents
            auto lastQuery = &singleExtent;
            while (GetLastError() == ERROR_MORE_DATA) {
                extentCount = lastQuery->NumberOfDiskExtents;
                auto sizeNeeded = FIELD_OFFSET(VOLUME_DISK_EXTENTS, Extents[extentCount]);
                workingBuffer.reset(new uint8_t[sizeNeeded]);
                // This cast is safe because we are resizing the working buffer
                lastQuery = (VOLUME_DISK_EXTENTS *) (workingBuffer.get());
                if (DeviceIoControl(hVolume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                    nullptr,
                                    0,
                                    lastQuery,
                                    sizeNeeded,
                                    (LPDWORD) &bytesReturned,
                                    nullptr)) {
                    extents = std::make_unique<VOLUME_DISK_EXTENTS>(*lastQuery);
                    break;
                }
            }
        }
        CloseHandle(hVolume);

        auto volumeInfo = Types::VolumeInfo();
        auto extentsProc = std::vector<Types::DiskExtent>(extentCount);
        volumeInfo.extentCount = extentCount;
        volumeInfo.volumeName = *volumeNameCopy;
        volumeInfo.volumePath = *volumeNameCopy;

        for (auto i = 0; i < extentCount; i++) {
            auto diskExtent = Types::DiskExtent();
            diskExtent.diskNumber = extents->Extents[i].DiskNumber;
            diskExtent.startingOffset = extents->Extents[i].StartingOffset.QuadPart;
            diskExtent.extentLength = extents->Extents[i].ExtentLength.QuadPart;
        }

        volumeInfo.extents = extentsProc.data();

        return volumeInfo;
    }

    size_t Utils::CountVolumes() {
        // Get the logical drives
        DWORD logicalDrives = GetLogicalDrives();
        // Count the logical drives
        size_t count = 0;
        for (uint32_t i = 0; i < 26; i++) {
            // Check if the drive exists
            if (logicalDrives & (1 << i)) {
                count++;
            }
        }
        // Return the count
        return count;
    }

    std::string Utils::GetDiskTypeString(DiskType diskType) {
        switch (diskType) {
            case DiskType::Fixed:
                return "Fixed";
            case DiskType::Removable:
                return "Removable";
            default:
                return "Unknown";
        }
    }

    std::wstring Utils::FormatNTErrorW(uint32_t langId, uint32_t errNo, bool keepNewLine) {
        // Get the error message
        auto errorMessage = std::array<wchar_t, 1024>();
        auto size = FormatMessageW(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                errNo,
                langId,
                errorMessage.data(),
                1024,
                nullptr
        );
        // Convert the error message to a string
        std::wstring errString(errorMessage.data(), size);
        // Remove the new line
        if (!keepNewLine) {
            errString.pop_back();
            errString.pop_back();
        }
        // Return the error message
        return errString;
    }
}
