#ifndef _ed6_common_H_E0FE925A_BC78_484e_B212_2DFA2B7787C9
#define _ed6_common_H_E0FE925A_BC78_484e_B212_2DFA2B7787C9

#include "MyLibrary.h"
//#include "pragma_once.h"
//#include <Windows.h>
//#include "my_headers.h"


typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char UCHAR, ubyte;

#define FSkip(n) DUMMY_STRUCT(n)
#define RESISTANCE UINT

#pragma pack(1)
#pragma warning (disable: 4201)

typedef struct  //DAT DIR�ļ�����
{
	union
	{
		int	file;
		struct
		{
			USHORT  Index;
			USHORT  DatNo;
		};
	};
} FileIndex;

typedef struct  //ÿ��12+24�ֽ�
{
	union
	{
		DUMMY_STRUCT(0x24);
		struct
		{
			char	fileName[0x10];
			UINT	fileSize;
		};
	};

} FILE_IN_DIR;

typedef struct  // һ�� 20�ֽ�
{
	RESISTANCE	condition;
	DUMMY_STRUCT(4);
	USHORT      Parameter;  // 0101 �غ�, 0201 ����, 0301 AT�������ٴ�, 0401 ����(ʯ��)
	SHORT       Effect; // �ٷֱ���ֵ
	USHORT      ST;
	DUMMY_STRUCT(6);
} CONDITION;

typedef struct
{
	ushort	CraftIndex; 
	ubyte   CraftAnimation1;
	ubyte   CraftAnimation2;
} CraftLastUsed_INFO;

namespace NED61
{
	typedef struct
	{
		USHORT				Level;
		USHORT				HPMax;
		USHORT				HP;

		USHORT				EPMax;
		USHORT				EP;
		USHORT				CP;
		int					EXP;    
		SHORT				STR;
		SHORT				DEF;
		SHORT				ATS;
		SHORT				ADF;
		SHORT				DEX;
		SHORT				AGL;
		SHORT				MOV;
		SHORT				SPD;
		USHORT				CPMAX;
		DUMMY_STRUCT(4);
		USHORT				RNG;
		DUMMY_STRUCT(4);
		RESISTANCE          condition;  // RESISTANCE���ͣ���������Ч
		USHORT              CharacterIndex;
		DUMMY_STRUCT(2);
	} ED6_CHARACTER_STATUS;
	typedef ED6_CHARACTER_STATUS ED6_STATUS;

	RESISTANCE conditionAbnormal = 0xBFF;
	RESISTANCE conditionDown = 0x2AA0400;

	bool IsSoldierParty(UINT SoldierNo)
	{
		return SoldierNo < 6;
	}

	bool IsSoldierEnemy(UINT SoldierNo)
	{
		return (SoldierNo >= 6) && (SoldierNo < 6+8);
	}
}

namespace NED62
{
	typedef struct
	{
		UINT				Level;
		UINT				HPMax;
		UINT				HP;
		USHORT				EPMax;
		USHORT				EP;
		USHORT				CP;
		DUMMY_STRUCT(2);
		UINT				EXP;    
		SHORT				STR;
		SHORT				DEF;
		SHORT				ATS;
		SHORT				ADF;
		SHORT				DEX;
		SHORT				AGL;
		SHORT				MOV;
		SHORT				SPD;
		USHORT				CPMAX;
		DUMMY_STRUCT(4);
		USHORT				RNG;
		DUMMY_STRUCT(4);
		RESISTANCE			condition;  // RESISTANCE���ͣ���������Ч
		USHORT              CharacterIndex;
		DUMMY_STRUCT(2);
	} ED6_CHARACTER_STATUS;
	typedef ED6_CHARACTER_STATUS ED6_STATUS;

	RESISTANCE conditionAbnormal = 0x28000BFF;
	RESISTANCE conditionDown = 0x2AA8400;

	bool IsSoldierParty(UINT SoldierNo)
	{
		return SoldierNo < 8;
	}

	bool IsSoldierEnemy(UINT SoldierNo)
	{
		return (SoldierNo >= 8) && (SoldierNo < 8+8);
	}
}

namespace NED63
{
	typedef NED62::ED6_CHARACTER_STATUS ED6_CHARACTER_STATUS;
	typedef NED62::ED6_CHARACTER_STATUS ED6_STATUS;

	RESISTANCE conditionAbnormal = 0x28000BFF;
	RESISTANCE conditionDown = 0x2AA8400;

	bool IsSoldierParty(UINT SoldierNo)
	{
		return SoldierNo < 8;
	}

	bool IsSoldierEnemy(UINT SoldierNo)
	{
		return (SoldierNo >= 8) && (SoldierNo < 8+8);
	}
}


typedef struct  // Ai��ʽ
{
	ubyte	Time;
	ubyte	Probability;
	ubyte	Target;
	ubyte	DUMMY_STRUCT_01;        // 01
	ubyte   CraftAnimation1;        // �������룬��AS�ļ��ж��塣��ħ��06����ħ��07��C��10-19��S��1A-1F
	ubyte   CraftAnimation2;
	ushort	CraftIndex;             // ���ܴ��룬�����t_magic._dt���Զ��弼�ܴ�E8 03��ʼ
	int		TimeParameter1;
	int		TimeParameter2;
	int		TargetParameter1;
	int		TargetParameter2;
} ED6_AI_INFO;
typedef ED6_AI_INFO ED7_AI_INFO;


typedef struct  // �Զ��弼�ܸ�ʽ
{
	ushort	AnimationType;
	ubyte	Target; 
	ubyte	SpecialEffect;		// 01 ǿ������ 02 ����ʱ�޷������
	ubyte	Att;				// ����
	// �չ�FCSC		0�� 1�� 2ˮ 3�� 4�� 5�� 6�� 0ʱ	// FCSCû�� ʱ�ջ���Ч�ʣ���������ν
	// �չ�3rd		0�� 1�� 2ˮ 3�� 4�� 5�� 6�� 7ʱ
	// ���̹�		0�� 1�� 2ˮ 3�� 4�� 5ʱ 6�� 7��
	ubyte	ShapeScope;			// ��Χ��״
	ubyte	Effect1;
	ubyte	Effect2;
#if MAGICSIZE == 32

	ushort	RNG;
	ushort	ScopeRadius;		// �뾶
	ushort	STCharge;
	ushort	STCoolDown;
	ushort	CP_EP;
	ushort	DisplayIndex;		// ħ���б��е���ʾ�Ⱥ�t_magic._dt�еĲ���Ч��ms�ļ��е�һ��Ϊ1��0��

#elif MAGICSIZE == 28

	ubyte	DisplayIndex;		// ħ���б��е���ʾ�Ⱥ�t_magic._dt�еĲ���Ч��ms�ļ��е�һ��Ϊ1��0��
	ubyte	RNG;
	ubyte	STCharge;
	ubyte	STCoolDown;
	ushort	CP_EP;
	ushort	ScopeRadius;		// �뾶
#else
#endif

	short	Effect1Parameter;
	ushort	Effect1ST;
	short	Effect2Parameter;
	ushort	Effect2ST;

	//DUMMY_STRUCT(4);
	ushort  addrMagicName;	// ħ���� ��ַ
	ushort  addrMagicIntro;	// ħ��˵�� ��ַ
} ED6_CRAFT_INFO;
typedef ED6_CRAFT_INFO ED7_CRAFT_INFO;

typedef struct
{
	CHAR Description[0x100];
	CHAR Name[0x20];
} ED6_CRAFT_INTRO;
typedef ED6_CRAFT_INTRO ED7_CRAFT_INTRO;

enum
{
    CHR_FLAG_ENEMY  = 0x1000,
    CHR_FLAG_NPC    = 0x2000,
    CHR_FLAG_PARTY  = 0x4000,
    CHR_FLAG_EMPTY  = 0x8000,
};

enum
{
    CHR_FLAG_ResistBeatBack     = 0x0200,
    CHR_FLAG_ResistATDelay      = 0x0800,
    CHR_FLAG_AbsoluteMiss       = 0x2000,
};

namespace NED61
{
	typedef struct // 0x23C8
	{
		union
		{
			DUMMY_STRUCT(0x23C8);
			struct
			{
				USHORT              SoldierNo;                  // �ҷ�0-7���з�8-15��֧Ԯ16-19��������20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
				USHORT				Flags;                      // 10 �з� 40 ���� ...
				BYTE				DeathFlags;                 // 02 ������ս��ʤ���ж� 04 ��������ս����
				BYTE				UnderAttackFlags;           // 08 Resist ATDelay 02 �������� 01 ��������ת��(3D)  10 �޷���������  20 ǿ��miss
                */
				DUMMY_STRUCT(4);
				USHORT              CharacterIndex;
				//DUMMY_STRUCT(2);
				//USHORT              ATActTime;  				// AT�������ٴΣ���ʼΪ0
				FileIndex			SYFileIndex;
				FileIndex			MSFileIndex;
				FileIndex			ASFileIndex;				// 0x14
				DUMMY_STRUCT(0x1B8);
				// 0x1D0
				ED6_STATUS			StatusBasic;    			// normal�ѶȻ���ֵ
				ED6_STATUS			StatusSum;					// �����Ѷȡ�װ������·
				USHORT				MoveSPD;					// �ƶ��ٶȣ��ҷ���ԱҲ��ms�ļ��ж�ȡ
				USHORT				MoveAfterAttack;			// ��ms�ļ��ж�ȡ
				CONDITION           Condition[29];
				DUMMY_STRUCT(24);
				int					AT;
				int					AT2;
				DUMMY_STRUCT(6);
				USHORT				AIType;
				DUMMY_STRUCT(2);
				ushort				DropIndex[2];               // ������ ��Ʒ����
				ubyte				DropProbability[2];         // ������� 
				ubyte				Sex; 
				DUMMY_STRUCT(1);
				ushort				Equip[5];                   // װ��
				ushort				Orb[6];                     // ��·   7�� 
				//DUMMY_STRUCT(2);
				ED6_AI_INFO	        NormalAttack;
				ED6_AI_INFO         Art[80];
				ED6_AI_INFO         Craft[10];
				ED6_AI_INFO         SCraft[4];

				CraftLastUsed_INFO	CraftLastUsed;
				BYTE				fleeParameter[4];
				ED6_CRAFT_INFO		CraftInf[16];
				ED6_CRAFT_INTRO		CraftIntro[16];

				//DUMMY_STRUCT(0x203A);
				DUMMY_STRUCT(0xC);
				// 0x21C4
				ubyte				Sepith[7];                  // ����ҫ��Ƭ
				FSkip(1);
				ushort				EXPGet;                     // ս��ʤ�� ���㾭��ǰ�ݴ�
				FSkip(2);
				ushort				ArtsAttResistance[4];       // ����������Ч��
				DUMMY_STRUCT(0x84);
				RESISTANCE			Resistance;					// 0x225C
				DUMMY_STRUCT(0x48);
				char				CharacterIntro[0x100];		// 0x22A8
			};
		};
	} ED6_CHARACTER_BATTLE_INF;
}

namespace NED62
{
	typedef struct // 0x2478
	{
		union
		{
			DUMMY_STRUCT(0x2478);
			struct
			{
				USHORT              SoldierNo;                  // �ҷ�0-7���з�8-15��֧Ԯ16-19��������20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
				USHORT				Flags;                      // 10 �з� 40 ���� ...
				BYTE				DeathFlags;                 // 02 ������ս��ʤ���ж� 04 ��������ս����
				BYTE				UnderAttackFlags;           // 08 Resist ATDelay 02 �������� 01 ��������ת��(3D)  10 �޷���������  20 ǿ��miss
                */
				DUMMY_STRUCT(4);
				USHORT              CharacterIndex;
				//DUMMY_STRUCT(2);
				//USHORT              ATActTime;  				// AT�������ٴΣ���ʼΪ0
				FileIndex			SYFileIndex;
				FileIndex			MSFileIndex;
				FileIndex			ASFileIndex;				// 0x14
				DUMMY_STRUCT(0x214);
				// 0x22C
				ED6_STATUS			StatusBasic;    			// normal�ѶȻ���ֵ
				ED6_STATUS			StatusSum;					// �����Ѷȡ�װ������·
				USHORT				MoveSPD;					// �ƶ��ٶȣ��ҷ���ԱҲ��ms�ļ��ж�ȡ
				USHORT				MoveAfterAttack;			// ��ms�ļ��ж�ȡ
				CONDITION           Condition[31];
				DUMMY_STRUCT(24);
				int					AT;
				int					AT2;
				DUMMY_STRUCT(6);
				USHORT				AIType;
				ushort				DropIndex[2];               // ������ ��Ʒ����
				ubyte				DropProbability[2];         // ������� 
				ubyte				Sex; 
				DUMMY_STRUCT(1);
				ushort				Equip[5];                   // װ��
				ushort				Orb[7];                     // ��·   7�� 
				//DUMMY_STRUCT(2);
				ED6_AI_INFO	        NormalAttack;
				ED6_AI_INFO         Art[80];
				ED6_AI_INFO         Craft[10];
				ED6_AI_INFO         SCraft[5];

				CraftLastUsed_INFO	CraftLastUsed;
				BYTE				fleeParameter[4];
				ED6_CRAFT_INFO		CraftInf[16];
				ED6_CRAFT_INTRO		CraftIntro[16];

				//DUMMY_STRUCT(0x203A);
				DUMMY_STRUCT(0x4C);
				// 0x22B0
				ubyte				Sepith[7];                  // ����ҫ��Ƭ
				FSkip(1);
				ushort				EXPGet;                     // ս��ʤ�� ���㾭��ǰ�ݴ�
				FSkip(2);
				ushort				ArtsAttResistance[4];       // ����������Ч��
				//FSkip(2);
				RESISTANCE			Resistance;					// 0x22C4
				DUMMY_STRUCT(0x94);
				//char				CharacterIntro[0x120];		// 0x2358
				char				CharacterIntro[0x100];		// 0x2358
			};
		};
	} ED6_CHARACTER_BATTLE_INF;
}

namespace NED63
{
	typedef struct // 0x2490
	{
		union
		{
			DUMMY_STRUCT(0x2490);
			struct
			{
				USHORT              SoldierNo;                  // �ҷ�0-7���з�8-15��֧Ԯ16-19��������20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
				USHORT				Flags;                      // 10 �з� 40 ���� ...
				BYTE				DeathFlags;                 // 02 ������ս��ʤ���ж� 04 ��������ս����
				BYTE				UnderAttackFlags;           // 08 Resist ATDelay 02 �������� 01 ��������ת��(3D)  10 �޷���������  20 ǿ��miss
                */
				DUMMY_STRUCT(4);
				USHORT              CharacterIndex;
				DUMMY_STRUCT(2);
				USHORT              ATActTime;  				// AT�������ٴΣ���ʼΪ0
				FileIndex			SYFileIndex;
				FileIndex			MSFileIndex;
				FileIndex			ASFileIndex;				// 0x18
				DUMMY_STRUCT(0x162);
				USHORT				wActType;
				DUMMY_STRUCT(0xA);
				USHORT				wUseMagic;
				DUMMY_STRUCT(0xA0);
				//DUMMY_STRUCT(0x210);
				// 0x22C
				ED6_STATUS			StatusBasic;    			// normal�ѶȻ���ֵ
				ED6_STATUS			StatusSum;					// �����Ѷȡ�װ������·
				USHORT				MoveSPD;					// �ƶ��ٶȣ��ҷ���ԱҲ��ms�ļ��ж�ȡ
				USHORT				MoveAfterAttack;			// ��ms�ļ��ж�ȡ
				CONDITION           Condition[32];
				DUMMY_STRUCT(24);
				int					AT;
				int					AT2;
				DUMMY_STRUCT(4);
				USHORT				AIType;
				ushort				DropIndex[2];               // ������ ��Ʒ����
				ubyte				DropProbability[2];         // �������   
				ubyte				Sex; 
				DUMMY_STRUCT(1);
				ushort				Equip[5];                   // װ��
				ushort				Orb[7];                     // ��·   7�� 
				DUMMY_STRUCT(2);
				ED6_AI_INFO	        NormalAttack;
				ED6_AI_INFO         Art[80];
				ED6_AI_INFO         Craft[10];
				ED6_AI_INFO         SCraft[5];

				CraftLastUsed_INFO	CraftLastUsed;
				BYTE				fleeParameter[4];
				ED6_CRAFT_INFO		CraftInf[16];
				ED6_CRAFT_INTRO		CraftIntro[16];

				//DUMMY_STRUCT(0x203A);
				DUMMY_STRUCT(0x4B);
				ubyte				Sepith[7];                  // ����ҫ��Ƭ
				FSkip(2);
				ushort				EXPGet;                     // ս��ʤ�� ���㾭��ǰ�ݴ�
				FSkip(2);
				ushort				ArtsAttResistance[7];       // ����������Ч��
				FSkip(2);
				RESISTANCE			Resistance;					// 0x22E0
				DUMMY_STRUCT(0x8C);
				char				CharacterIntro[0x100];		// 0x2370
			};
		};
	} ED6_CHARACTER_BATTLE_INF;

	typedef struct
	{
		ushort  index;
		FSkip(2);
		ushort	item[8];
		byte	probability[8];	
	} ED6_DROP_ITEM;
}


namespace NEDZERO
{
	typedef struct
	{
		int					HPMax;
		int					HP;
		ushort				Level;
		ushort				EPMax;
		ushort				EP;
		ushort				CP;
		UINT				EXP;    
		short				STR;
		short				DEF;
		short				ATS;
		short				ADF;
		short				DEX;
		short				AGL;
		short				MOV;
		short				SPD;
		short				DEXRate;
		short				AGLRate;
		ushort				CPMAX;
		DUMMY_STRUCT(0x2);
		ushort				RNG;		// �ڴ��и�StatusBasic�еĲ���Ч
		DUMMY_STRUCT(0x2);
		RESISTANCE          condition;  // RESISTANCE���ͣ���������Ч; ս��������浵�� DEATH��Ч
	} ED6_CHARACTER_STATUS;
	typedef ED6_CHARACTER_STATUS ED6_STATUS;
	typedef ED6_CHARACTER_STATUS ED7_CHARACTER_STATUS;
	typedef ED6_CHARACTER_STATUS ED7_STATUS;
}

namespace NEDAO
{
	typedef NEDZERO::ED6_CHARACTER_STATUS ED6_CHARACTER_STATUS;
	typedef ED6_CHARACTER_STATUS ED6_STATUS;
	typedef ED6_CHARACTER_STATUS ED7_CHARACTER_STATUS;
	typedef ED6_CHARACTER_STATUS ED7_STATUS;

	typedef struct // 0x2424
	{
		union
		{
			DUMMY_STRUCT(0x2424);
			struct
			{
				USHORT              SoldierNo;                  // �ҷ�0-7���з�8-15��֧Ԯ16-19��������20-21
				USHORT				Flags;                      // 10 �з� 40 ���� ...
				BYTE				DeathFlags;                 // 02 ������ս��ʤ���ж� 04 ��������ս����
				BYTE				UnderAttackFlags;           // 08 Resist ATDelay 02 �������� 01 ��������ת��(3D)  10 �޷���������  20 ǿ��miss
				DUMMY_STRUCT(4);
				USHORT              CharacterIndex;
				DUMMY_STRUCT(2);
				USHORT              ATActTime;  				// AT�������ٴΣ���ʼΪ0
				FileIndex			SYFileIndex;
				FileIndex			MSFileIndex;
				FileIndex			ASFileIndex;				// 0x18
				DUMMY_STRUCT(0x156);
				USHORT				wActType;
				DUMMY_STRUCT(0xA);
				USHORT				wUseMagic;
				DUMMY_STRUCT(0xB4);
				//DUMMY_STRUCT(0x218);
				ED6_STATUS			StatusBasic;    			// normal�ѶȻ���ֵ
				ED6_STATUS			StatusSum;					// �����Ѷȡ�װ������·

				ushort				MoveSPD;                // �ƶ��ٶȣ��ҷ���ԱҲ��ms�ļ��ж�ȡ
				ushort				MoveAfterAttack;        // ��ms�ļ��ж�ȡ
				CONDITION			Condition[32];
				DUMMY_STRUCT(24);
				int				AT;
				int				AT2;    // AiTime08��
				
				ushort				AIType;
				ushort				EXPGet;                     // ս��ʤ�� ���㾭��ǰ�ݴ�
				ushort				DropIndex[2];                 // ������ ��Ʒ����
				//ushort				DropIndex2;
				ubyte				DropProbability[2];           // �������
				//ubyte				DropProbability2;   
				ubyte				Sex; 
				ubyte				DisplayHighLevelArtsAttResistance;      
				// �Ƿ���ʱ�ջ���Ч����ʾ��0/8 ������ 1/9 ������Ӧ���Ǳ�־λ�����ܺ��и�����Ϣ
				ushort				Equip[5];                   // װ��
				ushort				Orb[7];                     // ��·   7�� 
				
				ED7_AI_INFO			NormalAttack;
				ED7_AI_INFO			Arts[80];
				ED7_AI_INFO			Craft[10];
				ED7_AI_INFO			SCraft[5];
				ED7_AI_INFO			SupportCraft[3]; 
				CraftLastUsed_INFO	CraftLastUsed;
				BYTE				fleeParameter[4];
				ED7_CRAFT_INFO		CraftInf[16];
				ED7_CRAFT_INTRO		CraftIntro[16];
				
				ubyte				Sepith[7];                  // ����ҫ��Ƭ
				FSkip(3);
				ushort				ArtsAttResistance[7];       // ����������Ч��
				RESISTANCE			Resistance;					// 0x2304
				DUMMY_STRUCT(0x78);
				char				CharacterIntro[0x80];		// 0x2380
			};
		};
	} ED6_CHARACTER_BATTLE_INF;
	typedef ED6_CHARACTER_BATTLE_INF ED7_CHARACTER_BATTLE_INF;
}
#pragma pack()
#pragma warning (default: 4201)

#endif