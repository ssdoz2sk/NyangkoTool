#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Uefi.h>


#define MAX_STRING_SIZE 80

#define UI_MSG_PAUSE L"Press any key to continue"

STATIC VOID
OutNc(UINT8 N, CHAR16 C)
{
    CHAR16  Str[2] = L" ";
    Str[0] = C;

    while (N-- > 0) {
	    gST->ConOut->OutputString(gST->ConOut, Str);
    }
}

// STATIC VOID
// Nblank(UINT8 N)
// {
//     OutNc(N, L' ');
// }

STATIC VOID
FillNs(UINT8 N, CONST CHAR16 *Str)
{
    CHAR16  S[2] = {L'\0'};
    CHAR16  *Buffer;
    UINTN   Index = 0;

    if (Str == NULL) {
        Buffer = AllocateCopyPool(StrSize(L" "), L" ");
    } else {
        Buffer = AllocateCopyPool(StrSize(Str), Str);
    }

    while (N > 0) {
        S[0] = Buffer[Index];
        gST->ConOut->OutputString(gST->ConOut, S);
        N--;
	    Index++;
        if (Index == StrLen(Buffer)) {
            Index = 0;
        }
    }
    if (N > 0) {
	    OutNc(N, L' ');
    }
    FreePool(Buffer);
}

#if 0
STATIC VOID
ClrToEol ()
{
    UINT8 X = (UINT8)gST->ConOut->Mode->CursorColumn;
    UINT8 N = (UINT8)DisplayCol - X;

    FillNs (N, NULL);
}

VOID
ShowMsg (CHAR16 *Msg)
{
    UINT8 Col = (UINT8)DisplayCol;
    
    gST->ConOut->SetCursorPosition(gST->ConOut, Col, DisplayRow);
    ClrToEol();

    if (!Msg) {
        Col -= (UINT8)StrLen(UI_MSG_PAUSE);
        gST->ConOut->SetCursorPosition(gST->ConOut, Col, DisplayRow);
        gST->ConOut->OutputString(gST->ConOut, UI_MSG_PAUSE);
    } else {
        Col -= (UINT8)StrLen(Msg);
        gST->ConOut->SetCursorPosition(gST->ConOut, Col, DisplayRow);
        gST->ConOut->OutputString(gST->ConOut, Msg);
    }
}
#endif

BOOLEAN
IsAtDashOffset (
    UINT8   Offset
) {
    UINT8           DashOffset[]    = {8, 13, 18, 23};
    UINT8           Index;

    for (Index = 0; Index < sizeof(DashOffset) / sizeof(UINT8); Index += 1) {
        if (DashOffset[Index] == Offset) {
            return TRUE;
        }
    }
    return FALSE;
}

STATIC
BOOLEAN
IsGuidStringValid (
    CHAR16  *GuidStrBuffer,
    BOOLEAN AllowSpace
) {
    UINT8   Index;

    ASSERT(StrLen(GuidStrBuffer) == 36);

    for (Index = 0; Index < StrLen(GuidStrBuffer); Index += 1) {
        if (IsAtDashOffset(Index)) {
            if (L'-' == GuidStrBuffer[Index]) {
                continue;
            } else {
                return FALSE;
            }
        }
        if ((L'0' <= GuidStrBuffer[Index] && GuidStrBuffer[Index] <= L'9') ||
            (L'a' <= GuidStrBuffer[Index] && GuidStrBuffer[Index] <= L'f') ||
            (L'A' <= GuidStrBuffer[Index] && GuidStrBuffer[Index] <= L'F')) {
            continue;
        } else if (AllowSpace && L' ' == GuidStrBuffer[Index] ) {
            continue;
        }
        return FALSE;
    }
    return TRUE;
}

STATIC
UINT8
FirstSpaceInGuid (
    CHAR16  *GuidStrBuffer
) {
    UINT8   Index;

    ASSERT(StrLen(GuidStrBuffer) == 36);

    for (Index = 0; Index < StrLen(GuidStrBuffer); Index += 1) {
        if (L' ' == GuidStrBuffer[Index] ) {
            return Index;
        }
    }
    return Index;
}


EFI_STATUS
EditBox (
    CHAR16  **Buffer, 
    UINTN   Length,
    UINT16  CursorX,
    UINT16  CursorY,
    CHAR16  *DefaultString
) {
    EFI_STATUS      Status;
    CHAR16          Buf[MAX_STRING_SIZE] = L"";
    BOOLEAN         Abort                = FALSE;
    UINT8           CursorOffset         = 0;
    UINT8           BufSize              = 0;
    UINT8           Index                = 0;
    EFI_INPUT_KEY   Key;

    if (Length > MAX_STRING_SIZE) {
        Length = MAX_STRING_SIZE;
    }

    if (DefaultString && *DefaultString) {
        StrnCpyS(Buf, MAX_STRING_SIZE, DefaultString, StrLen(DefaultString));
        CursorOffset = (UINT8)StrLen(Buf);
        BufSize = CursorOffset;
    }

    gST->ConOut->SetCursorPosition(gST->ConOut, CursorX, CursorY);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BRIGHT | EFI_BACKGROUND_LIGHTGRAY);
    FillNs((UINT8)Length, NULL);
    gST->ConOut->SetCursorPosition(gST->ConOut, CursorX, CursorY);
    gST->ConOut->OutputString(gST->ConOut, Buf);
    gST->ConOut->SetCursorPosition(gST->ConOut, CursorX + CursorOffset, CursorY);

    while (!Abort) {        
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
        if (Status == EFI_DEVICE_ERROR) {
            DEBUG((DEBUG_ERROR, "The keystroke information was not returned due to hardware errors."));
            break;
        }
        if (EFI_ERROR (Status))  {
            continue;
        }

        switch (Key.ScanCode) {
            case SCAN_HOME:
                CursorOffset = 0;
                break;
            case SCAN_END:
                CursorOffset = BufSize;
                break;
            case SCAN_LEFT:
                CursorOffset = CursorOffset > 0 ? CursorOffset - 1 : CursorOffset;
                break;
            case SCAN_RIGHT:
                CursorOffset = CursorOffset < BufSize ? CursorOffset + 1 : BufSize;
                break;
            case SCAN_DELETE:
                if (CursorOffset < BufSize) {
                    Index = CursorOffset;
                    while (Index < BufSize - 1) {
                        Buf[Index] = Buf[Index+1];
                        Index ++;
                    }
                    BufSize -= 1;
                    Buf[BufSize] = CHAR_NULL;    
                    break;
                }
            case SCAN_ESC:
                *Buffer = NULL;
                return EFI_ABORTED;
            case SCAN_NULL: {
                if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
                    Abort = TRUE;
                } else if (Key.UnicodeChar == CHAR_BACKSPACE) {
                    if (CursorOffset > 0) {
                        Index = CursorOffset - 1;
                        while (Index < BufSize - 1) {
                            Buf[Index] = Buf[Index+1];
                            Index ++;
                        }
                        CursorOffset -= 1;
                        BufSize -= 1;
                        Buf[BufSize] = CHAR_NULL;
                    }
                } else if (L' ' <= Key.UnicodeChar && Key.UnicodeChar <= L'~') {
                    Index = BufSize;
                    while (CursorOffset < Index) {
                        Buf[Index] = Buf[Index - 1];
                        Index --; 
                    }
                    Buf[CursorOffset] = Key.UnicodeChar;
                    CursorOffset += 1;
                    BufSize += 1;
                    Buf[BufSize] = CHAR_NULL;
                }
                break;
            }
        }
               
        gST->ConOut->SetCursorPosition(gST->ConOut, CursorX, CursorY);
        FillNs(BufSize+1, NULL);
        gST->ConOut->SetCursorPosition(gST->ConOut, CursorX, CursorY);
        gST->ConOut->OutputString(gST->ConOut, Buf);
        gST->ConOut->SetCursorPosition(gST->ConOut, CursorX + CursorOffset, CursorY);
    }
    
    *Buffer = AllocateCopyPool(StrSize(Buf), Buf);
    DEBUG((DEBUG_INFO, "buf: %s\n", Buf));

    return EFI_SUCCESS;
}

EFI_STATUS
GuidEditBox (
    CHAR16  **GuidStr,
    CHAR16  *Length,
    UINT16  CursorX,
    UINT16  CursorY,
    CHAR16  *DefaultGuidStr
) {
    CHAR16          GuidStrBuffer[] = L"        -    -    -    -            ";
    BOOLEAN         IsValidGuid     = TRUE;
    EFI_STATUS      Status;
    BOOLEAN         Abort           = FALSE;
    UINT8           CursorOffset    = 0;
    UINT8           LastBufferOffset= 0;
    UINT8           Index           = 0;
    EFI_INPUT_KEY   Key;

    IsValidGuid = FALSE;

    if (DefaultGuidStr && *DefaultGuidStr) {
        if (IsGuidStringValid(DefaultGuidStr, TRUE)) {
            // Check Default Guid String
            StrnCpyS(GuidStrBuffer, StrLen(GuidStrBuffer), DefaultGuidStr, StrLen(DefaultGuidStr));
            CursorOffset = FirstSpaceInGuid(GuidStrBuffer);
            LastBufferOffset = CursorOffset;
        }
        DEBUG((DEBUG_INFO, "GuidStrBuffer: \"%s\", DefaultGuidStr: \"%s\"\n", GuidStrBuffer, DefaultGuidStr));
        DEBUG((DEBUG_INFO, "GuidEditBox Check: is valid guid ?: %d\n", IsValidGuid));
    }

    gST->ConOut->SetCursorPosition(gST->ConOut, CursorX, CursorY);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BRIGHT | EFI_BACKGROUND_LIGHTGRAY);
    gST->ConOut->OutputString(gST->ConOut, GuidStrBuffer);
    gST->ConOut->SetCursorPosition(gST->ConOut, CursorX + CursorOffset, CursorY);

    gST->ConOut->EnableCursor (gST->ConOut, TRUE);

    while (!Abort) {        
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
        if (Status == EFI_DEVICE_ERROR) {
            DEBUG((DEBUG_ERROR, "The keystroke information was not returned due to hardware errors."));
            break;
        }
        if (EFI_ERROR (Status))  {
            continue;
        }

        switch (Key.ScanCode) {
            case SCAN_HOME:
                CursorOffset = 0;
                break;
            case SCAN_END:
                CursorOffset = LastBufferOffset;
                break;
            case SCAN_LEFT:
                CursorOffset = CursorOffset > 0 ? CursorOffset - 1 : CursorOffset;
                if (IsAtDashOffset(CursorOffset)) {
                    CursorOffset -= 1;
                }
                break;
            case SCAN_RIGHT:
                CursorOffset = CursorOffset < LastBufferOffset ? CursorOffset + 1 : LastBufferOffset;
                if (IsAtDashOffset(CursorOffset)) {
                    CursorOffset += 1;
                }
                break;
            case SCAN_DELETE:
                if (CursorOffset >= LastBufferOffset) {
                    break;
                }
                Index = CursorOffset;

                while (Index < LastBufferOffset - 1) {
                    if (IsAtDashOffset(Index + 1)) {
                        GuidStrBuffer[Index] = GuidStrBuffer[Index + 2];
                        Index += 2;
                    } else {
                        GuidStrBuffer[Index] = GuidStrBuffer[Index+1];
                        Index ++;
                    }
                }
                if (IsAtDashOffset(LastBufferOffset - 1)) {
                    LastBufferOffset -= 2;
                } else {
                    LastBufferOffset -= 1;
                }
                GuidStrBuffer[LastBufferOffset] = L' ';
                break;
            case SCAN_ESC:
                gST->ConOut->EnableCursor (gST->ConOut, FALSE);
                return EFI_ABORTED;
            case SCAN_NULL: {
                if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
                    Abort = TRUE;
                } else if (Key.UnicodeChar == CHAR_BACKSPACE) {
                    if (CursorOffset > 0) {
                        if (IsAtDashOffset(CursorOffset - 1)) {
                            Index = CursorOffset - 2;
                        } else {
                            Index = CursorOffset - 1;
                        }
                        while (Index < LastBufferOffset - 1) {
                            if (IsAtDashOffset(Index + 1)) {
                                GuidStrBuffer[Index] = GuidStrBuffer[Index + 2];
                                Index += 2;
                            } else {
                                GuidStrBuffer[Index] = GuidStrBuffer[Index+1];
                                Index ++;
                            }
                        }
                        if (IsAtDashOffset(LastBufferOffset - 1)) {
                            LastBufferOffset -= 2;
                        } else {
                            LastBufferOffset -= 1;
                        }
                        GuidStrBuffer[LastBufferOffset] = L' ';

                        if (IsAtDashOffset(CursorOffset - 1)) {
                            CursorOffset -= 2;
                        } else {
                            CursorOffset -= 1;
                        }
                    }
                    DEBUG((DEBUG_INFO, "%2X, %2X\n", CursorOffset, LastBufferOffset));
                    DEBUG((DEBUG_INFO, "01234567-9ABC-EF01-3456-89ABCDEF012C\n"));
                    DEBUG((DEBUG_INFO, "%s\n\n", GuidStrBuffer));
                } else if ((L'0' <= Key.UnicodeChar && Key.UnicodeChar <= L'9') ||
                           (L'a' <= Key.UnicodeChar && Key.UnicodeChar <= L'f') ||
                           (L'A' <= Key.UnicodeChar && Key.UnicodeChar <= L'F'))  {
                    if (LastBufferOffset == StrLen(GuidStrBuffer)){
                        break;
                    } else if (IsAtDashOffset(LastBufferOffset)) {
                        Index = LastBufferOffset + 1;
                    } else {
                        Index = LastBufferOffset;
                    }
                    while (CursorOffset < Index) {
                        if (IsAtDashOffset(Index - 1)) {
                            GuidStrBuffer[Index] = GuidStrBuffer[Index - 2];
                            Index -= 2;
                        } else {
                            GuidStrBuffer[Index] = GuidStrBuffer[Index - 1];
                            Index --;
                        } 
                    }
                    if (IsAtDashOffset(CursorOffset)) {
                        CursorOffset += 1;
                        LastBufferOffset += 1;
                    }
                    // To upper case
                    if (L'a' <= Key.UnicodeChar && Key.UnicodeChar <= L'f') {
                        GuidStrBuffer[CursorOffset] = Key.UnicodeChar - L'a' + L'A';
                    } else {
                        GuidStrBuffer[CursorOffset] = Key.UnicodeChar;
                    }
                    if (IsAtDashOffset(CursorOffset + 1)) {
                        CursorOffset += 2;
                    } else {
                        CursorOffset += 1;
                    }
                    if (IsAtDashOffset(LastBufferOffset + 1)) {
                        LastBufferOffset += 2;
                    } else {
                        LastBufferOffset += 1;
                    }
                }
                break;
            }
        }
               
        gST->ConOut->SetCursorPosition(gST->ConOut, CursorX, CursorY);
        FillNs(LastBufferOffset+1, NULL);
        gST->ConOut->SetCursorPosition(gST->ConOut, CursorX, CursorY);
        gST->ConOut->OutputString(gST->ConOut, GuidStrBuffer);
        gST->ConOut->SetCursorPosition(gST->ConOut, CursorX + CursorOffset, CursorY);
    }
    

    gST->ConOut->EnableCursor (gST->ConOut, FALSE);
    *GuidStr = AllocateCopyPool(StrSize(GuidStrBuffer), GuidStrBuffer);
    *Length  = LastBufferOffset; 
    DEBUG((DEBUG_INFO, "GuidStrBuffer: %s\n", GuidStrBuffer));

    return EFI_SUCCESS;
}


/**
    Prints an EFI_GUID into specified Buffer.

    @param[in]     Guid          Pointer to GUID to print.
    @param[in]     Buffer        Buffer to print Guid into.
    @param[in]     BufferSize    Size of Buffer.

    @retval    Number of characters printed.
**/

UINTN
GuidToString (
    IN  EFI_GUID  *Guid,
    IN  CHAR16    *Buffer,
    IN  UINTN     BufferSize
)
{
    return UnicodeSPrint (Buffer,
                          BufferSize,
                          L"%g",
                          Guid);
}

/**
    Compares fist N character of two GUIDs.

    String consists of 36 characters, as follows:
        aabbccdd-eeff-gghh-iijj-kkllmmnnoopp

    In Memory:
        [dd cc bb aa ff ee hh gg] [ii jj kk ll mm nn oo pp]

    This function compares Guid1 to Guid2.  If the GUIDs are identical then TRUE is returned.
    If there are any bit differences in the two GUIDs, then FALSE is returned.

    If Guid1 is NULL, then ASSERT().
    If Guid2 is NULL, then ASSERT().

    @param  Guid1       A pointer to a 128 bit GUID.
    @param  Guid2       A pointer to a 128 bit GUID.
    @param  Length      The fist Length offset would be compare.

    @retval TRUE        Guid1 and Guid2 are identical.
    @retval FALSE       Guid1 and Guid2 are not identical.

**/
BOOLEAN
EFIAPI
CompareNGuid (
    IN GUID  *Guid1,
    IN GUID  *Guid2,
    IN CONST UINT8 Length
)
{
    CHAR16  *GuidStr1 = AllocateZeroPool(100);
    CHAR16  *GuidStr2 = AllocateZeroPool(100);
    BOOLEAN IsIdentical;
    UINT8   LengthWithDash;

    GuidToString (Guid1, GuidStr1, 100);
    GuidToString (Guid2, GuidStr2, 100);

    if (Length > 20) {
        LengthWithDash = Length + 4;
    } else if (Length > 16) {
        LengthWithDash = Length + 3;
    } else if (Length > 12) {
        LengthWithDash = Length + 2;
    } else if (Length > 8) {
        LengthWithDash = Length + 1;
    } else {
        LengthWithDash = Length;
    }

    IsIdentical = StrnCmp(GuidStr1, GuidStr2, LengthWithDash) == 0;

    FreePool(GuidStr1);
    FreePool(GuidStr2);

    return IsIdentical;
}
