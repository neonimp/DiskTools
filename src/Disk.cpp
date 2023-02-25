#include <Disk.hpp>
#include <memory>

DiskTools::Disk::Disk(const wchar_t *drivePath) {
    // copy the drive path buffer
    auto temp = new wchar_t[wcslen(drivePath) + 1];
    wcscpy_s(temp, wcslen(drivePath) + 1, drivePath);
    this->drivePath = new std::wstring(temp);
    // Initialize the class variables
    this->totalSize = 0;
    this->freeSize = 0;
    this->usedSize = 0;
    this->lastNTError = ERROR_SUCCESS;
    this->diskType = DiskType::Unknown;
    // Check if the drive path exists, and we can get a handle to it
    this->GetHandle();
    if (this->hDrive == nullptr) {
        // Set the last NT error
        this->lastNTError = GetLastError();
        return;
    }
    // Get the disk type
    this->QueryDiskGeometry();
    if (this->HasError()) {
        return;
    }
    printf("Done initializing Disk\n");
}

DiskTools::Disk::~Disk() {
    // Close the handle to the drive
    CloseHandle(this->hDrive);
}

uint64_t DiskTools::Disk::GetTotalSize() const {
    return this->totalSize;
}

uint64_t DiskTools::Disk::GetFreeSize() const {
    return this->freeSize;
}

uint64_t DiskTools::Disk::GetUsedSize() const {
    return this->usedSize;
}

DiskTools::DiskType DiskTools::Disk::GetDiskType() {
    return this->diskType;
}

/// @brief Get the drive path
/// @return A pointer to a wchar_t array containing the drive path, the caller is responsible for freeing the memory
std::wstring *DiskTools::Disk::GetDrivePath() {
    // Copy the drive path to a new variable, to avoid memory leaks
    auto drivePathCopy = new std::wstring(*this->drivePath);
    return drivePathCopy;
}

bool DiskTools::Disk::HasError() const {
    return this->lastNTError != ERROR_SUCCESS;
}

DWORD DiskTools::Disk::GetLastNTError() const {
    return this->lastNTError;
}

void DiskTools::Disk::GetHandle() {
    printf("Opening drive: %ls\n", this->drivePath->c_str());
    // Open the drive
    this->hDrive = CreateFileW(
            this->drivePath->c_str(), // Drive to open
            GENERIC_READ, // Open for reading
            FILE_SHARE_READ, // Share for reading
            nullptr, // Default security
            OPEN_EXISTING, // Open existing drive
            0, // Normal file
            nullptr // No attr. template
    );
    if (this->hDrive == INVALID_HANDLE_VALUE) {
        this->lastNTError = GetLastError();
        return;
    }
}

void DiskTools::Disk::QueryDiskGeometry() {
    // Get the disk geometry
    if (!DeviceIoControl(this->hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0, &this->diskGeometry,
                         sizeof(this->diskGeometry), nullptr, &this->overlapped)) {
        this->lastNTError = GetLastError();
        return;
    }
    // Query the disk type
    switch (this->diskGeometry.Geometry.MediaType) {
        case FixedMedia:
            this->diskType = DiskType::Fixed;
            break;
        case RemovableMedia:
            this->diskType = DiskType::Removable;
            break;
        default:
            this->diskType = DiskType::Unknown;
            break;
    }
    auto driveLayout = new DRIVE_LAYOUT_INFORMATION_EX();
    while (true) {
        // Get the drive layout
        if (!DeviceIoControl(this->hDrive, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, nullptr, 0, driveLayout,
                             sizeof(DRIVE_LAYOUT_INFORMATION_EX) + driveLayout->PartitionCount * sizeof(PARTITION_INFORMATION_EX),
                             nullptr, &this->overlapped)) {
            this->lastNTError = GetLastError();
        }

        if (this->lastNTError == ERROR_INSUFFICIENT_BUFFER) {
            // The buffer was too small, so we need to resize it
            auto newSize = driveLayout->PartitionCount * sizeof(PARTITION_INFORMATION_EX);
            delete driveLayout;
            driveLayout = (DRIVE_LAYOUT_INFORMATION_EX *) new char[newSize];
            this->lastNTError = ERROR_SUCCESS;
            continue;
        } else if (this->lastNTError != ERROR_SUCCESS) {
            // Some other error occurred
            return;
        }
        // No error occurred, so we can break out of the loop
        break;
    }
    // Get the disk length
    this->totalSize = this->diskGeometry.DiskSize.QuadPart;
    // Get the free size
    this->freeSize = driveLayout->PartitionStyle == PARTITION_STYLE_MBR ? driveLayout->Mbr.Signature : driveLayout->Gpt.DiskId.Data1;
}

DiskTools::Disk::Disk() {
    // Initialize the class variables
    this->drivePath = nullptr;
    this->totalSize = 0;
    this->freeSize = 0;
    this->usedSize = 0;
    this->lastNTError = ERROR_SUCCESS;
    this->diskType = DiskType::Unknown;
}

std::wstring DiskTools::Disk::GetLastNTErrorStringW(uint64_t langId) const {
    // Get the error message
    auto errorMessage = new wchar_t[1024];
    auto size = FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            this->lastNTError,
            langId,
            errorMessage,
            1024,
            nullptr
    );
    // Convert the error message to a string
    std::wstring errString(errorMessage, size);
    // Free the memory
    delete[] errorMessage;
    // Return the error message
    return errString;
}
