#include "reflector.h"

SIZE_T s_TotalSize = 0;
ZydisDecoder Decoder = { 0 };

PVOID
RINAPI
RinAllocateMemory(
    __in ULONG Length
)
{
    PVOID Pointer = NULL;

    if (0 != Length) {
        Pointer = VirtualAlloc(NULL, Length, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }

    return Pointer;
}

VOID
RINAPI
RinFreeMemory(
    __in PVOID Pointer
)
{
    VirtualFree(Pointer, 0, MEM_RELEASE);
}

BOOLEAN
RINAPI
InitializeReflector(
    VOID
)
{
    ZyanStatus Status = ZYAN_STATUS_SUCCESS;

    Status = ZydisDecoderInit(&Decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

    return ZYAN_SUCCESS(Status);
}

ULONG
RINAPI
GetAnalyzerCount(
    __in PUCHAR Address,
    __in ULONG Length
)
{
    ZyanStatus Status = ZYAN_STATUS_SUCCESS;
    ZydisDecodedInstruction Instruction = { 0 };
    ZydisDecodedOperand Operands[ZYDIS_MAX_OPERAND_COUNT] = { 0 };
    ZyanUSize Offset = 0;
    ULONG Count = 0;

    while (FALSE != ZYAN_SUCCESS(Status)) {
        Status = ZydisDecoderDecodeFull(
            &Decoder,
            Address + Offset,
            Length - Offset,
            &Instruction,
            Operands,
            ZYDIS_MAX_OPERAND_COUNT,
            0);

        if (FALSE != ZYAN_SUCCESS(Status)) {
            Count += 1;
            Offset += Instruction.length;
        }
    }

    return Count;
}

ULONG
RINAPI
GetCodeLength(
	__in PUCHAR Address,
	__in ULONG Length
)
{
	ZyanStatus Status = ZYAN_STATUS_SUCCESS;
	ZydisDecodedInstruction Instruction = { 0 };
	ZydisDecodedOperand Operands[ZYDIS_MAX_OPERAND_COUNT] = { 0 };
	ZyanUSize Offset = 0;


	Status = ZydisDecoderDecodeFull(
		&Decoder,
		Address + Offset,
		Length - Offset,
		&Instruction,
		Operands,
		ZYDIS_MAX_OPERAND_COUNT,
		0);

	if (FALSE != ZYAN_SUCCESS(Status)) {
		return Instruction.length;
	}
	else
	{
		return 0;
	}

}


PANALYZER
RINAPI
BuildAnalyzerTable(
    __in PUCHAR Address,
    __in ULONG Count
)
{
    ZyanStatus Status = ZYAN_STATUS_SUCCESS;
    ZyanUSize Offset = 0;
    PANALYZER Table = NULL;
    ULONG Index = 0;

    if (0 != Count) {
        Table = RinAllocateMemory(sizeof(ANALYZER) * Count);

        if (NULL != Table) {

            for (Index = 0; Index < Count; Index++) {
                Status = ZydisDecoderDecodeFull(
                    &Decoder,
                    Address + Offset,
                    ZYDIS_MAX_INSTRUCTION_LENGTH,
                    &Table[Index].Decoded.Instruction,
                    Table[Index].Decoded.Operands,
                    ZYDIS_MAX_OPERAND_COUNT,
                    0);

                if (FALSE == ZYAN_SUCCESS(Status)) {
                    break;
                }

                Status = ZydisEncoderDecodedInstructionToEncoderRequest(
                    &Table[Index].Decoded.Instruction,
                    Table[Index].Decoded.Operands,
                    Table[Index].Decoded.Instruction.operand_count_visible,
                    &Table[Index].Encoder.Request);

                if (FALSE == ZYAN_SUCCESS(Status)) {
                    break;
                }

                Table[Index].Decoded.Address = Address + Offset;
                Table[Index].Decoded.Length = Table[Index].Decoded.Instruction.length;
                Offset += Table[Index].Decoded.Length;
            }

            if (Index == Count) {
                return Table;
            }
        }
    }

    return NULL;
}

VOID
RINAPI
CleanAnalyzerTable(
    __in PANALYZER Table
)
{
    RinFreeMemory(Table);
}

BOOLEAN
RINAPI
BuildBranchType(
    __in PANALYZER Analyzer
)
{
    switch (Analyzer->Decoded.Instruction.mnemonic) {
    case ZYDIS_MNEMONIC_JB:
    case ZYDIS_MNEMONIC_JBE:
    case ZYDIS_MNEMONIC_JCXZ:
    case ZYDIS_MNEMONIC_JECXZ:
    case ZYDIS_MNEMONIC_JKNZD:
    case ZYDIS_MNEMONIC_JKZD:
    case ZYDIS_MNEMONIC_JL:
    case ZYDIS_MNEMONIC_JLE:
    case ZYDIS_MNEMONIC_JNB:
    case ZYDIS_MNEMONIC_JNBE:
    case ZYDIS_MNEMONIC_JNL:
    case ZYDIS_MNEMONIC_JNLE:
    case ZYDIS_MNEMONIC_JNO:
    case ZYDIS_MNEMONIC_JNP:
    case ZYDIS_MNEMONIC_JNS:
    case ZYDIS_MNEMONIC_JNZ:
    case ZYDIS_MNEMONIC_JO:
    case ZYDIS_MNEMONIC_JP:
    case ZYDIS_MNEMONIC_JRCXZ:
    case ZYDIS_MNEMONIC_JS:
    case ZYDIS_MNEMONIC_JZ:
        Analyzer->Repair.Flags = REPAIR_JCC;
        break;
    case ZYDIS_MNEMONIC_JMP:
        Analyzer->Repair.Flags = REPAIR_JMP;
        break;
    case ZYDIS_MNEMONIC_CALL:
        Analyzer->Repair.Flags = REPAIR_CALL;
        break;
    default:
        Analyzer->Repair.Flags = REPAIR_NONE;
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
RINAPI
BuildBranchAnalyzerId(
    __in PANALYZER Table,
    __in ULONG Count,
    __in PANALYZER Analyzer
)
{
    ULONG Index = 0;

    for (Index = 0; Index < Count; Index++) {
        if (Table[Index].Decoded.Address ==
            Analyzer->Decoded.Address +
            Analyzer->Decoded.Length +
            Analyzer->Decoded.Instruction.raw.imm->value.s) {
            Analyzer->Repair.Index = Index;
            return TRUE;
        }
    }

    return FALSE;
}

VOID
RINAPI
BuildAllRepairInfo(
    __in PANALYZER Table,
    __in ULONG Count
)
{
    ULONG Index = 0;
    PANALYZER Analyzer = NULL;

    for (Index = 0; Index < Count; Index++) {
        Analyzer = &Table[Index];

        if (FALSE != BuildBranchType(Analyzer)) {
            if (REPAIR_NONE != Analyzer->Repair.Flags) {
                if (ZYDIS_ATTRIB_HAS_MODRM & Analyzer->Decoded.Instruction.attributes) {
                    if (0 == Analyzer->Decoded.Instruction.raw.modrm.mod &&
                        5 == Analyzer->Decoded.Instruction.raw.modrm.rm) {
                        Analyzer->Repair.Address =
                            Analyzer->Decoded.Address +
                            Analyzer->Decoded.Length +
                            Analyzer->Decoded.Instruction.raw.disp.value;
                        Analyzer->Repair.Flags |= BRANCH_IMP;
                    }
                    else {
                        Analyzer->Repair.Flags = REPAIR_NONE;
                    }
                }
                else {
                    if (FALSE != BuildBranchAnalyzerId(Table, Count, Analyzer)) {
                        Analyzer->Repair.Flags |= BRANCH_LOCAL;
                    }
                    else {
                        Analyzer->Repair.Address =
                            Analyzer->Decoded.Address +
                            Analyzer->Decoded.Length +
                            Analyzer->Decoded.Instruction.raw.imm->value.s;
                        Analyzer->Repair.Flags |= BRANCH_REMOTE;
                    }
                }
            }
        }
        else {
            if (ZYDIS_ATTRIB_HAS_MODRM & Analyzer->Decoded.Instruction.attributes) {
                if (0 == Analyzer->Decoded.Instruction.raw.modrm.mod &&
                    5 == Analyzer->Decoded.Instruction.raw.modrm.rm) {
                    Analyzer->Repair.Address =
                        Analyzer->Decoded.Address +
                        Analyzer->Decoded.Length +
                        Analyzer->Decoded.Instruction.raw.disp.value;

                    Analyzer->Repair.Flags = REPAIR_REL;
                }
                else {
                    Analyzer->Repair.Flags = REPAIR_NONE;
                }
            }
            else {
                Analyzer->Repair.Flags = REPAIR_NONE;
            }
        }
    }
}

ULONG
RINAPI
CreateRegisterBitmap(
    __in PANALYZER Analyzer
)
{
    ULONG Index = 0;
    ULONG Bitmap = 0;

    for (Index = 0; Index < Analyzer->Decoded.Instruction.operand_count; Index++) {
        if (ZYDIS_OPERAND_TYPE_REGISTER == Analyzer->Decoded.Operands[Index].type) {
            switch (Analyzer->Decoded.Operands[Index].reg.value) {
            case ZYDIS_REGISTER_AL:
            case ZYDIS_REGISTER_AH:
            case ZYDIS_REGISTER_AX:
            case ZYDIS_REGISTER_EAX:
            case ZYDIS_REGISTER_RAX:
                Bitmap |= REGISTER_BITMAP_EAX;
                break;
            case ZYDIS_REGISTER_CL:
            case ZYDIS_REGISTER_CH:
            case ZYDIS_REGISTER_CX:
            case ZYDIS_REGISTER_ECX:
            case ZYDIS_REGISTER_RCX:
                Bitmap |= REGISTER_BITMAP_ECX;
                break;
            case ZYDIS_REGISTER_DL:
            case ZYDIS_REGISTER_DH:
            case ZYDIS_REGISTER_DX:
            case ZYDIS_REGISTER_EDX:
            case ZYDIS_REGISTER_RDX:
                Bitmap |= REGISTER_BITMAP_EDX;
                break;
            case ZYDIS_REGISTER_BL:
            case ZYDIS_REGISTER_BH:
            case ZYDIS_REGISTER_BX:
            case ZYDIS_REGISTER_EBX:
            case ZYDIS_REGISTER_RBX:
                Bitmap |= REGISTER_BITMAP_EBX;
                break;
            case ZYDIS_REGISTER_SIL:
            case ZYDIS_REGISTER_SI:
            case ZYDIS_REGISTER_ESI:
            case ZYDIS_REGISTER_RSI:
                Bitmap |= REGISTER_BITMAP_ESI;
                break;
            case ZYDIS_REGISTER_DIL:
            case ZYDIS_REGISTER_DI:
            case ZYDIS_REGISTER_EDI:
            case ZYDIS_REGISTER_RDI:
                Bitmap |= REGISTER_BITMAP_EDI;
                break;
            case ZYDIS_REGISTER_BPL:
            case ZYDIS_REGISTER_BP:
            case ZYDIS_REGISTER_EBP:
            case ZYDIS_REGISTER_RBP:
                Bitmap |= REGISTER_BITMAP_EBP;
                break;
            }
        }
    }

    return Bitmap;
}

ZydisRegister
RINAPI
SelectIdleRegister(
    __in PANALYZER Analyzer
)
{
    ULONG Index = 0;
    ULONG Bitmap = 0;

    Bitmap = CreateRegisterBitmap(Analyzer);

    for (Index = 0; Index < 7; Index++) {
        if (0 == (REGISTER_BITMAP_EAX & Bitmap)) {
            return ZYDIS_REGISTER_RAX;
        }
        if (0 == (REGISTER_BITMAP_ECX & Bitmap)) {
            return ZYDIS_REGISTER_RCX;
        }
        if (0 == (REGISTER_BITMAP_EDX & Bitmap)) {
            return ZYDIS_REGISTER_RDX;
        }
        if (0 == (REGISTER_BITMAP_EBX & Bitmap)) {
            return ZYDIS_REGISTER_RBX;
        }
        if (0 == (REGISTER_BITMAP_ESI & Bitmap)) {
            return ZYDIS_REGISTER_RSI;
        }
        if (0 == (REGISTER_BITMAP_EDI & Bitmap)) {
            return ZYDIS_REGISTER_RDI;
        }
        if (0 == (REGISTER_BITMAP_EBP & Bitmap)) {
            return ZYDIS_REGISTER_RBP;
        }
    }

    return ZYDIS_REGISTER_NONE;
}

PUCHAR
RINAPI
BuildInstructionFromRequest(
    __in PUCHAR Pointer,
    __in ULONG Count,
    __in ZydisEncoderRequest* Request
)
{
    ZyanStatus Status = ZYAN_STATUS_SUCCESS;
    ZyanUSize Length = 0;
    ZyanUSize Offset = 0;
    ULONG Index = 0;

    for (Index = 0; Index < Count; Index++) {
        Length = 15;
        Status = ZydisEncoderEncodeInstruction(
            &Request[Index],
            Pointer + Offset,
            &Length);

        if (FALSE == ZYAN_SUCCESS(Status)) {
            return NULL;
        }
        else {
            Offset += Length;
        }
    }

    return Pointer + Offset;
}

PUCHAR
RINAPI
BuildNormalInstruction(
    __in PUCHAR Pointer,
    __in PANALYZER Analyzer
)
{
    RtlCopyMemory(
        Pointer,
        Analyzer->Decoded.Address,
        Analyzer->Decoded.Length);

    Analyzer->Encoder.Address = Pointer;
    Analyzer->Encoder.Length = Analyzer->Decoded.Length;
    return Pointer + Analyzer->Encoder.Length;
}

PUCHAR
RINAPI
BuildRelativeInstruction(
    __in PUCHAR Pointer,
    __in PANALYZER Analyzer
)
{
    ZydisRegister IdleRegister = ZYDIS_REGISTER_NONE;
    ZydisEncoderRequest Request[4] = { 0 };
    ZyanU32 Index = 0;

    IdleRegister = SelectIdleRegister(Analyzer);

    if (ZYDIS_REGISTER_NONE != IdleRegister) {
        for (Index = 0; Index < Analyzer->Encoder.Request.operand_count; Index++) {
            if (ZYDIS_OPERAND_TYPE_MEMORY == Analyzer->Encoder.Request.operands[Index].type) {
                Analyzer->Encoder.Request.operands[Index].mem.base = IdleRegister;
                Analyzer->Encoder.Request.operands[Index].mem.displacement = 0;
            }
        }

        Request[0].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        Request[0].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
        Request[0].mnemonic = ZYDIS_MNEMONIC_PUSH;
        Request[0].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
        Request[0].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_64;
        Request[0].operand_count = 1;
        Request[0].operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
        Request[0].operands[0].reg.value = IdleRegister;

        Request[1].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        Request[1].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
        Request[1].mnemonic = ZYDIS_MNEMONIC_MOV;
        Request[1].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
        Request[1].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_64;
        Request[1].operand_count = 2;
        Request[1].operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
        Request[1].operands[0].reg.value = IdleRegister;
        Request[1].operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
        Request[1].operands[1].imm.u = (ZyanU64)Analyzer->Repair.Address;

        RtlCopyMemory(
            &Request[2],
            &Analyzer->Encoder.Request,
            sizeof(ZydisEncoderRequest));

        Request[3].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        Request[3].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
        Request[3].mnemonic = ZYDIS_MNEMONIC_POP;
        Request[3].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
        Request[3].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_64;
        Request[3].operand_count = 1;
        Request[3].operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
        Request[3].operands[0].reg.value = IdleRegister;

        Analyzer->Encoder.Address = Pointer;

        return BuildInstructionFromRequest(
            Analyzer->Encoder.Address,
            ARRAYSIZE(Request),
            Request);
    }

    return NULL;
}

BOOLEAN
RINAPI
BuildRemoteJccInstruction(
    __in PANALYZER Analyzer
)
{
    ZydisEncoderRequest Request[6] = { 0 };

    Request[0].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
    Request[0].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
    Request[0].mnemonic = Analyzer->Encoder.Request.mnemonic;
    Request[0].branch_type = ZYDIS_BRANCH_TYPE_SHORT;
    Request[0].branch_width = ZYDIS_BRANCH_WIDTH_8;
    Request[0].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
    Request[0].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_64;
    Request[0].operand_count = 1;
    Request[0].operands[0].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
    Request[0].operands[0].imm.s = 2;

    Request[1].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
    Request[1].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
    Request[1].mnemonic = ZYDIS_MNEMONIC_JMP;
    Request[1].branch_type = ZYDIS_BRANCH_TYPE_SHORT;
    Request[1].branch_width = ZYDIS_BRANCH_WIDTH_8;
    Request[1].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
    Request[1].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_64;
    Request[1].operand_count = 1;
    Request[1].operands[0].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
    Request[1].operands[0].imm.s = 16;

    Request[2].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
    Request[2].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
    Request[2].mnemonic = ZYDIS_MNEMONIC_PUSH;
    Request[2].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
    Request[2].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_64;
    Request[2].operand_count = 1;
    Request[2].operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
    Request[2].operands[0].reg.value = ZYDIS_REGISTER_RAX;

    Request[3].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
    Request[3].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
    Request[3].mnemonic = ZYDIS_MNEMONIC_MOV;
    Request[3].branch_type = ZYDIS_BRANCH_TYPE_NONE;
    Request[3].branch_width = ZYDIS_BRANCH_WIDTH_NONE;
    Request[3].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
    Request[3].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_64;
    Request[3].operand_count = 2;
    Request[3].operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
    Request[3].operands[0].reg.value = ZYDIS_REGISTER_RAX;
    Request[3].operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
    Request[3].operands[1].imm.u = (ZyanU64)Analyzer->Repair.Address;

    Request[4].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
    Request[4].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
    Request[4].mnemonic = ZYDIS_MNEMONIC_XCHG;
    Request[4].branch_type = ZYDIS_BRANCH_TYPE_NONE;
    Request[4].branch_width = ZYDIS_BRANCH_WIDTH_NONE;
    Request[4].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
    Request[4].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_32;
    Request[4].operand_count = 2;
    Request[4].operands[0].type = ZYDIS_OPERAND_TYPE_MEMORY;
    Request[4].operands[0].mem.base = ZYDIS_REGISTER_RSP;
    Request[4].operands[0].mem.index = ZYDIS_REGISTER_NONE;
    Request[4].operands[0].mem.scale = 0;
    Request[4].operands[0].mem.displacement = 0;
    Request[4].operands[0].mem.size = 8;
    Request[4].operands[1].type = ZYDIS_OPERAND_TYPE_REGISTER;
    Request[4].operands[1].reg.value = ZYDIS_REGISTER_RAX;

    Request[5].machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
    Request[5].allowed_encodings = ZYDIS_ENCODABLE_ENCODING_LEGACY;
    Request[5].mnemonic = ZYDIS_MNEMONIC_RET;
    Request[5].branch_type = ZYDIS_BRANCH_TYPE_NEAR;
    Request[5].branch_width = ZYDIS_BRANCH_WIDTH_64;
    Request[5].address_size_hint = ZYDIS_ADDRESS_SIZE_HINT_64;
    Request[5].operand_size_hint = ZYDIS_OPERAND_SIZE_HINT_64;
    Request[5].operand_count = 0;

    if (NULL == BuildInstructionFromRequest(
        Analyzer->Encoder.Address,
        ARRAYSIZE(Request),
        Request)) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
RINAPI
BuildLocalInstruction(
    __in PANALYZER Table,
    __in PANALYZER Analyzer
)
{
    ZyanStatus Status = ZYAN_STATUS_SUCCESS;
    ZyanUSize Length = 0;

    Analyzer->Encoder.Request.branch_type = ZYDIS_BRANCH_TYPE_NEAR;
    Analyzer->Encoder.Request.branch_width = ZYDIS_BRANCH_WIDTH_32;
    Analyzer->Encoder.Request.operands[0].imm.s
        = (ZyanI64)Table[Analyzer->Repair.Index].Encoder.Address
        - (ZyanI64)Analyzer->Encoder.Address - Analyzer->Encoder.Length;

    Length = 15;
    Status = ZydisEncoderEncodeInstruction(
        &Analyzer->Encoder.Request,
        Analyzer->Encoder.Address, &Length);

    if (FALSE == ZYAN_SUCCESS(Status)) {
        return FALSE;
    }

    return TRUE;
}

VOID
RINAPI
BuildTruncationInstruction(
    __in PANALYZER Analyzer
)
{
    UCHAR InstructionCode[] = {
        0x50,
        0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x48, 0x87, 0x04, 0x24,
        0xC3 };

    *(PUCHAR*)&InstructionCode[3] = Analyzer->Decoded.Address + Analyzer->Decoded.Length;

    RtlCopyMemory(
        Analyzer->Encoder.Address + Analyzer->Encoder.Length,
        InstructionCode,
        sizeof(InstructionCode));
}

VOID
RINAPI
BuildRemoteJmpInstruction(
    __in PANALYZER Analyzer
)
{
    UCHAR InstructionCode[] = {
        0x50,
        0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x48, 0x87, 0x04, 0x24,
        0xC3 };

    *(PUCHAR*)&InstructionCode[3] = Analyzer->Repair.Address;

    RtlCopyMemory(
        Analyzer->Encoder.Address,
        InstructionCode,
        sizeof(InstructionCode));
}

VOID
RINAPI
BuildRemoteCallInstruction(
    __in PANALYZER Analyzer
)
{
    UCHAR InstructionCode[] = {
        0xE8, 0x02, 0x00, 0x00, 0x00,
        0xEB, 0x10,
        0x50,
        0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x48, 0x87, 0x04, 0x24,
        0xC3 };

    *(PUCHAR*)&InstructionCode[10] = Analyzer->Repair.Address;

    RtlCopyMemory(
        Analyzer->Encoder.Address,
        InstructionCode,
        sizeof(InstructionCode));
}

VOID
RINAPI
BuildImpJmpInstruction(
    __in PANALYZER Analyzer
)
{
    UCHAR InstructionCode[] = {
        0x50,
        0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x48, 0x8B, 0x00,
        0x48, 0x87, 0x04, 0x24,
        0xC3 };

    *(PUCHAR*)&InstructionCode[3] = Analyzer->Repair.Address;

    RtlCopyMemory(
        Analyzer->Encoder.Address,
        InstructionCode,
        sizeof(InstructionCode));
}

VOID
RINAPI
BuildImpCallInstruction(
    __in PANALYZER Analyzer
)
{
    UCHAR InstructionCode[] = {
        0xE8, 0x02, 0x00, 0x00, 0x00,
        0xEB, 0x13,
        0x50,
        0x48, 0xB8, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
        0x48, 0x8B, 0x00,
        0x48, 0x87, 0x04, 0x24,
        0xC3 };

    *(PUCHAR*)&InstructionCode[10] = Analyzer->Repair.Address;

    RtlCopyMemory(
        Analyzer->Encoder.Address,
        InstructionCode,
        sizeof(InstructionCode));
}

BOOLEAN
RINAPI
BuildFirstProcedure(
    __in PUCHAR Procedure,
    __in PANALYZER Table,
    __in ULONG Count,
    __in_opt REPAIR_CALLBACK Callback
)
{
    ULONG Index = 0;
    PANALYZER Analyzer = NULL;
    PUCHAR Pointer = NULL;

    Pointer = Procedure;

    for (Index = 0; Index < Count; Index++) {
        Analyzer = &Table[Index];

        if (NULL != Callback) {
            if (0 != (BRANCH_REMOTE & Analyzer->Repair.Flags) ||
                0 != (BRANCH_IMP & Analyzer->Repair.Flags) ||
                0 != (REPAIR_REL & Analyzer->Repair.Flags)) {
                Analyzer->Repair.Address = Callback(
                    Analyzer->Repair.Flags,
                    Analyzer->Repair.Address);
            }
        }

        switch (Analyzer->Repair.Flags) {
        case REPAIR_NONE:
            Pointer = BuildNormalInstruction(Pointer, Analyzer);
            break;
        case REPAIR_REL:
            Pointer = BuildRelativeInstruction(Pointer, Analyzer);
            break;
        case REPAIR_JCC | BRANCH_LOCAL:
            Analyzer->Encoder.Length = 6;
            Analyzer->Encoder.Address = Pointer;
            Pointer += Analyzer->Encoder.Length;
            break;
        case REPAIR_JCC | BRANCH_REMOTE:
            Analyzer->Encoder.Length = 20;
            Analyzer->Encoder.Address = Pointer;
            Pointer += Analyzer->Encoder.Length;
            break;
        case REPAIR_JMP | BRANCH_LOCAL:
            Analyzer->Encoder.Length = 5;
            Analyzer->Encoder.Address = Pointer;
            Pointer += Analyzer->Encoder.Length;
            break;
        case REPAIR_JMP | BRANCH_REMOTE:
            Analyzer->Encoder.Length = 16;
            Analyzer->Encoder.Address = Pointer;
            Pointer += Analyzer->Encoder.Length;
            break;
        case REPAIR_JMP | BRANCH_IMP:
            Analyzer->Encoder.Length = 19;
            Analyzer->Encoder.Address = Pointer;
            Pointer += Analyzer->Encoder.Length;
            break;
        case REPAIR_CALL | BRANCH_LOCAL:
            Analyzer->Encoder.Length = 5;
            Analyzer->Encoder.Address = Pointer;
            Pointer += Analyzer->Encoder.Length;
            break;
        case REPAIR_CALL | BRANCH_REMOTE:
            Analyzer->Encoder.Length = 23;
            Analyzer->Encoder.Address = Pointer;
            Pointer += Analyzer->Encoder.Length;
            break;
        case REPAIR_CALL | BRANCH_IMP:
            Analyzer->Encoder.Length = 26;
            Analyzer->Encoder.Address = Pointer;
            Pointer += Analyzer->Encoder.Length;
            break;
        }

        if (NULL == Pointer) {
            return FALSE;
        }
    }

    BuildTruncationInstruction(Analyzer);
    return TRUE;
}

BOOLEAN
RINAPI
BuildSecondProcedure(
    __in PANALYZER Table,
    __in ULONG Count
)
{
    ULONG Index = 0;
    PANALYZER Analyzer = NULL;
    BOOLEAN State = TRUE;

    for (Index = 0; Index < Count; Index++) {
        Analyzer = &Table[Index];

        switch (Analyzer->Repair.Flags) {
        case REPAIR_JCC | BRANCH_LOCAL:
        case REPAIR_JMP | BRANCH_LOCAL:
        case REPAIR_CALL | BRANCH_LOCAL:
            State = BuildLocalInstruction(Table, Analyzer);
            break;
        case REPAIR_JCC | BRANCH_REMOTE:
            State = BuildRemoteJccInstruction(Analyzer);
            break;
        case REPAIR_JMP | BRANCH_REMOTE:
            BuildRemoteJmpInstruction(Analyzer);
            break;
        case REPAIR_JMP | BRANCH_IMP:
            BuildImpJmpInstruction(Analyzer);
            break;
        case REPAIR_CALL | BRANCH_REMOTE:
            BuildRemoteCallInstruction(Analyzer);
            break;
        case REPAIR_CALL | BRANCH_IMP:
            BuildImpCallInstruction(Analyzer);
            break;
        }

        if (FALSE == State) {
            return FALSE;
        }
    }

    return TRUE;
}

PVOID
RINAPI
RinReflector(
    __in PUCHAR Address,
    __in ULONG Length,
    __in ULONG Multiple,
    __in_opt REPAIR_CALLBACK Callback
)
{
    ULONG Count = 0;
    PANALYZER Table = NULL;
    PUCHAR Procedure = NULL;

    Procedure = RinAllocateMemory(Length * Multiple);

    if (NULL != Procedure) {
        RtlFillMemory(Procedure, Length * Multiple, 0xCC);
        Count = GetAnalyzerCount(Address, Length);
        Table = BuildAnalyzerTable(Address, Count);

        if (NULL != Table) {
            BuildAllRepairInfo(Table, Count);

            if (FALSE != BuildFirstProcedure(Procedure, Table, Count, Callback) &&
                FALSE != BuildSecondProcedure(Table, Count)) {
                CleanAnalyzerTable(Table);
                return Procedure;
            }

            CleanAnalyzerTable(Table);
        }

        RinFreeMemory(Procedure);
    }

    return NULL;
}

PUCHAR 
RINAPI 
ReflectCode(
    _In_ PUCHAR Address,
    _In_ ULONG Length,
    _In_ PUCHAR pTrampoline, 
    _In_ ULONG Size) 
{
	ULONG count = 0;
	PANALYZER pAnalyzers = NULL;
	BOOLEAN status = FALSE;
	s_TotalSize = 0;

	do
	{
		if (!ZYAN_SUCCESS(ZydisDecoderInit(&Decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64)))
			break;
		// 获取重构指令的数量
		count = GetAnalyzerCount(Address, Length);
		pAnalyzers = BuildAnalyzerTable(Address, count);
		if (NULL == pAnalyzers) {
			break;
		}
		BuildAllRepairInfo(pAnalyzers, count);
		status = BuildFirstProcedure(pTrampoline, pAnalyzers, count, NULL);
		if (!status) {
			break;
		}
		status = BuildSecondProcedure(pAnalyzers, count);
		if (!status) {
			break;
		}
	} while (FALSE);

	assert(s_TotalSize < Size);

	if (NULL != pAnalyzers) {
        RinFreeMemory(pAnalyzers);
		pAnalyzers = NULL;
	}

	if (!status) {
		return pTrampoline;
	}
	return pTrampoline + s_TotalSize;
}