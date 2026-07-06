#pragma once

#define SAVE_FILE_PATH "SaveData.bin"
#define FILE_LOAD 0
#define FILE_SAVE 1

// セーブ・ロード用の構造体
typedef struct
{
	// 仮の構造体です
	// 必要に応じて変数を追加してください
	int currentStage;		// 現在のステージ
	int playerMaxHp;		// プレイヤーの最大HP
	int playerCurrentHp;	// プレイヤーの現在HP
	int money;				// 所持金
	int coin;				// コイン数
	int kunai;				// 手裏剣数
	int playTime;			// プレイ時間（秒）
	int clearStages;		// クリアしたステージ数
}SaveData;

// セーブ・ロード機能のプロトタイプ宣言
bool FileSaveData(const SaveData& data);
bool FileLoadData(SaveData& data);
bool FileDeleteData();
bool FileDataExists();