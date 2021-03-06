//**********************************************************************************`
//* This is an include file generated by Message Compiler.                         *`
//*                                                                                *`
//* The MIT License(MIT)                                                           *`
//*                                                                                *`
//* Copyright(c) 2015 Aleksey Kabanov                                              *`
//*                                                                                *`
//* Permission is hereby granted, free of charge, to any person obtaining a copy   *`
//* of this software and associated documentation files(the "Software"), to deal   *`
//* in the Software without restriction, including without limitation the rights   *`
//* to use, copy, modify, merge, publish, distribute, sublicense, and / or sell    *`
//* copies of the Software, and to permit persons to whom the Software is          *`
//* furnished to do so, subject to the following conditions :                      *`
//*                                                                                *`
//* The above copyright notice and this permission notice shall be included in all *`
//* copies or substantial portions of the Software.                                *`
//*                                                                                *`
//* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *`
//* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       *`
//* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE     *`
//* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         *`
//* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  *`
//* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  *`
//* SOFTWARE.                                                                      *`
//**********************************************************************************`
#pragma once
#include <wmistr.h>
#include <evntrace.h>
#include "evntprov.h"
//
//  Initial Defs
//
#if !defined(ETW_INLINE)
#define ETW_INLINE DECLSPEC_NOINLINE __inline
#endif

#if defined(__cplusplus)
extern "C" {
#endif

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION
#if  !defined(McGenDebug)
#define McGenDebug(a,b)
#endif


#if !defined(MCGEN_TRACE_CONTEXT_DEF)
#define MCGEN_TRACE_CONTEXT_DEF
typedef struct _MCGEN_TRACE_CONTEXT
{
    TRACEHANDLE            RegistrationHandle;
    TRACEHANDLE            Logger;
    ULONGLONG              MatchAnyKeyword;
    ULONGLONG              MatchAllKeyword;
    ULONG                  Flags;
    ULONG                  IsEnabled;
    UCHAR                  Level; 
    UCHAR                  Reserve;
    USHORT                 EnableBitsCount;
    PULONG                 EnableBitMask;
    const ULONGLONG*       EnableKeyWords;
    const UCHAR*           EnableLevel;
} MCGEN_TRACE_CONTEXT, *PMCGEN_TRACE_CONTEXT;
#endif

#if !defined(MCGEN_LEVEL_KEYWORD_ENABLED_DEF)
#define MCGEN_LEVEL_KEYWORD_ENABLED_DEF
FORCEINLINE
BOOLEAN
McGenLevelKeywordEnabled(
    _In_ PMCGEN_TRACE_CONTEXT EnableInfo,
    _In_ UCHAR Level,
    _In_ ULONGLONG Keyword
    )
{
    //
    // Check if the event Level is lower than the level at which
    // the channel is enabled.
    // If the event Level is 0 or the channel is enabled at level 0,
    // all levels are enabled.
    //

    if ((Level <= EnableInfo->Level) || // This also covers the case of Level == 0.
        (EnableInfo->Level == 0)) {

        //
        // Check if Keyword is enabled
        //

        if ((Keyword == (ULONGLONG)0) ||
            ((Keyword & EnableInfo->MatchAnyKeyword) &&
             ((Keyword & EnableInfo->MatchAllKeyword) == EnableInfo->MatchAllKeyword))) {
            return TRUE;
        }
    }

    return FALSE;

}
#endif

#if !defined(MCGEN_EVENT_ENABLED_DEF)
#define MCGEN_EVENT_ENABLED_DEF
FORCEINLINE
BOOLEAN
McGenEventEnabled(
    _In_ PMCGEN_TRACE_CONTEXT EnableInfo,
    _In_ PCEVENT_DESCRIPTOR EventDescriptor
    )
{

    return McGenLevelKeywordEnabled(EnableInfo, EventDescriptor->Level, EventDescriptor->Keyword);

}
#endif


//
// EnableCheckMacro
//
#ifndef MCGEN_ENABLE_CHECK
#define MCGEN_ENABLE_CHECK(Context, Descriptor) (Context.IsEnabled &&  McGenEventEnabled(&Context, &Descriptor))
#endif

#if !defined(MCGEN_CONTROL_CALLBACK)
#define MCGEN_CONTROL_CALLBACK

DECLSPEC_NOINLINE __inline
VOID
__stdcall
McGenControlCallbackV2(
    _In_ LPCGUID SourceId,
    _In_ ULONG ControlCode,
    _In_ UCHAR Level,
    _In_ ULONGLONG MatchAnyKeyword,
    _In_ ULONGLONG MatchAllKeyword,
    _In_opt_ PEVENT_FILTER_DESCRIPTOR FilterData,
    _Inout_opt_ PVOID CallbackContext
    )
/*++

Routine Description:

    This is the notification callback for Vista.

Arguments:

    SourceId - The GUID that identifies the session that enabled the provider. 

    ControlCode - The parameter indicates whether the provider 
                  is being enabled or disabled.

    Level - The level at which the event is enabled.

    MatchAnyKeyword - The bitmask of keywords that the provider uses to 
                      determine the category of events that it writes.

    MatchAllKeyword - This bitmask additionally restricts the category 
                      of events that the provider writes. 

    FilterData - The provider-defined data.

    CallbackContext - The context of the callback that is defined when the provider 
                      called EtwRegister to register itself.

Remarks:

    ETW calls this function to notify provider of enable/disable

--*/
{
    PMCGEN_TRACE_CONTEXT Ctx = (PMCGEN_TRACE_CONTEXT)CallbackContext;
    ULONG Ix;
#ifndef MCGEN_PRIVATE_ENABLE_CALLBACK_V2
    UNREFERENCED_PARAMETER(SourceId);
    UNREFERENCED_PARAMETER(FilterData);
#endif

    if (Ctx == NULL) {
        return;
    }

    switch (ControlCode) {

        case EVENT_CONTROL_CODE_ENABLE_PROVIDER:
            Ctx->Level = Level;
            Ctx->MatchAnyKeyword = MatchAnyKeyword;
            Ctx->MatchAllKeyword = MatchAllKeyword;
            Ctx->IsEnabled = EVENT_CONTROL_CODE_ENABLE_PROVIDER;

            for (Ix = 0; Ix < Ctx->EnableBitsCount; Ix += 1) {
                if (McGenLevelKeywordEnabled(Ctx, Ctx->EnableLevel[Ix], Ctx->EnableKeyWords[Ix]) != FALSE) {
                    Ctx->EnableBitMask[Ix >> 5] |= (1 << (Ix % 32));
                } else {
                    Ctx->EnableBitMask[Ix >> 5] &= ~(1 << (Ix % 32));
                }
            }
            break;

        case EVENT_CONTROL_CODE_DISABLE_PROVIDER:
            Ctx->IsEnabled = EVENT_CONTROL_CODE_DISABLE_PROVIDER;
            Ctx->Level = 0;
            Ctx->MatchAnyKeyword = 0;
            Ctx->MatchAllKeyword = 0;
            if (Ctx->EnableBitsCount > 0) {
                RtlZeroMemory(Ctx->EnableBitMask, (((Ctx->EnableBitsCount - 1) / 32) + 1) * sizeof(ULONG));
            }
            break;
 
        default:
            break;
    }

#ifdef MCGEN_PRIVATE_ENABLE_CALLBACK_V2
    //
    // Call user defined callback
    //
    MCGEN_PRIVATE_ENABLE_CALLBACK_V2(
        SourceId,
        ControlCode,
        Level,
        MatchAnyKeyword,
        MatchAllKeyword,
        FilterData,
        CallbackContext
        );
#endif
   
    return;
}

#endif
#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION
//+
// Provider LazyCopyDriver Event Count 11
//+
EXTERN_C __declspec(selectany) const GUID LazyCopyDriverGuid = {0x0fe08ee4, 0xb08f, 0x4d27, {0x8c, 0xbb, 0xc8, 0x16, 0x30, 0x8a, 0xe2, 0x35}};

//
// Channel
//
#define LazyCopyDriverGuid_CHANNEL_System 0x8

//
// Tasks
//
#define File_Fetch 0x1
EXTERN_C __declspec(selectany) const GUID File_FetchId = {0xbfdb9f63, 0x0939, 0x4a77, {0x93, 0x82, 0x2f, 0x31, 0x70, 0x5d, 0x5d, 0xba}};
#define Driver_Init 0x2
EXTERN_C __declspec(selectany) const GUID Driver_InitId = {0x5d9eda03, 0xb418, 0x4f19, {0x86, 0xa4, 0xdd, 0xc0, 0xea, 0xf2, 0x42, 0x2b}};
#define Configuration_Load 0x3
EXTERN_C __declspec(selectany) const GUID Configuration_LoadId = {0x0c74bd94, 0x1354, 0x4bce, {0x84, 0x7d, 0x97, 0x8c, 0xd1, 0x11, 0xf5, 0xb5}};
#define File_Open 0x4
EXTERN_C __declspec(selectany) const GUID File_OpenId = {0xe38de1ef, 0xe7ef, 0x4e73, {0x83, 0x95, 0x47, 0xaa, 0xf3, 0x2c, 0x29, 0xb5}};
//
// Keyword
//
#define Performance 0x2
#define Failure 0x1

//
// Event Descriptors
//
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR FileAccessedEvent = {0x1, 0x1, 0x0, 0x4, 0x0, 0x0, 0x0};
#define FileAccessedEvent_value 0x1
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR FileFetchedEvent = {0x2, 0x1, 0x0, 0x4, 0x0, 0x0, 0x0};
#define FileFetchedEvent_value 0x2
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR FileNotFetchedEvent = {0x3, 0x1, 0x8, 0x2, 0x0, 0x0, 0x8000000000000001};
#define FileNotFetchedEvent_value 0x3
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR File_Fetch_Start = {0x64, 0x1, 0x0, 0x4, 0x1, 0x1, 0x2};
#define File_Fetch_Start_value 0x64
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR File_Fetch_Stop = {0x65, 0x1, 0x0, 0x4, 0x2, 0x1, 0x2};
#define File_Fetch_Stop_value 0x65
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR Driver_Init_Start = {0x66, 0x1, 0x0, 0x4, 0x1, 0x2, 0x2};
#define Driver_Init_Start_value 0x66
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR Driver_Init_Stop = {0x67, 0x1, 0x0, 0x4, 0x2, 0x2, 0x2};
#define Driver_Init_Stop_value 0x67
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR Configuration_Load_Start = {0x68, 0x1, 0x0, 0x4, 0x1, 0x3, 0x2};
#define Configuration_Load_Start_value 0x68
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR Configuration_Load_Stop = {0x69, 0x1, 0x0, 0x4, 0x2, 0x3, 0x2};
#define Configuration_Load_Stop_value 0x69
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR File_Open_Start = {0x6a, 0x1, 0x0, 0x4, 0x1, 0x4, 0x2};
#define File_Open_Start_value 0x6a
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR File_Open_Stop = {0x6b, 0x1, 0x0, 0x4, 0x2, 0x4, 0x2};
#define File_Open_Stop_value 0x6b

//
// Note on Generate Code from Manifest Windows Vista and above
//
//Structures :  are handled as a size and pointer pairs. The macro for the event will have an extra 
//parameter for the size in bytes of the structure. Make sure that your structures have no extra padding.
//
//Strings: There are several cases that can be described in the manifest. For array of variable length 
//strings, the generated code will take the count of characters for the whole array as an input parameter. 
//
//SID No support for array of SIDs, the macro will take a pointer to the SID and use appropriate 
//GetLengthSid function to get the length.
//

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Globals 
//


//
// Event Enablement Bits
//

EXTERN_C __declspec(selectany) DECLSPEC_CACHEALIGN ULONG LazyCopyDriverEnableBits[1];
EXTERN_C __declspec(selectany) const ULONGLONG LazyCopyDriverKeywords[3] = {0x0, 0x8000000000000001, 0x2};
EXTERN_C __declspec(selectany) const UCHAR LazyCopyDriverLevels[3] = {4, 2, 4};
EXTERN_C __declspec(selectany) MCGEN_TRACE_CONTEXT LazyCopyDriverGuid_Context = {0, 0, 0, 0, 0, 0, 0, 0, 3, LazyCopyDriverEnableBits, LazyCopyDriverKeywords, LazyCopyDriverLevels};

EXTERN_C __declspec(selectany) REGHANDLE LazyCopyDriverHandle = (REGHANDLE)0;

#if !defined(McGenEventRegisterUnregister)
#define McGenEventRegisterUnregister
DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventRegister(
    _In_ LPCGUID ProviderId,
    _In_opt_ PETWENABLECALLBACK EnableCallback,
    _In_opt_ PVOID CallbackContext,
    _Inout_ PREGHANDLE RegHandle
    )
/*++

Routine Description:

    This function register the provider with ETW KERNEL mode.

Arguments:
    ProviderId - Provider Id to be register with ETW.

    EnableCallback - Callback to be used.

    CallbackContext - Context for this provider.

    RegHandle - Pointer to Registration handle.

Remarks:

    If the handle != NULL will return ERROR_SUCCESS

--*/
{
    ULONG Error;


    if (*RegHandle) {
        //
        // already registered
        //
        return STATUS_SUCCESS;
    }

    Error = EtwRegister( ProviderId, EnableCallback, CallbackContext, RegHandle); 

    return Error;
}


DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventUnregister(_Inout_ PREGHANDLE RegHandle)
/*++

Routine Description:

    Unregister from ETW KERNEL mode

Arguments:
            RegHandle this is the pointer to the provider context
Remarks:
            If Provider has not register RegHandle = NULL,
            return ERROR_SUCCESS
--*/
{
    ULONG Error;


    if(!(*RegHandle)) {
        //
        // Provider has not registerd
        //
        return STATUS_SUCCESS;
    }

    Error = EtwUnregister(*RegHandle); 
    *RegHandle = (REGHANDLE)0;
    
    return Error;
}
#endif
//
// Register with ETW Vista +
//
#ifndef EventRegisterLazyCopyDriver
#define EventRegisterLazyCopyDriver() McGenEventRegister(&LazyCopyDriverGuid, McGenControlCallbackV2, &LazyCopyDriverGuid_Context, &LazyCopyDriverHandle) 
#endif

//
// UnRegister with ETW
//
#ifndef EventUnregisterLazyCopyDriver
#define EventUnregisterLazyCopyDriver() McGenEventUnregister(&LazyCopyDriverHandle) 
#endif

//
// Enablement check macro for FileAccessedEvent
//

#define EventEnabledFileAccessedEvent() ((LazyCopyDriverEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for FileAccessedEvent
//
#define EventWriteFileAccessedEvent(Activity, Path, CreateOptions)\
        EventEnabledFileAccessedEvent() ?\
        Template_zx(LazyCopyDriverHandle, &FileAccessedEvent, Activity, Path, CreateOptions)\
        : STATUS_SUCCESS\

//
// Enablement check macro for FileFetchedEvent
//

#define EventEnabledFileFetchedEvent() ((LazyCopyDriverEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for FileFetchedEvent
//
#define EventWriteFileFetchedEvent(Activity, LocalPath, RemotePath, Size)\
        EventEnabledFileFetchedEvent() ?\
        Template_zzi(LazyCopyDriverHandle, &FileFetchedEvent, Activity, LocalPath, RemotePath, Size)\
        : STATUS_SUCCESS\

//
// Enablement check macro for FileNotFetchedEvent
//

#define EventEnabledFileNotFetchedEvent() ((LazyCopyDriverEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for FileNotFetchedEvent
//
#define EventWriteFileNotFetchedEvent(Activity, Path, RemoteRoot, Status)\
        EventEnabledFileNotFetchedEvent() ?\
        Template_zzi(LazyCopyDriverHandle, &FileNotFetchedEvent, Activity, Path, RemoteRoot, Status)\
        : STATUS_SUCCESS\

//
// Enablement check macro for File_Fetch_Start
//

#define EventEnabledFile_Fetch_Start() ((LazyCopyDriverEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for File_Fetch_Start
//
#define EventWriteFile_Fetch_Start(Activity)\
        EventEnabledFile_Fetch_Start() ?\
        TemplateEventDescriptor(LazyCopyDriverHandle, &File_Fetch_Start, Activity)\
        : STATUS_SUCCESS\

//
// Enablement check macro for File_Fetch_Stop
//

#define EventEnabledFile_Fetch_Stop() ((LazyCopyDriverEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for File_Fetch_Stop
//
#define EventWriteFile_Fetch_Stop(Activity)\
        EventEnabledFile_Fetch_Stop() ?\
        TemplateEventDescriptor(LazyCopyDriverHandle, &File_Fetch_Stop, Activity)\
        : STATUS_SUCCESS\

//
// Enablement check macro for Driver_Init_Start
//

#define EventEnabledDriver_Init_Start() ((LazyCopyDriverEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for Driver_Init_Start
//
#define EventWriteDriver_Init_Start(Activity)\
        EventEnabledDriver_Init_Start() ?\
        TemplateEventDescriptor(LazyCopyDriverHandle, &Driver_Init_Start, Activity)\
        : STATUS_SUCCESS\

//
// Enablement check macro for Driver_Init_Stop
//

#define EventEnabledDriver_Init_Stop() ((LazyCopyDriverEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for Driver_Init_Stop
//
#define EventWriteDriver_Init_Stop(Activity)\
        EventEnabledDriver_Init_Stop() ?\
        TemplateEventDescriptor(LazyCopyDriverHandle, &Driver_Init_Stop, Activity)\
        : STATUS_SUCCESS\

//
// Enablement check macro for Configuration_Load_Start
//

#define EventEnabledConfiguration_Load_Start() ((LazyCopyDriverEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for Configuration_Load_Start
//
#define EventWriteConfiguration_Load_Start(Activity)\
        EventEnabledConfiguration_Load_Start() ?\
        TemplateEventDescriptor(LazyCopyDriverHandle, &Configuration_Load_Start, Activity)\
        : STATUS_SUCCESS\

//
// Enablement check macro for Configuration_Load_Stop
//

#define EventEnabledConfiguration_Load_Stop() ((LazyCopyDriverEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for Configuration_Load_Stop
//
#define EventWriteConfiguration_Load_Stop(Activity)\
        EventEnabledConfiguration_Load_Stop() ?\
        TemplateEventDescriptor(LazyCopyDriverHandle, &Configuration_Load_Stop, Activity)\
        : STATUS_SUCCESS\

//
// Enablement check macro for File_Open_Start
//

#define EventEnabledFile_Open_Start() ((LazyCopyDriverEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for File_Open_Start
//
#define EventWriteFile_Open_Start(Activity, Path)\
        EventEnabledFile_Open_Start() ?\
        Template_z(LazyCopyDriverHandle, &File_Open_Start, Activity, Path)\
        : STATUS_SUCCESS\

//
// Enablement check macro for File_Open_Stop
//

#define EventEnabledFile_Open_Stop() ((LazyCopyDriverEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for File_Open_Stop
//
#define EventWriteFile_Open_Stop(Activity)\
        EventEnabledFile_Open_Stop() ?\
        TemplateEventDescriptor(LazyCopyDriverHandle, &File_Open_Stop, Activity)\
        : STATUS_SUCCESS\

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION


//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Template Functions 
//
//
//Template from manifest : FileAccessTemplate
//
#ifndef Template_zx_def
#define Template_zx_def
ETW_INLINE
ULONG
Template_zx(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ LPCGUID Activity,
    _In_opt_ PCWSTR  _Arg0,
    _In_ unsigned __int64  _Arg1
    )
{
#define ARGUMENT_COUNT_zx 2

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_zx];

    EventDataDescCreate(&EventData[0], 
                        (_Arg0 != NULL) ? _Arg0 : L"NULL",
                        (_Arg0 != NULL) ? (ULONG)((wcslen(_Arg0) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[1], &_Arg1, sizeof(unsigned __int64)  );

    return EtwWrite(RegHandle, Descriptor, Activity, ARGUMENT_COUNT_zx, EventData);
}
#endif

//
//Template from manifest : FileFetchTemplate
//
#ifndef Template_zzi_def
#define Template_zzi_def
ETW_INLINE
ULONG
Template_zzi(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ LPCGUID Activity,
    _In_opt_ PCWSTR  _Arg0,
    _In_opt_ PCWSTR  _Arg1,
    _In_ signed __int64  _Arg2
    )
{
#define ARGUMENT_COUNT_zzi 3

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_zzi];

    EventDataDescCreate(&EventData[0], 
                        (_Arg0 != NULL) ? _Arg0 : L"NULL",
                        (_Arg0 != NULL) ? (ULONG)((wcslen(_Arg0) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[1], 
                        (_Arg1 != NULL) ? _Arg1 : L"NULL",
                        (_Arg1 != NULL) ? (ULONG)((wcslen(_Arg1) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[2], &_Arg2, sizeof(signed __int64)  );

    return EtwWrite(RegHandle, Descriptor, Activity, ARGUMENT_COUNT_zzi, EventData);
}
#endif

//
//Template from manifest : (null)
//
#ifndef TemplateEventDescriptor_def
#define TemplateEventDescriptor_def


ETW_INLINE
ULONG
TemplateEventDescriptor(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ LPCGUID Activity
    )
{
    return EtwWrite(RegHandle, Descriptor, Activity, 0, NULL);
}
#endif

//
//Template from manifest : FileOpenTemplate
//
#ifndef Template_z_def
#define Template_z_def
ETW_INLINE
ULONG
Template_z(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ LPCGUID Activity,
    _In_opt_ PCWSTR  _Arg0
    )
{
#define ARGUMENT_COUNT_z 1

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_z];

    EventDataDescCreate(&EventData[0], 
                        (_Arg0 != NULL) ? _Arg0 : L"NULL",
                        (_Arg0 != NULL) ? (ULONG)((wcslen(_Arg0) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    return EtwWrite(RegHandle, Descriptor, Activity, ARGUMENT_COUNT_z, EventData);
}
#endif

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION

#if defined(__cplusplus)
};
#endif

#define MSG_LazyCopyDriver_event_3_message   0x00000003L
#define MSG_opcode_Start                     0x30000001L
#define MSG_opcode_Stop                      0x30000002L
#define MSG_level_Error                      0x50000002L
#define MSG_level_Informational              0x50000004L
#define MSG_task_None                        0x70000000L
#define MSG_channel_System                   0x90000001L
#define MSG_LazyCopyDriver_event_0_message   0xB0010001L
#define MSG_LazyCopyDriver_event_2_message   0xB0010002L
