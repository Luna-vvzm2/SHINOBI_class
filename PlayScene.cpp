#define NOMINMAX
#include "PlayScene.h"
#include "Game.h"
#include "Renderer.h"
#include "Input.h"
#include "PlayerEntity.h"
#include "WhiteEnemyEntity.h"
#include "YellowEnemyEntity.h"
#include "ArrowEnemyEntity.h"
#include "HealerEnemyEntity.h"
#include "ArmorEnemyEntity.h"
#include "GunnerEnemyEntity.h"
#include "YoroiBossEntity.h"
#include "SekienkiBossEntity.h"
#include "HitEffect.h"
#include "GroundBlock.h"
#include "HPBarUI.h"
#include "ShurikenUI.h"
#include "BackGroundUI.h"
#include "TransformComponent.h"
#include "HPComponent.h"
#include "Camera.h"
#include "MoneyUI.h"
#include "EnemyHPBar.h"

#include <algorithm>

PlayScene::PlayScene(Game* game)
	: Scene(game),
	m_player(nullptr),
	m_effect(nullptr),
	m_camera(static_cast<float>(game->GetWidth()), static_cast<float>(game->GetHeight())),
	m_stageIndex(0),
	m_comboCount(0),
	m_currentStage(1)
{

}

bool PlayScene::Init() {
	m_isRunning = true;
	m_type = Type::Play;
	m_stageIndex = 0;
	
	// CSV からマップ読み込み
	if (!m_mapData.LoadStage("assets/maps/stage1")) {
		std::cerr << "マップ１読み込み失敗" << std::endl;
		return false;
	}
	if (!m_mapData.LoadStage("assets/maps/stage2")) {
		std::cerr << "マップ２読み込み失敗" << std::endl;
		return false;
	}
	if (!m_mapData.LoadStage("assets/maps/stage3")) {
		std::cerr << "マップ３読み込み失敗" << std::endl;
		return false;
	}
	m_mapData.tileSize = 104;

	StageData& stage = m_mapData.stages[m_stageIndex];

	// マップのタイルを配置する
	const float tileSize = static_cast<float>(m_mapData.tileSize);
	for (const auto& layer : stage.layers) {
		if (!layer.visible) continue;
		const Layer& mapLayer = stage.layers[0];

		for (int y = 0; y < stage.height; ++y) {
			for (int x = 0; x < stage.width; ++x) {
				int tileID = mapLayer.tiles[y * stage.width + x];
				if (tileID == 0) continue; // 空タイル

				Vector2d pos(x * tileSize, y * tileSize);

				// ここでタイルIDに応じてブロック生成
				// 例：GrassBlock と IceBlock を仮に切り替え
				switch (tileID) {
				case 1:
					AddActor(new GroundBlock(this, pos, Vector2d(tileSize, tileSize)));
					break;
				}
			}
		}

		const Layer& objLayer = stage.layers[1];

		for (int y = 0; y < stage.height; y++)
		{
			for (int x = 0; x < stage.width; x++)
			{
				int objID =
					objLayer.tiles[y * stage.width + x];

				Vector2d pos(x * tileSize, y * tileSize);

				switch (objID)
				{
				case 1:
				{
					WhiteEnemyEntity* enemy = new WhiteEnemyEntity(this, pos);
					AddActor(enemy);

					// HPBar を PlayScene 側で作成して登録（UI は AddUIActor で登録）
					EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
					hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
					hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
					hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
					hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
					hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
					hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
					AddUIActor(hpBar);
					m_enemyToHPBarMap[enemy] = hpBar;

					// ダメージで表示：HP が減ったとき表示する
					enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
						if (hpBar && newHP < oldHP) {
							hpBar->ShowFor(0.0f); // 表示継続時間は調整可
						}
					};
					// 敵の死亡時にバーを消す（簡易）
					enemy->GetHP()->OnDeath = [hpBar]() {
						if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
				} break;

				case 2:
				{
					YellowEnemyEntity* enemy = new YellowEnemyEntity(this, pos);
					AddActor(enemy);
					EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
					hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
					hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
					hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
					hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
					hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
					hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
					AddUIActor(hpBar);
					m_enemyToHPBarMap[enemy] = hpBar;

					// ダメージで表示：HP が減ったときだけ表示する（2秒）
					enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
						if (hpBar && newHP < oldHP) {
							hpBar->ShowFor(0.0f); // 表示継続時間は調整可
						}
					};
					// 敵の死亡時にバーを消す（簡易）
					enemy->GetHP()->OnDeath = [hpBar]() {
						if (hpBar) hpBar->SetState(Actor::State::Dead);
					};
				} break;

				case 3:
				{
					ArrowEnemyEntity* enemy = new ArrowEnemyEntity(this, pos);
					AddActor(enemy);
					EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
					hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
					hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
					hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
					hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
					hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
					hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
					AddUIActor(hpBar);
					m_enemyToHPBarMap[enemy] = hpBar;

					// ダメージで表示：HP が減ったときだけ表示する（2秒）
					enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
						if (hpBar && newHP < oldHP) {
							hpBar->ShowFor(0.0f); // 表示継続時間は調整可
						}
						};
					// 敵の死亡時にバーを消す（簡易）
					enemy->GetHP()->OnDeath = [hpBar]() {
						if (hpBar) hpBar->SetState(Actor::State::Dead);
						};
				} break;

				case 4:
				{
					HealerEnemyEntity* enemy = new HealerEnemyEntity(this, pos);
					AddActor(enemy);
					EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
					hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
					hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
					hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
					hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
					hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
					hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
					AddUIActor(hpBar);
					m_enemyToHPBarMap[enemy] = hpBar;

					// ダメージで表示：HP が減ったときだけ表示する（2秒）
					enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
						if (hpBar && newHP < oldHP) {
							hpBar->ShowFor(0.0f); // 表示継続時間は調整可
						}
						};
					// 敵の死亡時にバーを消す（簡易）
					enemy->GetHP()->OnDeath = [hpBar]() {
						if (hpBar) hpBar->SetState(Actor::State::Dead);
						};
				} break;

				case 5:
				{
					ArmorEnemyEntity* enemy = new ArmorEnemyEntity(this, pos);
					AddActor(enemy);
					EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_armor.png");
					hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
					hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
					hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
					hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
					hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
					hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
					AddUIActor(hpBar);
					m_enemyToHPBarMap[enemy] = hpBar;

					// ダメージで表示：HP が減ったときだけ表示する（2秒）
					enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
						if (hpBar && newHP < oldHP) {
							hpBar->ShowFor(0.0f); // 表示継続時間は調整可
						}
						};
					// 敵の死亡時にバーを消す（簡易）
					enemy->GetHP()->OnDeath = [hpBar]() {
						if (hpBar) hpBar->SetState(Actor::State::Dead);
						};
				} break;

				case 6:
				{
					GunnerEnemyEntity* enemy = new GunnerEnemyEntity(this, pos);
					AddActor(enemy);
					EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
					hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
					hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
					hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
					hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
					hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
					hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
					AddUIActor(hpBar);
					m_enemyToHPBarMap[enemy] = hpBar;

					// ダメージで表示：HP が減ったときだけ表示する（2秒）
					enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
						if (hpBar && newHP < oldHP) {
							hpBar->ShowFor(0.0f); // 表示継続時間は調整可
						}
						};
					// 敵の死亡時にバーを消す（簡易）
					enemy->GetHP()->OnDeath = [hpBar]() {
						if (hpBar) hpBar->SetState(Actor::State::Dead);
						};
				} break;

				case 7:
				{
					YoroiBossEntity* enemy = new YoroiBossEntity(this, pos, Vector2d(192, 192));
					AddActor(enemy);
					EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
					hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
					hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
					hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
					hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
					hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
					hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
					AddUIActor(hpBar);
					m_enemyToHPBarMap[enemy] = hpBar;

					// ダメージで表示：HP が減ったときだけ表示する（2秒）
					enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
						if (hpBar && newHP < oldHP) {
							hpBar->ShowFor(0.0f); // 表示継続時間は調整可
						}
						};
					// 敵の死亡時にバーを消す（簡易）
					enemy->GetHP()->OnDeath = [hpBar]() {
						if (hpBar) hpBar->SetState(Actor::State::Dead);
						};
				} break;

				case 8:
				{
					SekienkiBossEntity* enemy = new SekienkiBossEntity(this, pos, Vector2d(192, 192));
					AddActor(enemy);
					EnemyHPBar* hpBar = new EnemyHPBar(this, enemy->GetHP(), "assets/images/uies/HP_enemy_black.png");
					hpBar->SetPosIsCenter(false);          // transform は左上座標を使う（デフォルト）
					hpBar->SetFrameOffset(80.0f, 50.0f);   // 少し上に出す
					hpBar->SetGaugeScale(0.8f, 0.05f);     // 幅を小さめ, 高さ半分
					hpBar->SetPadding(3.0f, 1.0f, 3.0f, 1.0f);
					hpBar->SetGaugeOffset(10.0f, 0.0f);// 枠はそのまま、ゲージを右に +3px、下に +1px 移動
					hpBar->SetGaugeOffset(0.0f, 0.0f);// ゲージを枠の中心より少し上に表示（上にずらすなら負の値）
					AddUIActor(hpBar);
					m_enemyToHPBarMap[enemy] = hpBar;

					// ダメージで表示：HP が減ったときだけ表示する（2秒）
					enemy->GetHP()->OnHPChanged = [hpBar](int newHP, int oldHP) {
						if (hpBar && newHP < oldHP) {
							hpBar->ShowFor(0.0f); // 表示継続時間は調整可
						}
						};
					// 敵の死亡時にバーを消す（簡易）
					enemy->GetHP()->OnDeath = [hpBar]() {
						if (hpBar) hpBar->SetState(Actor::State::Dead);
						};
				} break;

				default:
					break;
				} // switch
			} // for x
		} // for y
	}
	m_player = new PlayerEntity(this, Vector2d({200, 800}), Vector2d({192, 64
		}));
	AddActor(m_player);

	// ---- HP UI 作成 ----
	HPBarUI* hpBar = new HPBarUI(
		this,
		m_player->GetHP()
	);
	AddUIActor(hpBar);

	// プレイヤーの HP が減ったらコンボをリセットする
	if (m_player && m_player->GetHP()) {
		m_player->GetHP()->OnHPChanged = [this](int newHP, int oldHP) {
			if (newHP < oldHP) {
				m_comboCount = 0;
				// 必要ならログやサウンドを追加
				std::cout << "Combo reset due to player damage\n";
			}
			};
	}

	ShurikenUI* shuriken = new ShurikenUI(this, 18, 60);
	AddUIActor(shuriken);

	// プレイヤー所持金 UI（左上に表示）
	m_moneyUI = new MoneyUI(this, m_player, "assets/images/uies/money.png");
	m_moneyUI->SetPosition(20.0f, 110.0f);   // テキスト左上基準（スクリーン座標）
	m_moneyUI->SetImageSize(40.0f, 40.0f);   // 画像を 40x40 px に
	m_moneyUI->SetImageOffset(0.0f, 0.0f);   // 画像の相対オフセット（必要なら微調整）
	m_moneyUI->SetTextOffset(46.0f);         // 画像右側に数字を表示する距離
	m_moneyUI->SetAnchorTopRight(220.0f, 50.0f, -80.0f);// 右上に固定：右端から220px, 上から50px, 画像と数字の間隔を-60px にする
	AddUIActor(m_moneyUI);

	// プレイヤー金額変更時に MoneyUI を 3 秒表示（増えたときのみ）
	if (m_player) {
		m_player->OnMoneyChanged = [this](int newMoney, int oldMoney) {
			if (newMoney > oldMoney && m_moneyUI) {
				m_moneyUI->ShowFor(3.0f); // 3秒表示
			}
			};
	}

	BackGroundUI* back = new BackGroundUI(this, "assets/images/uies/bg.png");
	AddBackActor(back);

	float halfTile = m_mapData.tileSize * 0.5f;
	m_camera.SetTileHalfSize(Vector2d(halfTile, halfTile));

	// Camera のマップ範囲設定
	float mapW = (float)stage.width * m_mapData.tileSize;
	float mapH = (float)stage.height * m_mapData.tileSize;
	m_camera.SetBounds(Vector2d(0, 0), Vector2d(mapW, mapH));

	// Renderer に Camera をセット
	m_game->GetRenderer()->SetCamera(&m_camera);

	return true;
}

void PlayScene::Update(float deltaTime) {
	updateActors(m_backactors, deltaTime);
	updateActors(m_actors, deltaTime);
	updateActors(m_UIactors, deltaTime);

	// カメラ設定など（既にある処理）
	if (m_player) {
		Vector2d playerPos = m_player->GetComponent<TransformComponent>()->GetPosition();
		Vector2d camPos = playerPos;
		camPos.y -= 150;
		m_camera.SetCenter(camPos);
		m_camera.SetZoom(1.0f);
	}

	// 敵ごとのHPバー追従：World -> Screen using m_camera (PlayScene の m_camera)
	const Vector2d baroffset(-90.0f, -250.0f); // 敵の頭上に表示したければ負の Y オフセット。要調整。
	for (auto it = m_enemyToHPBarMap.begin(); it != m_enemyToHPBarMap.end(); ) {
		EnemyEntity* enemy = it->first;
		EnemyHPBar* hpBar = it->second;

		bool enemyStillPresent = std::find(m_actors.begin(), m_actors.end(), enemy) != m_actors.end();
		bool hpBarStillPresent = std::find(m_UIactors.begin(), m_UIactors.end(), hpBar) != m_UIactors.end();

		if (!enemyStillPresent || !hpBarStillPresent) {
			// どちらか存在しなければマップから除去（まれに両方既に削除済みの場合もある）
			it = m_enemyToHPBarMap.erase(it);
			continue;
		}

		// 敵が存在しない、または死亡していれば UI を消して map から削除
		if (!enemy || enemy->IsDead() || enemy->GetComponent<HPComponent>() == nullptr) {
			if (hpBar) { hpBar->SetState(Actor::State::Dead); }
			it = m_enemyToHPBarMap.erase(it);
			continue;
		}

		// 敵位置取得
		if (hpBar && hpBar->GetState() != Actor::State::Dead) {
			auto transform = enemy->GetComponent<TransformComponent>();
			if (transform) {
				Vector2d worldPos = transform->GetPosition() + baroffset;
				// PlayScene のカメラでワールド→スクリーン
				Vector2d screenPos = m_camera.WorldToScreen(worldPos);

				const float barWidth = 63.0f; // SetBarSize と合わせる
				hpBar->SetPosition(screenPos.x - barWidth * 0.5f, screenPos.y);

			}
		}
		
		++it;
	}
	RemoveDeadActors();

	if (m_shurikenUI && m_player)
	{
		m_shurikenUI->SetCount(m_player->GetShurikenCount());
	}
}

void PlayScene::Draw() {
	Renderer* renderer = m_game->GetRenderer();
	if (!renderer) return;


	// --- アクター描画 ---
	drawActors(m_backactors);
	drawActors(m_actors);
	drawActors(m_UIactors);

	std::vector<NumberInfo> comboInfo = {
		{ (float)m_comboCount, 0 }
	};

	// コンボ表示（m_comboCount が 1 以上なら表示）
	if (m_comboCount > 0) {
		const std::string& debugFont = m_game->GatDebugFont();
		// ここではフォントサイズを大きめ（例 48）で真ん中上に表示
		std::string comboText = std::to_string(m_comboCount) + " Combo";
		renderer->DrawTextL(Vector2d(20.0f, 120.0f), comboText, Color(255, 200, 32), debugFont, 32, false);
	}

	if (m_resultShown) {
		const std::string& debugFont = m_game->GatDebugFont();
		renderer->DrawNumberFormatW(
			Vector2d(m_game->GetWidth() / 2.4f, m_game->GetHeight() / 2.2f),
			Color(0, 0, 0),
			debugFont,
			32,
			"{0} Combo",
			comboInfo,
			false
		);
	}

	// --- デバッグ文字 ---
#ifdef _DEBUG
	const std::string& debugFont = m_game->GatDebugFont();
	renderer->DrawTextL(Vector2d(m_game->GetWidth() - 150.0f, 0), "PlayScene", Color(255, 64, 0), debugFont, 24, false);
#endif
}

void PlayScene::SpawnHitEffect(const Vector2d& pos) {
	m_effect = new HitEffect(this, pos, {32, 32});
	AddActor(m_effect);
	std::cout << "Spawned HitEffect at: " << pos.x << ", " << pos.y << std::endl;
	
}

void PlayScene::AddCombo() {
	m_comboCount++;
	// タイマーは廃止するため不要
	std::cout << "Combo: " << m_comboCount << std::endl;
}