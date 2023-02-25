#pragma once
#if !defined(DISKINFO_H_)
#define DISKINFO_H_
#define DLLExport __declspec(dllexport)

#include <cstdint>
#include <string>
#include <vector>
#include <gsl/gsl>
#include <Windows.h>

namespace DiskTools {
    enum class DiskType {
        Unknown,
        Removable,
        Fixed,
        Network,
        CDROM,
        RAMDisk
    };

    /**
     * @brief This class can be used to get information about a disk,
     * such as the total size, free size, used size and the disk type.
     * @warning When you create an instance of this class, it will try to open a handle to the drive,
     * it falls on the caller to check HasError() to see if the handle was opened successfully, if not,
     * call LastNTError() to get the error code.
     */
    class DLLExport Disk {
    public:
        Disk();

        explicit Disk(const wchar_t *drivePath);

        [[nodiscard]] uint64_t GetTotalSize() const;

        [[nodiscard]] uint64_t GetFreeSize() const;

        [[nodiscard]] uint64_t GetUsedSize() const;

        [[nodiscard]] DWORD GetLastNTError() const;

        [[nodiscard]] std::wstring GetLastNTErrorStringW(uint64_t langId) const;

        DiskType GetDiskType();

        std::wstring *GetDrivePath();

        [[nodiscard]] bool HasError() const;

        ~Disk();

    private:
        std::wstring *drivePath;
        uint64_t totalSize;
        uint64_t freeSize;
        uint64_t usedSize;
        DiskType diskType;
        DISK_GEOMETRY_EX diskGeometry{};
        LARGE_INTEGER diskLength{};
        OVERLAPPED overlapped{};
        DWORD lastNTError;
        HANDLE hDrive{};

        void GetHandle();

        void QueryDiskGeometry();

    };
}

#endif // DISKINFO_H_
