#pragma once
#if !defined(DISKINFOUTILS_H_)
#define DISKINFOUTILS_H_
#define DLLExport __declspec(dllexport)

#include <cstdint>
#include <string>
#include <vector>
#include <gsl/gsl>
#include <Windows.h>
#include <Disk.hpp>
#include <Types.hpp>

namespace DiskTools {

    /**
     * @brief This namespace contains utility functions for the DiskTools library that are not part of the Disk class.
     * These are functions that are used internally by the Disk class, but are also useful for other purposes.
     */
    namespace Utils {
        DLLExport size_t CountVolumes();

        /**
         * @brief Get a list of all volumes on the system
         * @param stopOnException If true, the function will stop when an exception is thrown,
         * otherwise it will continue and return the volumes that did not throw an exception.
         * @return A vector containing VolumeInfo structs
         */
        DLLExport std::vector<Types::VolumeInfo> ListVolumes(bool stopOnException = true);

        /**
         * @brief Get the VolumeInfo struct for the specified volume name (e.g. \\\\?\\Volume{1234-5678})
         * @param volumeName The volume name (e.g. \\\\?\\Volume{1234-5678}) it will strip a right slash if it is present
         * @return A VolumeInfo struct containing information about the volume and its extents
         */
        DLLExport Types::VolumeInfo GetVolumeInfo(const std::wstring *volumeName);

        /**
         * @brief Get the wstring representation of a DiskType enum value
         * @param diskType The DiskType enum value
         * @return A wstring containing the wstring representation of the DiskType enum value
         */
        DLLExport std::string GetDiskTypeString(DiskTools::DiskType diskType);

        /**
         * @brief Get the formatted NT error message for the specified error code and language ID
         * @param langId A language ID, for example: MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)
         * @param errNo The NT error code, for example: ERROR_FILE_NOT_FOUND or a call to GetLastError()
         * @param keepNewLine If true, the new line character will be kept in the error message, otherwise it will be removed
         * @return A wstring containing the formatted NT error message
         */
        DLLExport std::wstring FormatNTErrorW(uint32_t langId, uint32_t errNo, bool keepNewLine = false);
    };

    enum class DiskToolsExceptionType {
        Unknown,
        NTError,
        ENoEnt,
    };

    class DLLExport DiskToolsException : public std::exception {
    public:
        DiskToolsException(std::wstring message, uint32_t NTError, DiskToolsExceptionType exceptionType,
                           std::wstring exceptionContext);

        DiskToolsException(const std::wstring &message, uint32_t i, const std::wstring &pString);

        [[nodiscard]] const wchar_t *whatW() const noexcept;

        /**
         * @brief On some cases we can attempt to get more information about the exception
         * @return A string containing more information about the exception,
         * or an empty string if no further information is available.
         */
        [[nodiscard]] const wchar_t *GetFurtherInfoW() const noexcept;

        [[nodiscard]] bool HasFurtherInfo() const noexcept;

    private:
        std::wstring message;
        std::wstring formattedNTError;
        DiskToolsExceptionType exceptionType;
        std::wstring exceptionContext;
        uint32_t NTError;
        std::wstring whatStr;
    };
}

#endif // DISKINFOUTILS_H_