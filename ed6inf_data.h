namespace NED63
{
	unsigned char* pMemPatchShowAtData;
	unsigned char pMemPatchShowAtDataNew[6] = { 0xE9, 0x2E, 0x01, 0x00, 0x00, 0x90 };
	unsigned char pMemPatchShowAtDataOld[6] = { 0x8B, 0x86, 0x8C, 0x00, 0x00, 0x00 };
}

namespace NED62
{
	unsigned char* pMemPatchShowAtData;
	unsigned char pMemPatchShowAtDataNew[6] = { 0xE9, 0xF2, 0x00, 0x00, 0x00, 0x90 };
	unsigned char pMemPatchShowAtDataOld[6] = { 0x8B, 0x86, 0x88, 0x00, 0x00, 0x00 };
}

namespace NED61
{
	unsigned char* pMemPatchShowAtData;
	unsigned char pMemPatchShowAtDataNew[8] = { 0xE9, 0xB6, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };
	unsigned char pMemPatchShowAtDataOld[8] = { 0xDB, 0x46, 0x48, 0x68, 0xFF, 0xFF, 0xFF, 0x00 };
}