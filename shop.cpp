#include "Shop.h"
#include <DxLib.h>
#include <math.h>

Shop::Shop() {
    // コンストラクタでアイテムの初期データをセット
    m_items[0] = { "苦無連 (x5)", "炸裂式手裏剣。敵・味方を問わず危険。", 100, 0, -1, false };
    m_items[1] = { "疔と宝", "の力を20ターン回復する。いやし。", 50, 1, -1, false };

    m_money = 500;
    m_kunai = 0;

    Init();
}

Shop::~Shop() {
    // 画像ハンドルをマネージしている場合は、ここで破棄 (DeleteGraphなど)
}

void Shop::Init() {
    m_state = ShopState::Closed;
    m_selectedItem = 0;
    m_animeTimer = 0.0f;
    m_enterPressTime = 0;
    m_purchaseSuccessFlash = 0;
    m_itemAngle = 0.0f;

    m_prevW = 0;
    m_prevS = 0;
    m_prevUp = 0;
    m_prevDown = 0;
    m_prevEsc = 0;

    for (int i = 0; i < MAX_ITEMS; i++) {
        m_itemOffsets[i] = 0.0f;
    }
}

void Shop::Update() {
    // 入力の状態判定用変数（フレーム間の判定に使用）
    int nowW = CheckHitKey(KEY_INPUT_W);
    int nowS = CheckHitKey(KEY_INPUT_S);
    int nowUp = CheckHitKey(KEY_INPUT_UP);
    int nowDown = CheckHitKey(KEY_INPUT_DOWN);
    int nowEsc = CheckHitKey(KEY_INPUT_ESCAPE);
    int nowEnter = CheckHitKey(KEY_INPUT_RETURN);

    int mouseX, mouseY;
    GetMousePoint(&mouseX, &mouseY);
    int mouseInput = GetMouseInput();

    // --- ショップ開く ---
    if (m_state == ShopState::Closed && nowW && !m_prevW) {
        m_state = ShopState::Opening;
        m_animeTimer = 0.0f;
    }

    // --- カスタムの選択 ---
    if (m_state == ShopState::Open) {
        // 1. キーボード選択
        if ((nowUp && !m_prevUp) || (nowW && !m_prevW)) {
            m_selectedItem = (m_selectedItem - 1 + MAX_ITEMS) % MAX_ITEMS;
            m_enterPressTime = 0;
        }
        if ((nowDown && !m_prevDown) || (nowS && !m_prevS)) {
            m_selectedItem = (m_selectedItem + 1) % MAX_ITEMS;
            m_enterPressTime = 0;
        }

        // 2. マウス移動による選択判定
        int winW = 900, winH = 500;
        int x1 = 640 - winW / 2;
        int y1 = 360 - winH / 2;
        bool isMouseOnAnyItem = false;

        for (int i = 0; i < MAX_ITEMS; i++) {
            int itemY1 = y1 + 120 + i * 80;
            if (mouseX >= x1 + 30 && mouseX <= 640 + winW / 2 - 30 &&
                mouseY >= itemY1 && mouseY <= itemY1 + 60) {
                if (m_selectedItem != i) {
                    m_selectedItem = i;
                    m_enterPressTime = 0;
                }
                isMouseOnAnyItem = true;
            }
        }

        // 3. 購入ロジック
        bool isClicking = (isMouseOnAnyItem && (mouseInput & MOUSE_INPUT_LEFT));

        if (!m_items[m_selectedItem].isSoldOut && (nowEnter || isClicking)) {
            m_enterPressTime += 16;

            if (m_enterPressTime >= PURCHASE_HOLD_TIME) {
                if (m_money >= m_items[m_selectedItem].price) {
                    m_money -= m_items[m_selectedItem].price;
                    if (m_items[m_selectedItem].type == 0) m_kunai += 5;

                    m_items[m_selectedItem].isSoldOut = true;
                    m_purchaseSuccessFlash = 20;
                    m_enterPressTime = 0;
                }
                else {
                    m_itemOffsets[m_selectedItem] = 10.0f; // 購入失敗の揺れ
                    m_enterPressTime = 0;
                }
            }
        }
        else {
            m_enterPressTime = 0;
        }

        // 4. 閉じる
        if (nowEsc && !m_prevEsc) {
            m_state = ShopState::Closing;
        }
    }

    // --- アニメーション処理 ---
    if (m_state == ShopState::Opening) {
        m_animeTimer += 0.08f;
        if (m_animeTimer >= 1.0f) { m_animeTimer = 1.0f; m_state = ShopState::Open; }
    }
    if (m_state == ShopState::Closing) {
        m_animeTimer -= 0.1f;
        if (m_animeTimer <= 0.0f) { m_animeTimer = 0.0f; m_state = ShopState::Closed; }
    }

    m_itemAngle += 0.05f;
    if (m_purchaseSuccessFlash > 0) m_purchaseSuccessFlash--;
    for (int i = 0; i < MAX_ITEMS; i++) m_itemOffsets[i] *= 0.8f;

    // 前フレームの入力を保存
    m_prevW = nowW;
    m_prevS = nowS;
    m_prevUp = nowUp;
    m_prevDown = nowDown;
    m_prevEsc = nowEsc;
}

void Shop::Draw() {
    if (m_state == ShopState::Closed) return;

    // 背景暗転
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(180 * m_animeTimer));
    DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    float scale = m_animeTimer;
    int winW = (int)(900 * scale), winH = (int)(500 * scale);
    int x1 = 640 - winW / 2, y1 = 360 - winH / 2;
    int x2 = 640 + winW / 2, y2 = 360 + winH / 2;

    if (m_animeTimer > 0.1f) {
        DrawBox(x1, y1, x2, y2, GetColor(30, 30, 35), TRUE); // 背景
        DrawBox(x1, y1, x2, y2, GetColor(200, 180, 100), FALSE); // 枠

        if (m_state == ShopState::Open) {
            DrawFormatString(x1 + 40, y1 + 30, GetColor(255, 220, 100), "--- 商店 -SHINOBI SHOP- ---");
            DrawFormatString(x2 - 250, y1 + 30, GetColor(255, 255, 255), "所持金: %d 両", m_money);

            for (int i = 0; i < MAX_ITEMS; i++) {
                int iy = y1 + 120 + i * 80;
                float xOffset = (i == m_selectedItem) ? 30.0f : 0.0f;
                xOffset += m_itemOffsets[i] * sinf(GetNowCount() * 0.1f);

                int bgCol = m_items[i].isSoldOut ? GetColor(20, 20, 20) :
                    ((i == m_selectedItem) ? GetColor(70, 70, 100) : GetColor(45, 45, 50));

                DrawBox(x1 + 30 + (int)xOffset, iy, x2 - 30 + (int)xOffset, iy + 60, bgCol, TRUE);

                // アイコン描画
                if (m_items[i].imgHandle != -1) {
                    if (m_items[i].isSoldOut) SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
                    DrawRotaGraph(x1 + 80 + (int)xOffset, iy + 30,
                        (i == m_selectedItem ? 1.2f : 1.0f),
                        (i == m_selectedItem ? m_itemAngle : 0.0f), m_items[i].imgHandle, TRUE);
                    if (m_items[i].isSoldOut) SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
                }

                int color = m_items[i].isSoldOut ? GetColor(100, 100, 100) :
                    ((i == m_selectedItem) ? GetColor(255, 255, 255) : GetColor(150, 150, 150));

                if (m_items[i].isSoldOut) {
                    DrawFormatString(x1 + 130 + (int)xOffset, iy + 20, color, "--- SOLD OUT ---");
                }
                else {
                    DrawFormatString(x1 + 130 + (int)xOffset, iy + 20, color, "%s", m_items[i].name);
                    DrawFormatString(x2 - 180 + (int)xOffset, iy + 20, color, "%d 両", m_items[i].price);
                }
            }

            // 説明欄エリア
            DrawBox(x1 + 30, y2 - 120, x2 - 30, y2 - 30, GetColor(20, 20, 20), TRUE);
            const char* descText = m_items[m_selectedItem].isSoldOut ? "品切れだ。また来るのを待ってる。" : m_items[m_selectedItem].desc;
            DrawFormatString(x1 + 50, y2 - 100, GetColor(180, 180, 180), descText);

            // ゲージの描画
            if (m_enterPressTime > 0) {
                int barW = (200 * m_enterPressTime) / PURCHASE_HOLD_TIME;
                DrawBox(x1 + 150, y2 - 60, x1 + 150 + barW, y2 - 45, GetColor(0, 255, 100), TRUE);
                DrawBox(x1 + 150, y2 - 60, x1 + 150 + 200, y2 - 45, GetColor(100, 100, 100), FALSE);
            }

            // 購入成功エフェクト
            if (m_purchaseSuccessFlash > 0) {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_purchaseSuccessFlash * 10);
                DrawBox(x1, y1, x2, y2, GetColor(255, 255, 255), TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }
        }
    }
}
