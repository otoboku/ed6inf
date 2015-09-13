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

typedef struct  //DAT DIR文件索引
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

typedef struct  //每组12+24字节
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

typedef struct  // 一组 20字节
{
	RESISTANCE	condition;
	DUMMY_STRUCT(4);
	USHORT      Parameter;  // 0101 回合, 0201 次数, 0301 AT条动多少次, 0401 永久(石化)
	SHORT       Effect; // 百分比数值
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
		RESISTANCE          condition;  // RESISTANCE类型，进程中有效
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
		RESISTANCE			condition;  // RESISTANCE类型，进程中有效
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


typedef struct  // Ai格式
{
	ubyte	Time;
	ubyte	Probability;
	ubyte	Target;
	ubyte	DUMMY_STRUCT_01;        // 01
	ubyte   CraftAnimation1;        // 动画代码，在AS文件中定义。蓄魔法06、放魔法07、C技10-19，S技1A-1F
	ubyte   CraftAnimation2;
	ushort	CraftIndex;             // 技能代码，常规查t_magic._dt，自定义技能从E8 03开始
	int		TimeParameter1;
	int		TimeParameter2;
	int		TargetParameter1;
	int		TargetParameter2;
} ED6_AI_INFO;
typedef ED6_AI_INFO ED7_AI_INFO;


typedef struct  // 自定义技能格式
{
	ushort	AnimationType;
	ubyte	Target; 
	ubyte	SpecialEffect;		// 01 强制命中 02 驱动时无法被打断
	ubyte	Att;				// 属性
	// 空轨FCSC		0无 1地 2水 3火 4风 5空 6幻 0时	// FCSC没有 时空幻有效率，所以无所谓
	// 空轨3rd		0无 1地 2水 3火 4风 5空 6幻 7时
	// 零轨碧轨		0无 1地 2水 3火 4风 5时 6空 7幻
	ubyte	ShapeScope;			// 范围形状
	ubyte	Effect1;
	ubyte	Effect2;
#if MAGICSIZE == 32

	ushort	RNG;
	ushort	ScopeRadius;		// 半径
	ushort	STCharge;
	ushort	STCoolDown;
	ushort	CP_EP;
	ushort	DisplayIndex;		// 魔法列表中的显示先后，t_magic._dt中的才有效，ms文件中的一般为1或0。

#elif MAGICSIZE == 28

	ubyte	DisplayIndex;		// 魔法列表中的显示先后，t_magic._dt中的才有效，ms文件中的一般为1或0。
	ubyte	RNG;
	ubyte	STCharge;
	ubyte	STCoolDown;
	ushort	CP_EP;
	ushort	ScopeRadius;		// 半径
#else
#endif

	short	Effect1Parameter;
	ushort	Effect1ST;
	short	Effect2Parameter;
	ushort	Effect2ST;

	//DUMMY_STRUCT(4);
	ushort  addrMagicName;	// 魔法名 地址
	ushort  addrMagicIntro;	// 魔法说明 地址
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
				USHORT              SoldierNo;                  // 我方0-7，敌方8-15，支援16-19，待分身20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
				USHORT				Flags;                      // 10 敌方 40 己方 ...
				BYTE				DeathFlags;                 // 02 不参与战场胜利判定 04 死后留在战场上
				BYTE				UnderAttackFlags;           // 08 Resist ATDelay 02 不被击退 01 被攻击不转身(3D)  10 无法被攻击到  20 强制miss
                */
				DUMMY_STRUCT(4);
				USHORT              CharacterIndex;
				//DUMMY_STRUCT(2);
				//USHORT              ATActTime;  				// AT条动多少次，初始为0
				FileIndex			SYFileIndex;
				FileIndex			MSFileIndex;
				FileIndex			ASFileIndex;				// 0x14
				DUMMY_STRUCT(0x1B8);
				// 0x1D0
				ED6_STATUS			StatusBasic;    			// normal难度基础值
				ED6_STATUS			StatusSum;					// 算上难度、装备、回路
				USHORT				MoveSPD;					// 移动速度，我方人员也从ms文件中读取
				USHORT				MoveAfterAttack;			// 从ms文件中读取
				CONDITION           Condition[29];
				DUMMY_STRUCT(24);
				int					AT;
				int					AT2;
				DUMMY_STRUCT(6);
				USHORT				AIType;
				DUMMY_STRUCT(2);
				ushort				DropIndex[2];               // 掉落物 物品代码
				ubyte				DropProbability[2];         // 掉落概率 
				ubyte				Sex; 
				DUMMY_STRUCT(1);
				ushort				Equip[5];                   // 装备
				ushort				Orb[6];                     // 回路   7个 
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
				ubyte				Sepith[7];                  // 掉落耀晶片
				FSkip(1);
				ushort				EXPGet;                     // 战斗胜利 结算经验前暂存
				FSkip(2);
				ushort				ArtsAttResistance[4];       // 七曜属性有效率
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
				USHORT              SoldierNo;                  // 我方0-7，敌方8-15，支援16-19，待分身20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
				USHORT				Flags;                      // 10 敌方 40 己方 ...
				BYTE				DeathFlags;                 // 02 不参与战场胜利判定 04 死后留在战场上
				BYTE				UnderAttackFlags;           // 08 Resist ATDelay 02 不被击退 01 被攻击不转身(3D)  10 无法被攻击到  20 强制miss
                */
				DUMMY_STRUCT(4);
				USHORT              CharacterIndex;
				//DUMMY_STRUCT(2);
				//USHORT              ATActTime;  				// AT条动多少次，初始为0
				FileIndex			SYFileIndex;
				FileIndex			MSFileIndex;
				FileIndex			ASFileIndex;				// 0x14
				DUMMY_STRUCT(0x214);
				// 0x22C
				ED6_STATUS			StatusBasic;    			// normal难度基础值
				ED6_STATUS			StatusSum;					// 算上难度、装备、回路
				USHORT				MoveSPD;					// 移动速度，我方人员也从ms文件中读取
				USHORT				MoveAfterAttack;			// 从ms文件中读取
				CONDITION           Condition[31];
				DUMMY_STRUCT(24);
				int					AT;
				int					AT2;
				DUMMY_STRUCT(6);
				USHORT				AIType;
				ushort				DropIndex[2];               // 掉落物 物品代码
				ubyte				DropProbability[2];         // 掉落概率 
				ubyte				Sex; 
				DUMMY_STRUCT(1);
				ushort				Equip[5];                   // 装备
				ushort				Orb[7];                     // 回路   7个 
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
				ubyte				Sepith[7];                  // 掉落耀晶片
				FSkip(1);
				ushort				EXPGet;                     // 战斗胜利 结算经验前暂存
				FSkip(2);
				ushort				ArtsAttResistance[4];       // 七曜属性有效率
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
				USHORT              SoldierNo;                  // 我方0-7，敌方8-15，支援16-19，待分身20-21
                USHORT              RoleFlag;
                USHORT              HitFlag;
                /*
				USHORT				Flags;                      // 10 敌方 40 己方 ...
				BYTE				DeathFlags;                 // 02 不参与战场胜利判定 04 死后留在战场上
				BYTE				UnderAttackFlags;           // 08 Resist ATDelay 02 不被击退 01 被攻击不转身(3D)  10 无法被攻击到  20 强制miss
                */
				DUMMY_STRUCT(4);
				USHORT              CharacterIndex;
				DUMMY_STRUCT(2);
				USHORT              ATActTime;  				// AT条动多少次，初始为0
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
				ED6_STATUS			StatusBasic;    			// normal难度基础值
				ED6_STATUS			StatusSum;					// 算上难度、装备、回路
				USHORT				MoveSPD;					// 移动速度，我方人员也从ms文件中读取
				USHORT				MoveAfterAttack;			// 从ms文件中读取
				CONDITION           Condition[32];
				DUMMY_STRUCT(24);
				int					AT;
				int					AT2;
				DUMMY_STRUCT(4);
				USHORT				AIType;
				ushort				DropIndex[2];               // 掉落物 物品代码
				ubyte				DropProbability[2];         // 掉落概率   
				ubyte				Sex; 
				DUMMY_STRUCT(1);
				ushort				Equip[5];                   // 装备
				ushort				Orb[7];                     // 回路   7个 
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
				ubyte				Sepith[7];                  // 掉落耀晶片
				FSkip(2);
				ushort				EXPGet;                     // 战斗胜利 结算经验前暂存
				FSkip(2);
				ushort				ArtsAttResistance[7];       // 七曜属性有效率
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
		ushort				RNG;		// 内存中改StatusBasic中的才有效
		DUMMY_STRUCT(0x2);
		RESISTANCE          condition;  // RESISTANCE类型，进程中有效; 战斗外包括存档中 DEATH有效
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
				USHORT              SoldierNo;                  // 我方0-7，敌方8-15，支援16-19，待分身20-21
				USHORT				Flags;                      // 10 敌方 40 己方 ...
				BYTE				DeathFlags;                 // 02 不参与战场胜利判定 04 死后留在战场上
				BYTE				UnderAttackFlags;           // 08 Resist ATDelay 02 不被击退 01 被攻击不转身(3D)  10 无法被攻击到  20 强制miss
				DUMMY_STRUCT(4);
				USHORT              CharacterIndex;
				DUMMY_STRUCT(2);
				USHORT              ATActTime;  				// AT条动多少次，初始为0
				FileIndex			SYFileIndex;
				FileIndex			MSFileIndex;
				FileIndex			ASFileIndex;				// 0x18
				DUMMY_STRUCT(0x156);
				USHORT				wActType;
				DUMMY_STRUCT(0xA);
				USHORT				wUseMagic;
				DUMMY_STRUCT(0xB4);
				//DUMMY_STRUCT(0x218);
				ED6_STATUS			StatusBasic;    			// normal难度基础值
				ED6_STATUS			StatusSum;					// 算上难度、装备、回路

				ushort				MoveSPD;                // 移动速度，我方人员也从ms文件中读取
				ushort				MoveAfterAttack;        // 从ms文件中读取
				CONDITION			Condition[32];
				DUMMY_STRUCT(24);
				int				AT;
				int				AT2;    // AiTime08用
				
				ushort				AIType;
				ushort				EXPGet;                     // 战斗胜利 结算经验前暂存
				ushort				DropIndex[2];                 // 掉落物 物品代码
				//ushort				DropIndex2;
				ubyte				DropProbability[2];           // 掉落概率
				//ubyte				DropProbability2;   
				ubyte				Sex; 
				ubyte				DisplayHighLevelArtsAttResistance;      
				// 是否开启时空幻有效率显示，0/8 不开启 1/9 开启，应该是标志位，可能含有更多信息
				ushort				Equip[5];                   // 装备
				ushort				Orb[7];                     // 回路   7个 
				
				ED7_AI_INFO			NormalAttack;
				ED7_AI_INFO			Arts[80];
				ED7_AI_INFO			Craft[10];
				ED7_AI_INFO			SCraft[5];
				ED7_AI_INFO			SupportCraft[3]; 
				CraftLastUsed_INFO	CraftLastUsed;
				BYTE				fleeParameter[4];
				ED7_CRAFT_INFO		CraftInf[16];
				ED7_CRAFT_INTRO		CraftIntro[16];
				
				ubyte				Sepith[7];                  // 掉落耀晶片
				FSkip(3);
				ushort				ArtsAttResistance[7];       // 七曜属性有效率
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