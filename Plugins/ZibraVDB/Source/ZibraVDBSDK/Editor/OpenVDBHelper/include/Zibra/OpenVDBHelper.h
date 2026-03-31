#pragma once

#include <filesystem>
#include <Zibra/CE/Compression.h>

namespace Zibra::OpenVDBHelper
{
    struct SequenceInfo
    {
        uint64_t originalSize;
        uint32_t frameCount;
        uint32_t channelCount;
        char** channelNames;
    };

    Zibra::CE::Compression::SparseFrame* ReadFrame(const wchar_t* filePath, const char* const* orderedChannelNames,
                                                   size_t orderedChannelNamesCount) noexcept;

    SequenceInfo* ReadSequenceInfo(const wchar_t* const* filePaths, size_t fileCount, float* progress) noexcept;

    void ReleaseSequenceInfo(SequenceInfo* sequenceInfo) noexcept;

    void ReleaseSparseFrame(Zibra::CE::Compression::SparseFrame* sparseFrame) noexcept;
} // namespace Zibra::OpenVDBHelper

#pragma region CAPI

#define ZIB_CONCAT_HELPER(A, B) A##B
#define ZIB_PFN(name) ZIB_CONCAT_HELPER(PFN_, name)
#if defined(_MSC_VER)
#define ZIB_API_IMPORT extern "C" __declspec(dllimport)
#define ZIB_CALL_CONV __cdecl
#elif defined(__GNUC__)
#define ZIB_API_IMPORT extern "C"
#define ZIB_CALL_CONV
#else
#error "Unsupported compiler"
#endif
#define ZIB_NS Zibra::OpenVDBHelper

#ifndef ZIB_NO_CAPI_IMPL

#pragma region Funcs
#define ZIB_OPENVDBHELPER_FUNCS_EXPORT_FNPFX(name) Zibra_OpenVDBHelper_##name

#define ZIB_OPENVDBHELPER_FUNCS_API_APPLY(macro)                      \
    macro(ZIB_OPENVDBHELPER_FUNCS_EXPORT_FNPFX(ReadFrame));           \
    macro(ZIB_OPENVDBHELPER_FUNCS_EXPORT_FNPFX(ReadSequenceInfo));    \
    macro(ZIB_OPENVDBHELPER_FUNCS_EXPORT_FNPFX(ReleaseSequenceInfo)); \
    macro(ZIB_OPENVDBHELPER_FUNCS_EXPORT_FNPFX(ReleaseSparseFrame))

#define ZIB_FNPFX(name) ZIB_OPENVDBHELPER_FUNCS_EXPORT_FNPFX(name)

typedef Zibra::CE::Compression::SparseFrame*(ZIB_CALL_CONV* ZIB_PFN(ZIB_FNPFX(ReadFrame)))(const wchar_t* filePath,
                                                                                           const char* const* orderedChannelNames,
                                                                                           size_t orderedChannelNamesCount);
typedef Zibra::OpenVDBHelper::SequenceInfo*(ZIB_CALL_CONV* ZIB_PFN(ZIB_FNPFX(ReadSequenceInfo)))(const wchar_t* const* filePaths, size_t fileCount,
                                                                                                 float* progress);
typedef void(ZIB_CALL_CONV* ZIB_PFN(ZIB_FNPFX(ReleaseSequenceInfo)))(Zibra::OpenVDBHelper::SequenceInfo* sequenceInfo);
typedef void(ZIB_CALL_CONV* ZIB_PFN(ZIB_FNPFX(ReleaseSparseFrame)))(Zibra::CE::Compression::SparseFrame* sparseFrame);

#ifndef ZIB_NO_STATIC_API_DECL
ZIB_API_IMPORT Zibra::CE::Compression::SparseFrame* ZIB_CALL_CONV ZIB_FNPFX(ReadFrame)(const wchar_t* filePath,
                                                                                       const char* const* orderedChannelNames,
                                                                                       size_t orderedChannelNamesCount) noexcept;
ZIB_API_IMPORT Zibra::OpenVDBHelper::SequenceInfo* ZIB_CALL_CONV ZIB_FNPFX(ReadSequenceInfo)(const wchar_t* const* filePaths, size_t fileCount,
                                                                                             float* progress) noexcept;
ZIB_API_IMPORT void ZIB_CALL_CONV ZIB_FNPFX(ReleaseSequenceInfo)(Zibra::OpenVDBHelper::SequenceInfo* sequenceInfo) noexcept;
ZIB_API_IMPORT void ZIB_CALL_CONV ZIB_FNPFX(ReleaseSparseFrame)(Zibra::CE::Compression::SparseFrame* sparseFrame) noexcept;
#else
#define ZIB_DECLARE_API_EXTERN_FUNCS(name) extern ZIB_PFN(name) name;
ZIB_OPENVDBHELPER_FUNCS_API_APPLY(ZIB_DECLARE_API_EXTERN_FUNCS);
#undef ZIB_DECLARE_API_EXTERN_FUNCS
#endif

namespace ZIB_NS::CAPI
{
    inline Zibra::CE::Compression::SparseFrame* ReadFrame(const wchar_t* filePath, const char* const* orderedChannelNames,
                                                          size_t orderedChannelNamesCount) noexcept
    {
        return ZIB_FNPFX(ReadFrame)(filePath, orderedChannelNames, orderedChannelNamesCount);
    }
    inline Zibra::OpenVDBHelper::SequenceInfo* ReadSequenceInfo(const wchar_t* const* filePaths, size_t fileCount, float* progress) noexcept
    {
        return ZIB_FNPFX(ReadSequenceInfo)(filePaths, fileCount, progress);
    }
    inline void ReleaseSequenceInfo(Zibra::OpenVDBHelper::SequenceInfo* sequenceInfo) noexcept
    {
        ZIB_FNPFX(ReleaseSequenceInfo)(sequenceInfo);
    }
    inline void ReleaseSparseFrame(Zibra::CE::Compression::SparseFrame* sparseFrame) noexcept
    {
        ZIB_FNPFX(ReleaseSparseFrame)(sparseFrame);
    }
} // namespace ZIB_NS::CAPI

#undef ZIB_FNPFX
#pragma endregion Funcs

#define ZIB_OPENVDBHELPER_API_APPLY(macro) ZIB_OPENVDBHELPER_FUNCS_API_APPLY(macro)

#endif // ZIB_NO_CAPI_IMPL

#undef ZIB_NS
#undef ZIB_CALL_CONV
#undef ZIB_API_IMPORT

#pragma endregion CAPI
