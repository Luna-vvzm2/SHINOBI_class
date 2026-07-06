#include "SaveData.h"
#include <cstdio>
#include <ctime>
#include <iostream>

// セーブ機能
bool FileSaveData(const SaveData& data)
{
	FILE* fp = nullptr;
	errno_t err;

	printf("[SaveData] データをセーブします...\n");

	// ファイルをバイナリ書き込みモードで開く
	err = fopen_s(&fp, SAVE_FILE_PATH, "wb");

	if (err == 0 && fp != nullptr)
	{
		// ファイルに構造体を書き込む
		size_t written = fwrite(&data, sizeof(SaveData), 1, fp);
		fclose(fp);

		if (written == 1)
		{
			printf("[SaveData] セーブ完了！\n");
			return true;
		}
		else
		{
			printf("[SaveData] エラー: ファイルへの書き込みに失敗しました\n");
			return false;
		}
	}
	else
	{
		printf("[SaveData] エラー: ファイルをオープンできません\n");
		return false;
	}
}

// ロード機能
bool FileLoadData(SaveData& data)
{
	FILE* fp = nullptr;
	errno_t err;

	printf("[SaveData] データをロードします...\n");

	// ファイルをバイナリ読み込みモードで開く
	err = fopen_s(&fp, SAVE_FILE_PATH, "rb");

	if (err == 0 && fp != nullptr)
	{
		// ファイルから構造体に読み込む
		size_t readCount = fread(&data, sizeof(SaveData), 1, fp);
		fclose(fp);

		if (readCount == 1)
		{
			printf("[SaveData] ロード完了！\n");
			printf("  ステージ: %d\n", data.currentStage);
			printf("  HP: %d / %d\n", data.playerCurrentHp, data.playerMaxHp);
			printf("  所持金: %d\n", data.money);
			return true;
		}
		else
		{
			printf("[SaveData] エラー: ファイルの読み込みに失敗しました\n");
			return false;
		}
	}
	else
	{
		printf("[SaveData] エラー: セーブファイルが見つかりません\n");
		return false;
	}
}

// データ削除機能
bool FileDeleteData()
{
	if (remove(SAVE_FILE_PATH) == 0)
	{
		printf("[SaveData] セーブデータを削除しました\n");
		return true;
	}
	else
	{
		printf("[SaveData] エラー: セーブデータの削除に失敗しました\n");
		return false;
	}
}

// セーブデータ存在確認
bool FileDataExists()
{
	FILE* fp = nullptr;
	errno_t err = fopen_s(&fp, SAVE_FILE_PATH, "rb");

	if (err == 0 && fp != nullptr)
	{
		fclose(fp);
		return true;
	}

	return false;
}