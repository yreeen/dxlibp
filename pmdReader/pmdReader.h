/*
PMDファイル読み込みルーチンforWindows
このコードは以下のサイトの情報を元に作られています。
http://blog.goo.ne.jp/torisu_tetosuki/e/209ad341d3ece2b1b4df24abf619d6e4
*/


#ifndef PMDREADER_H__
#define PMDREADER_H__

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef _MSC_VER
typedef unsigned __int8		u8;
typedef unsigned __int16	u16;
typedef unsigned __int32	u32;
#endif // _MSC_VER
#else
#include <psptypes.h>
#endif

typedef struct sPmdHeader
{
	char	magic[3]; // "Pmd"
	float	version; // 00 00 80 3F == 1.00
	char	model_name[20];
	char	comment[256];
}sPmdHeader;

typedef struct sPmdVertex
{
	float	pos[3]; // x, y, z // 座標
	float	normal_vec[3]; // nx, ny, nz // 法線ベクトル
	float	uv[2]; // u, v // UV座標 // MMDは頂点UV
	u16		bone_num[2]; // ボーン番号1、番号2 // モデル変形(頂点移動)時に影響
	u8		bone_weight; // ボーン1に与える影響度 // min:0 max:100 // ボーン2への影響度は、(100 - bone_weight)
	u8		edge_flag; // 0:通常、1:エッジ無効 // エッジ(輪郭)が有効の場合
}sPmdVertex;

typedef struct sPmdFace
{
	u16 vertexIndex[3];//面を形成する頂点３つ
}sPmdFace;

typedef struct sPmdMaterial
{
	float diffuse_color[3]; // dr, dg, db // 減衰色
	float alpha;
	float specularity;
	float specular_color[3]; // sr, sg, sb // 光沢色
	float mirror_color[3]; // mr, mg, mb // 環境色(ambient)
	u8 toon_index; // toon??.bmp // 0.bmp:0xFF, 1(01).bmp:0x00 ・・・ 10.bmp:0x09
	u8 edge_flag; // 輪郭、影
	u32 face_vert_count; // 面頂点数 // インデックスに変換する場合は、材質0から順に加算
	char texture_file_name[21]; //テクスチャファイル名（PMDファイルは２０バイトしか保存しないが、0終端が無いことがあるので1増やしている）
}sPmdMaterial;

typedef struct sPmdBone
{
	char bone_name[20]; // ボーン名
	u16 parent_bone_index; // 親ボーン番号(ない場合は0xFFFF)
	u16 tail_pos_bone_index; // tail位置のボーン番号(チェーン末端の場合は0xFFFF) // 親：子は1：多なので、主に位置決め用
	u8 bone_type; // ボーンの種類
	u16 ik_parent_bone_index; // IKボーン番号(影響IKボーン。ない場合は0)
	float bone_head_pos[3]; // x, y, z // ボーンのヘッドの位置
}sPmdBone;

typedef struct sPmdIkData
{
	u16 ik_bone_index; // IKボーン番号
	u16 ik_target_bone_index; // IKターゲットボーン番号 // IKボーンが最初に接続するボーン
	u8 ik_chain_length; // IKチェーンの長さ(子の数)
	u16 iterations; // 再帰演算回数 // IK値1
	float control_weight; // IKの影響度 // IK値2
	u16 *ik_child_bone_index; // IK影響下のボーン番号
}sPmdIkData;

typedef struct sPmdSkinVertex
{
	u32 skin_vert_index; // 表情用の頂点の番号(頂点リストにある番号)
	union
	{
		float skin_vert_pos[3]; // x, y, z // 表情用の頂点の座標(頂点自体の座標)
		float skin_vert_pos_offset[3]; // x, y, z // 表情用の頂点の座標オフセット値(baseに対するオフセット)
	};
}sPmdSkinVertex;

typedef struct sPmdSkin
{
	char skin_name[20]; //　表情名
	u32 skin_vert_count; // 表情用の頂点数
	u8 skin_type; // 表情の種類 // 0：base、1：まゆ、2：目、3：リップ、4：その他
	sPmdSkinVertex *skin_vert_data; // 表情用の頂点のデータ(16Bytes/vert)
}sPmdSkin;

typedef struct sPmdBoneDispName
{
	char name[50];
}sPmdBoneDispName;

typedef struct sPmdBoneDispList
{
	u16 bone_index; // 枠用ボーン番号
	u8 bone_disp_frame_index; // 表示枠番号
}sPmdBoneDispList;

typedef struct sPmdEnglishHeader
{
	char model_name_eg[20]; // モデル名(英語)
	char comment_eg[256]; // コメント(英語)
}sPmdEnglishHeader;

typedef struct sPmdEnglishBoneName
{
	char name[20];
}sPmdEnglishBoneName;

typedef struct sPmdEnglishSkinName
{
	char name[20];
}sPmdEnglishSkinName;

typedef struct sPmdEnglishBoneDispName
{
	char name[50];
}sPmdEnglishBoneDispName;

typedef struct sPmdToonFileName
{
	char name[100];
}sPmdToonFileName;

typedef struct sPmdRigidBody
{
	char rigidbody_name[20]; // 諸データ：名称 // 頭
	u16 rigidbody_rel_bone_index; // 諸データ：関連ボーン番号 // 03 00 == 3 // 頭
	u8 rigidbody_group_index; // 諸データ：グループ // 00
	u16 rigidbody_group_target; // 諸データ：グループ：対象 // 0xFFFFとの差 // 38 FE
	u8 shape_type; // 形状：タイプ(0:球、1:箱、2:カプセル) // 00 // 球
	float shape_w; // 形状：半径(幅) // CD CC CC 3F // 1.6
	float shape_h; // 形状：高さ // CD CC CC 3D // 0.1
	float shape_d; // 形状：奥行 // CD CC CC 3D // 0.1
	float pos_pos[3]; // 位置：位置(x, y, z)
	float pos_rot[3]; // 位置：回転(rad(x), rad(y), rad(z))
	float rigidbody_weight; // 諸データ：質量 // 00 00 80 3F // 1.0
	float rigidbody_pos_dim; // 諸データ：移動減 // 00 00 00 00
	float rigidbody_rot_dim; // 諸データ：回転減 // 00 00 00 00
	float rigidbody_recoil; // 諸データ：反発力 // 00 00 00 00
	float rigidbody_friction; // 諸データ：摩擦力 // 00 00 00 00
	u8 rigidbody_type; // 諸データ：タイプ(0:Bone追従、1:物理演算、2:物理演算(Bone位置合せ)) // 00 // Bone追従
}sPmdRigidBody;

typedef struct sPmdJoint
{
	char joint_name[20]; // 諸データ：名称 // 右髪1
	u32 joint_rigidbody_a; // 諸データ：剛体A
	u32 joint_rigidbody_b; // 諸データ：剛体B
	float joint_pos[3]; // 諸データ：位置(x, y, z) // 諸データ：位置合せでも設定可
	float joint_rot[3]; // 諸データ：回転(rad(x), rad(y), rad(z))
	float constrain_pos_1[3]; // 制限：移動1(x, y, z)
	float constrain_pos_2[3]; // 制限：移動2(x, y, z)
	float constrain_rot_1[3]; // 制限：回転1(rad(x), rad(y), rad(z))
	float constrain_rot_2[3]; // 制限：回転2(rad(x), rad(y), rad(z))
	float spring_pos[3]; // ばね：移動(x, y, z)
	float spring_rot[3]; // ばね：回転(rad(x), rad(y), rad(z))
}sPmdJoint;

typedef struct sPmdFile
{
	sPmdHeader header;

	u32 vertexNum;
	sPmdVertex *vertex;

	u32 faceNum;
	sPmdFace *face;

	u32 materialNum;
	sPmdMaterial *material;

	u16 boneNum;
	sPmdBone *bone;

	u16 ikDataNum;
	sPmdIkData *ikData;

	u16 skinNum;
	sPmdSkin *skin;

	u8 skinIndexNum;
	u16 *skinIndex;

	u8 boneDispNameNum;
	sPmdBoneDispName *boneDispName;

	u16 boneDispListNum;
	sPmdBoneDispList *boneDispList;

	u8 englishExpansionFlag;
	sPmdEnglishHeader englishHeader;
	sPmdEnglishBoneName *englishBoneName;
	sPmdEnglishSkinName *englishSkinName;
	sPmdEnglishBoneDispName *englishBoneDispName;

	sPmdToonFileName toonFileName[10];

	u16 rigidBodyNum;
	sPmdRigidBody *rigidBody;

	u32 jointNum;
	sPmdJoint *joint;
}sPmdFile;

sPmdFile* PmdPerse(const char *filename);
void PmdDestruct(sPmdFile *pmd);

#ifdef __cplusplus
}
#endif


#endif
