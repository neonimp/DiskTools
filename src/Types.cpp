#include <Types.hpp>
#include <Utils.hpp>
#include <format>

namespace DiskTools::Types {
    DiskToolsException::DiskToolsException(std::wstring message, uint32_t NTError,
                                           DiskToolsExceptionType exceptionType,
                                           std::wstring exceptionContext) : exceptionType(exceptionType),
                                                                            NTError(NTError) {
        // Set the message
        this->message = std::move(message);
        // Set the formatted error
        this->formattedNTError = Utils::FormatNTErrorW(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), NTError);
        // Set the exception context
        this->exceptionContext = std::move(exceptionContext);
        // Format the exception
        auto formatted = std::format(L"DiskToolsException: {} ({}: {}) context: {}", message, NTError, formattedNTError,
                                     exceptionContext);
        this->whatStr = std::wstring(formatted);
    }

    const wchar_t *DiskToolsException::whatW() const noexcept {
        return this->whatStr.c_str();
    }

    DiskToolsException::DiskToolsException(const std::wstring &message, uint32_t i,
                                           const std::wstring &pString)
            : message(message), exceptionContext(pString), NTError(i) {
        this->formattedNTError = Utils::FormatNTErrorW(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), i);
        this->exceptionType = DiskToolsExceptionType::NTError;
        auto formatted = std::format(L"DiskToolsException: {} ({}: {}) context: {}", message, NTError, formattedNTError,
                                     pString);
        this->whatStr = std::wstring(formatted);
    }

    bool DiskToolsException::HasFurtherInfo() const noexcept {
        if (this->exceptionType != DiskToolsExceptionType::NTError) {
            return false;
        } else {
            switch (this->NTError) {
                case ERROR_SHARING_VIOLATION:
                case ERROR_ACCESS_DENIED:
                    return true;
                default:
                    return false;
            }
        }
    }

    const wchar_t *DiskToolsException::GetFurtherInfoW() const noexcept {
        if (!this->HasFurtherInfo())
            return L"";

        switch (this->NTError) {
            case ERROR_SHARING_VIOLATION:
                return L"Try closing any programs that may be using the disk.";
            case ERROR_ACCESS_DENIED:
                return L"Try running the program as administrator.";
            default:
                return L"";
        }
    }

    std::wstring Types::VolumeInfoToString(VolumeInfo &volumeInfo) {
        auto extents = std::wstring();
        for (auto i = 0; i < volumeInfo.extentCount; i++) {
            auto extent = volumeInfo.extents[i];
            extents += std::format(L"Disk: {}, Offset: {}, Length: {}",
                                   extent.diskNumber,
                                   extent.startingOffset,
                                   extent.extentLength);
            if (i != volumeInfo.extentCount - 1) {
                extents += L", ";
            }
        }
        return std::format(L"Volume: {}, Path: {}, Extents: {}",
                           volumeInfo.volumeName,
                           volumeInfo.volumePath,
                           extents);
    }

    std::wstring DiskExtentToString(Types::DiskExtent &diskExtent) {
        return std::format(L"Disk: {}, Offset: {}, Length: {}",
                           diskExtent.diskNumber,
                           diskExtent.startingOffset,
                           diskExtent.extentLength);
    }

    std::wstring DiskExtentToString(std::vector<Types::DiskExtent> diskExtents) {
        auto extents = std::wstring();
        for (auto i = 0; i < diskExtents.size(); i++) {
            auto extent = diskExtents[i];
            extents += std::format(L"Disk: {}, Offset: {}, Length: {}",
                                   extent.diskNumber,
                                   extent.startingOffset,
                                   extent.extentLength);
            if (i != diskExtents.size() - 1) {
                extents += L", ";
            }
        }
        return extents;
    }
}
