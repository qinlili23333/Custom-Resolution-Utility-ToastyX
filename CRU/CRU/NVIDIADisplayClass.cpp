//---------------------------------------------------------------------------
#include "Common.h"
#pragma hdrstop

#include "NVIDIADisplayClass.h"
//---------------------------------------------------------------------------
#define NVAPI_OK                        0
#define NVAPI_ERROR						-1
#define NVAPI_MAX_HEADS_PER_GPU         32
#define NVAPI_MAX_PHYSICAL_GPUS         64
#define NV_EDID_DATA_SIZE               256
#define NV_EDID_VER3                    ((3 << 16) | sizeof(NV_EDID_V3))
#define NV_I2C_INFO_VER2                ((2 << 16) | sizeof(NV_I2C_INFO_V2))
#define NVAPI_I2C_SPEED_DEPRECATED		65535
//---------------------------------------------------------------------------
enum
{
	I2C_READ,
	I2C_WRITE,
};

enum NV_I2C_SPEED
{
	NVAPI_I2C_SPEED_DEFAULT,
	NVAPI_I2C_SPEED_3KHZ,
	NVAPI_I2C_SPEED_10KHZ,
	NVAPI_I2C_SPEED_33KHZ,
	NVAPI_I2C_SPEED_100KHZ,
	NVAPI_I2C_SPEED_200KHZ,
	NVAPI_I2C_SPEED_400KHZ,
};
//---------------------------------------------------------------------------
struct NV_EDID_V3
{
	int version;
	unsigned char EDID_Data[NV_EDID_DATA_SIZE];
	int sizeofEDID;
	int edidId;
	int offset;
};

struct NV_I2C_INFO_V2
{
	int version;
	int displayMask;
	unsigned char bIsDDCPort;
	unsigned char i2cDevAddress;
	unsigned char *pbI2cRegAddress;
	int regAddrSize;
	unsigned char *pbData;
	int cbSize;
	int i2cSpeed;
	int i2cSpeedKhz;
};
//---------------------------------------------------------------------------
typedef void *(*NVAPI_QUERYINTERFACE)(int);
typedef int (*NVAPI_INITIALIZE)();
typedef int (*NVAPI_ENUMPHYSICALGPUS)(int **, int *);
typedef int (*NVAPI_GPU_GETACTIVEOUTPUTS)(int *, int *);
typedef int (*NVAPI_GPU_GETEDID)(int *, int, NV_EDID_V3 *);
typedef int (*NVAPI_I2CREAD)(int *, NV_I2C_INFO_V2 *);
typedef int (*NVAPI_I2CWRITE)(int *, NV_I2C_INFO_V2 *);
typedef int (*NVAPI_UNLOAD)();
//---------------------------------------------------------------------------
NVAPI_QUERYINTERFACE                    NvAPI_QueryInterface;
NVAPI_INITIALIZE                        NvAPI_Initialize;
NVAPI_ENUMPHYSICALGPUS                  NvAPI_EnumPhysicalGPUs;
NVAPI_GPU_GETACTIVEOUTPUTS              NvAPI_GPU_GetActiveOutputs;
NVAPI_GPU_GETEDID                       NvAPI_GPU_GetEDID;
NVAPI_I2CREAD							NvAPI_I2CRead;
NVAPI_I2CWRITE							NvAPI_I2CWrite;
NVAPI_UNLOAD                            NvAPI_Unload;
//---------------------------------------------------------------------------
int NVIDIADisplayClass::DisplayI2C(int Action, int *GPUHandle, int OutputID, unsigned char Address, unsigned char *Data, int Size)
{
	NV_I2C_INFO_V2 Info;
	Info.version = NV_I2C_INFO_VER2;
	Info.displayMask = OutputID;
	Info.bIsDDCPort = true;
	Info.i2cDevAddress = Address;
	Info.pbI2cRegAddress = NULL;
	Info.regAddrSize = 0;
	Info.pbData = Data + Action;
	Info.cbSize = Size - Action;
	Info.i2cSpeed = NVAPI_I2C_SPEED_DEPRECATED;
	Info.i2cSpeedKhz = NVAPI_I2C_SPEED_10KHZ;

	switch (Action)
	{
		case I2C_READ:
			return NvAPI_I2CRead(GPUHandle, &Info);

		case I2C_WRITE:
			return NvAPI_I2CWrite(GPUHandle, &Info);
	}

	return NVAPI_ERROR;
}
//---------------------------------------------------------------------------
bool NVIDIADisplayClass::LoadEDIDList(EDIDListClass &EDIDList)
{
	HMODULE Library;
	int *GPUHandle[NVAPI_MAX_PHYSICAL_GPUS];
	int GPUCount;
	int GPUIndex;
	int OutputMask;
	int OutputIndex;
	int OutputID;
	NV_EDID_V3 DisplayEDID;
	int EDIDSize;
	int DataSize;
	unsigned char NewDisplayEDID[MAX_EDID_BLOCKS * 128];
	int Offset;
	bool Status = false;

	Library = LoadLibrary("nvapi.dll");

	if (!Library)
		return false;

	NvAPI_QueryInterface = (NVAPI_QUERYINTERFACE)GetProcAddress(Library, "nvapi_QueryInterface");

	if (!NvAPI_QueryInterface)
		return false;

	NvAPI_Initialize = (NVAPI_INITIALIZE)NvAPI_QueryInterface(0x0150E828);
	NvAPI_EnumPhysicalGPUs = (NVAPI_ENUMPHYSICALGPUS)NvAPI_QueryInterface(0xE5AC921F);
	NvAPI_GPU_GetActiveOutputs = (NVAPI_GPU_GETACTIVEOUTPUTS)NvAPI_QueryInterface(0xE3E89B6F);
	NvAPI_GPU_GetEDID = (NVAPI_GPU_GETEDID)NvAPI_QueryInterface(0x37D32E69);
	NvAPI_I2CRead = (NVAPI_I2CREAD)NvAPI_QueryInterface(0x2FDE12C5);
	NvAPI_I2CWrite = (NVAPI_I2CWRITE)NvAPI_QueryInterface(0xE812EB07);
	NvAPI_Unload = (NVAPI_UNLOAD)NvAPI_QueryInterface(0xD22BDD7E);

	if (!NvAPI_Initialize)
		return false;

	if (!NvAPI_EnumPhysicalGPUs)
		return false;

	if (!NvAPI_GPU_GetActiveOutputs)
		return false;

	if (!NvAPI_GPU_GetEDID)
		return false;

	if (!NvAPI_I2CRead)
		return false;

	if (!NvAPI_I2CWrite)
		return false;

	if (!NvAPI_Unload)
		return false;

	if (NvAPI_Initialize() != NVAPI_OK)
		return false;

	if (NvAPI_EnumPhysicalGPUs(GPUHandle, &GPUCount) != NVAPI_OK)
		return false;

	for (GPUIndex = 0; GPUIndex < GPUCount; GPUIndex++)
	{
		if (NvAPI_GPU_GetActiveOutputs(GPUHandle[GPUIndex], &OutputMask) != NVAPI_OK)
			continue;

		for (OutputIndex = 0; OutputIndex < NVAPI_MAX_HEADS_PER_GPU; OutputIndex++)
		{
			OutputID = 1 << OutputIndex;

			if (OutputMask & OutputID)
			{
				std::memset(&DisplayEDID, 0, sizeof DisplayEDID);
				DisplayEDID.version = NV_EDID_VER3;

				if (NvAPI_GPU_GetEDID(GPUHandle[GPUIndex], OutputID, &DisplayEDID) != NVAPI_OK)
					continue;

				EDIDSize = DisplayEDID.sizeofEDID;

				if (EDIDSize > MAX_EDID_BLOCKS * 128)
					EDIDSize = MAX_EDID_BLOCKS * 128;

				DataSize = EDIDSize;

				if (DataSize > NV_EDID_DATA_SIZE)
					DataSize = NV_EDID_DATA_SIZE;

				std::memset(NewDisplayEDID, 0, MAX_EDID_BLOCKS * 128);
				std::memcpy(NewDisplayEDID, DisplayEDID.EDID_Data, DataSize);

				for (Offset = 256; Offset < EDIDSize; Offset += 256)
				{
					std::memset(&DisplayEDID, 0, sizeof DisplayEDID);
					DisplayEDID.version = NV_EDID_VER3;
					DisplayEDID.offset = Offset;

					if (NvAPI_GPU_GetEDID(GPUHandle[GPUIndex], OutputID, &DisplayEDID) != NVAPI_OK)
						break;

					if (DisplayEDID.EDID_Data[0] == 0)
						break;

					DataSize = EDIDSize - Offset;

					if (DataSize > NV_EDID_DATA_SIZE)
						DataSize = NV_EDID_DATA_SIZE;

					std::memcpy(&NewDisplayEDID[Offset], DisplayEDID.EDID_Data, DataSize);
				}
				/*
				for (Offset = 256; Offset < EDIDSize; Offset += 128)
				{
					Sleep(50);
					unsigned char SetSegment[] = {0x60, Offset / 256};

					if (DisplayI2C(I2C_WRITE, GPUHandle[GPUIndex], OutputID, 0x60, SetSegment, sizeof SetSegment) != NVAPI_OK)
						break;

					Sleep(50);
					unsigned char SetOffset[] = {0xA0, Offset % 256};

					if (DisplayI2C(I2C_WRITE, GPUHandle[GPUIndex], OutputID, 0xA0, SetOffset, sizeof SetOffset) != NVAPI_OK)
						break;

					Sleep(50);
					unsigned char Data[128];

					if (DisplayI2C(I2C_READ, GPUHandle[GPUIndex], OutputID, 0xA1, Data, sizeof Data) != NVAPI_OK)
						break;

					if (Data[0] == 0)
						break;

					std::memcpy(&NewDisplayEDID[Offset], Data, sizeof Data);
				}
				*/
				EDIDList.Add(NewDisplayEDID);
				Status = true;
			}
		}
	}

	NvAPI_Unload();
	FreeLibrary(Library);
	return Status;
}
//---------------------------------------------------------------------------
