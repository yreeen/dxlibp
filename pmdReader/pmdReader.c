#include "pmdReader.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
sPmdFile* PmdPerse(const char *filename)
{
	unsigned int i,j;
	sPmdFile *pmd;
	FILE *fp;
	pmd = malloc(sizeof(sPmdFile));
	if(!pmd)return NULL;

	memset(pmd,0,sizeof(sPmdFile));

	//ファイルオープン
	fp = fopen(filename,"rb");
	if(!fp)return NULL;
	
	//PMDファイルヘッダ読み出し
	fread(pmd->header.magic,1,3,fp);
	fread(&pmd->header.version,4,1,fp);
	fread(pmd->header.model_name,1,20,fp);
	fread(pmd->header.comment,1,256,fp);

	//頂点読み出し
	fread(&pmd->vertexNum,4,1,fp);
	if(pmd->vertexNum)
	{
		pmd->vertex = malloc(sizeof(sPmdVertex) * pmd->vertexNum);
		if(!pmd->vertex)goto err;
		for(i = 0;i < pmd->vertexNum;++i)
		{
			fread(pmd->vertex[i].pos,4,3,fp);
			fread(pmd->vertex[i].normal_vec,4,3,fp);
			fread(pmd->vertex[i].uv,4,2,fp);
			fread(pmd->vertex[i].bone_num,2,2,fp);
			fread(&pmd->vertex[i].bone_weight,1,1,fp);
			fread(&pmd->vertex[i].edge_flag,1,1,fp);
		}
	}
	
	//面読み出し
	fread(&pmd->faceNum,4,1,fp);
	if(pmd->faceNum % 3)goto err;
	pmd->faceNum /= 3;
	if(pmd->faceNum)
	{
		pmd->face = malloc(sizeof(sPmdFace) * pmd->faceNum);
		if(!pmd->face)goto err;
		for(i = 0;i < pmd->faceNum;++i)
			fread(pmd->face[i].vertexIndex,2,3,fp);
	}

	//マテリアル読み出し
	fread(&pmd->materialNum,4,1,fp);
	if(pmd->materialNum)
	{
		pmd->material = malloc(sizeof(sPmdMaterial) * pmd->materialNum);
		if(!pmd->material)goto err;
		for(i = 0;i < pmd->materialNum;++i)
		{
			fread(pmd->material[i].diffuse_color,4,3,fp);
			fread(&pmd->material[i].alpha,4,1,fp);
			fread(&pmd->material[i].specularity,4,1,fp);
			fread(pmd->material[i].specular_color,4,3,fp);
			fread(pmd->material[i].mirror_color,4,3,fp);
			fread(&pmd->material[i].toon_index,1,1,fp);
			fread(&pmd->material[i].edge_flag,1,1,fp);
			fread(&pmd->material[i].face_vert_count,4,1,fp);
			fread(pmd->material[i].texture_file_name,1,20,fp);
			pmd->material[i].texture_file_name[20] = '\0';
		}
	}

	//ボーン読み出し
	fread(&pmd->boneNum,2,1,fp);
	if(pmd->boneNum)
	{
		pmd->bone = malloc(sizeof(sPmdBone) * pmd->boneNum);
		if(!pmd->bone)goto err;
		for(i = 0;i < pmd->boneNum;++i)
		{
			fread(pmd->bone[i].bone_name,1,20,fp);
			fread(&pmd->bone[i].parent_bone_index,2,1,fp);
			fread(&pmd->bone[i].tail_pos_bone_index,2,1,fp);
			fread(&pmd->bone[i].bone_type,1,1,fp);
			fread(&pmd->bone[i].ik_parent_bone_index,2,1,fp);
			fread(pmd->bone[i].bone_head_pos,4,3,fp);
		}
	}
	
	//IKデータ読み出し
	fread(&pmd->ikDataNum,2,1,fp);
	if(pmd->ikDataNum)
	{
		pmd->ikData = malloc(sizeof(sPmdIkData) * pmd->ikDataNum);
		if(!pmd->ikData)goto err;
		memset(pmd->ikData,0,sizeof(sPmdIkData) * pmd->ikDataNum);
		for(i = 0;i < pmd->ikDataNum;++i)
		{
			fread(&pmd->ikData[i].ik_bone_index,2,1,fp);
			fread(&pmd->ikData[i].ik_target_bone_index,2,1,fp);
			fread(&pmd->ikData[i].ik_chain_length,1,1,fp);
			fread(&pmd->ikData[i].iterations,2,1,fp);
			fread(&pmd->ikData[i].control_weight,4,1,fp);
			if(pmd->ikData[i].ik_chain_length)
			{
				pmd->ikData[i].ik_child_bone_index = malloc(2 * pmd->ikData[i].ik_chain_length);
				if(!pmd->ikData[i].ik_target_bone_index)goto err;
				fread(pmd->ikData[i].ik_child_bone_index,2,pmd->ikData[i].ik_chain_length,fp);
			}
		}
	}


	//表情読み出し
	fread(&pmd->skinNum,2,1,fp);
	if(pmd->skinNum)
	{
		pmd->skin = malloc(sizeof(sPmdSkin) * pmd->skinNum);
		if(!pmd->skin)goto err;
		memset(pmd->skin,0,sizeof(sPmdSkin) * pmd->skinNum);
		for(i = 0;i < pmd->skinNum;++i)
		{
			fread(pmd->skin[i].skin_name,1,20,fp);
			fread(&pmd->skin[i].skin_vert_count,4,1,fp);
			fread(&pmd->skin[i].skin_type,1,1,fp);
			if(pmd->skin[i].skin_vert_count)
			{
				pmd->skin[i].skin_vert_data = malloc(16 * pmd->skin[i].skin_vert_count);
				if(!pmd->skin[i].skin_vert_data)goto err;
				for(j = 0;j < pmd->skin[i].skin_vert_count;++j)
				{
					fread(&pmd->skin[i].skin_vert_data[j].skin_vert_index,4,1,fp);
					fread(pmd->skin[i].skin_vert_data[j].skin_vert_pos,4,3,fp);
				}
			}
		}
	}


	//表情枠用表示リスト読み出し
	fread(&pmd->skinIndexNum,1,1,fp);
	if(pmd->skinIndexNum)
	{
		pmd->skinIndex = malloc(2 * pmd->skinIndexNum);
		if(!pmd->skinIndex)goto err;
		fread(pmd->skinIndex,2,pmd->skinIndexNum,fp);
	}

	//ボーン枠用枠名読み出し
	fread(&pmd->boneDispNameNum,1,1,fp);
	if(pmd->boneDispNameNum)
	{
		pmd->boneDispName = malloc(sizeof(sPmdBoneDispName) * pmd->boneDispNameNum);
		if(!pmd->boneDispName)goto err;
		for(i = 0;i < pmd->boneDispNameNum;++i)
		{
			fread(pmd->boneDispName[i].name,50,1,fp);
			for(j = 0;j < 50;++j)
				if(pmd->boneDispName[i].name[j] == 0)
				{
					if(pmd->boneDispName[i].name[j - 1] == 0x0a)
						pmd->boneDispName[i].name[j - 1] = '\0';
					break;
				}
		}
	}

	//ボーン枠用表示リスト読み出し
	fread(&pmd->boneDispListNum,4,1,fp);
	if(pmd->boneDispListNum)
	{
		pmd->boneDispList = malloc(sizeof(sPmdBoneDispList) * pmd->boneDispListNum);
		if(!pmd->boneDispList)goto err;
		for(i = 0;i < pmd->boneDispListNum;++i)
		{
			fread(&pmd->boneDispList[i].bone_index,2,1,fp);
			fread(&pmd->boneDispList[i].bone_disp_frame_index,1,1,fp);
		}
	}

	//英名拡張判定
	if(fread(&pmd->englishExpansionFlag,1,1,fp) == 0)goto end;
	if(pmd->englishExpansionFlag != 1)goto end;
	
	//英名ヘッダ読み込み
	fread(pmd->englishHeader.model_name_eg,1,20,fp);
	fread(pmd->englishHeader.comment_eg,1,256,fp);

	//英ボーン名読み込み
	if(pmd->boneNum)
	{
		pmd->englishBoneName = malloc(sizeof(sPmdEnglishBoneName) * pmd->boneNum);
		if(!pmd->englishBoneName)goto err;
		for(i = 0;i < pmd->boneNum;++i)
			fread(pmd->englishBoneName[i].name,20,1,fp);
	}

	//英スキン名読み込み
	if(pmd->skinNum >= 2)
	{
		pmd->englishSkinName = malloc(sizeof(sPmdEnglishSkinName) * (pmd->skinNum - 1));
		if(!pmd->englishSkinName)goto err;
		for(i = 0;i < pmd->skinNum - 1;++i)
			fread(pmd->englishSkinName[i].name,20,1,fp);
	}

	//英ボーン枠用枠名読み出し
	if(pmd->boneDispNameNum)
	{
		pmd->englishBoneDispName = malloc(sizeof(sPmdEnglishBoneDispName) * pmd->boneDispNameNum);
		if(!pmd->englishBoneDispName)goto err;
		for(i = 0;i < pmd->boneDispNameNum;++i)
			fread(pmd->englishBoneDispName[i].name,50,1,fp);
	}

	//トゥーンテクスチャファイル名読み込み
	for(i = 0;i < 10;++i)
		fread(pmd->toonFileName[i].name,1,100,fp);

	//剛体データ読み込み
	fread(&pmd->rigidBodyNum,4,1,fp);
	if(pmd->rigidBodyNum)
	{
		pmd->rigidBody = malloc(sizeof(sPmdRigidBody) * pmd->rigidBodyNum);
		if(!pmd->rigidBody)goto err;
		for(i = 0;i < pmd->rigidBodyNum;++i)
		{
			fread(&pmd->rigidBody[i].rigidbody_name,20,1,fp);
			fread(&pmd->rigidBody[i].rigidbody_rel_bone_index,2,1,fp);
			fread(&pmd->rigidBody[i].rigidbody_group_index,1,1,fp);
			fread(&pmd->rigidBody[i].rigidbody_group_target,2,1,fp);
			fread(&pmd->rigidBody[i].shape_type,1,1,fp);
			fread(&pmd->rigidBody[i].shape_w,4,1,fp);
			fread(&pmd->rigidBody[i].shape_h,4,1,fp);
			fread(&pmd->rigidBody[i].shape_d,4,1,fp);
			fread(pmd->rigidBody[i].pos_pos,4,3,fp);
			fread(pmd->rigidBody[i].pos_rot,4,3,fp);
			fread(&pmd->rigidBody[i].rigidbody_weight,4,1,fp);
			fread(&pmd->rigidBody[i].rigidbody_pos_dim,4,1,fp);
			fread(&pmd->rigidBody[i].rigidbody_rot_dim,4,1,fp);
			fread(&pmd->rigidBody[i].rigidbody_recoil,4,1,fp);
			fread(&pmd->rigidBody[i].rigidbody_friction,4,1,fp);
			fread(&pmd->rigidBody[i].rigidbody_type,1,1,fp);
		}
	}

	//ジョイント読み込み
	fread(&pmd->jointNum,4,1,fp);
	if(pmd->jointNum)
	{
		pmd->joint = malloc(sizeof(sPmdJoint) * pmd->jointNum);
		if(!pmd->joint)goto err;
		for(i = 0;i < pmd->jointNum;++i)
		{
			fread(pmd->joint[i].joint_name,1,20,fp);
			fread(&pmd->joint[i].joint_rigidbody_a,4,1,fp);
			fread(&pmd->joint[i].joint_rigidbody_b,4,1,fp);
			fread(pmd->joint[i].joint_pos,4,3,fp);
			fread(pmd->joint[i].joint_rot,4,3,fp);
			fread(pmd->joint[i].constrain_pos_1,4,3,fp);
			fread(pmd->joint[i].constrain_pos_2,4,3,fp);
			fread(pmd->joint[i].constrain_rot_1,4,3,fp);
			fread(pmd->joint[i].constrain_rot_2,4,3,fp);
			fread(pmd->joint[i].spring_pos,4,3,fp);
			fread(pmd->joint[i].spring_rot,4,3,fp);
		}
	}
end:
	//ファイルクローズ
	fclose(fp);
	return pmd;
err:
	fclose(fp);
	PmdDestruct(pmd);
	return NULL;
}

void PmdDestruct(sPmdFile *pmd)
{
	int i;
	free(pmd->vertex);
	free(pmd->face);
	free(pmd->material);
	free(pmd->bone);
	if(pmd->ikData)
	{
		for(i = 0;i < pmd->ikDataNum;++i)
			free(pmd->ikData[i].ik_child_bone_index);
	}
	free(pmd->ikData);
	if(pmd->skin)
	{
		for(i = 0;i < pmd->skinNum;++i)
			free(pmd->skin[i].skin_vert_data);
	}
	free(pmd->skin);
	free(pmd->skinIndex);
	free(pmd->boneDispName);
	free(pmd->boneDispList);
	free(pmd->englishBoneName);
	free(pmd->englishSkinName);
	free(pmd->englishBoneDispName);
	free(pmd->rigidBody);
	free(pmd->joint);
	memset(pmd,0,sizeof(sPmdFile));
	free(pmd);
}