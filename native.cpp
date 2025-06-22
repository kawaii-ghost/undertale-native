#include <Veil.h>

typedef struct _KEYBOARD_INPUT_DATA {
    USHORT UnitId;
    USHORT MakeCode;
    USHORT Flags;
    USHORT Reserved;
    ULONG ExtraInformation;
} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;

static ULONG RtlDrawText(const PCWSTR Message);
static VOID RtlSleep(ULONG Milliseconds);
static VOID DisplayDelay(const PCWSTR Message);

VOID NtProcessStartup(PPEB Peb) {
    DisplayDelay(L "It's you!");
    RtlSleep(3000); // 3 seconds

    DisplayDelay(L "Despite everything, it's still you.");
    RtlSleep(5000); // 5 seconds

    DisplayDelay(L "Press any key to continue...");

    // Read keyboard input
    HANDLE Keyboard, Event;
    UNICODE_STRING KeyboardPath = RTL_CONSTANT_STRING(L "\\Device\\KeyboardClass0");
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyboardPath,
                               OBJ_CASE_INSENSITIVE,
                               nullptr,
                               nullptr);

    Status = NtCreateFile(&Keyboard,
                          SYNCHRONIZE | GENERIC_READ | FILE_READ_ATTRIBUTES,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          nullptr,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OPEN,
                          FILE_DIRECTORY_FILE,
                          nullptr,
                          0);

    InitializeObjectAttributes(&ObjectAttributes,
                               nullptr,
                               0,
                               nullptr,
                               nullptr);

    NtCreateEvent(&Event,
                  &EVENT_ALL_ACCESS,
                  &ObjectAttributes,
                  &SynchronizationEvent,
                  FALSE);

    KEYBOARD_INPUT_DATA KeyboardData;
    LARGE_INTEGER ByteOffset;
    Status = NtReadFile(Keyboard,
                        Event,
                        nullptr,
                        nullptr,
                        &IoStatusBlock,
                        (PVOID)&KeyboardData,
                        sizeof(KeyboardData),
                        &ByteOffset,
                        nullptr);

    if (Status == STATUS_PENDING)
        NtWaitForSingleObject(Event, FALSE, nullptr);

    NtClose(Keyboard);
    NtClose(Event);

    RtlExitUserProcess(STATUS_SUCCESS);
}

static VOID DisplayDelay(PCWSTR Message) {
    WCHAR wstr[255] = {0};
    for (size_t i = 0; (i < 255) && (Message[i] != '\0'); i++) {
        wstr[i] = Message[i];
        RtlDrawText(wstr);
        if (wstr[i] == ',')
            RtlSleep(300);
        else
            RtlSleep(100);
    }
    return;
}

static ULONG RtlDrawText(PCWSTR Message) {
    UNICODE_STRING Text;
    RtlInitUnicodeString( & Text, Message);
    return RtlNtStatusToDosError(NtDrawText( & Text));
}

static VOID RtlSleep(ULONG Milliseconds) {
    LARGE_INTEGER Timeout = {
        .QuadPart = Milliseconds * -10000 LL
    };
    RtlDelayExecution(FALSE, & Timeout);
}