#pragma once

// シンプの状態を表す列挙型
enum class ShopState {
    Closed,
    Opening,
    Open,
    Closing
};

// アイテムの構造体
struct ShopItem {
    const char* name;
    const char* desc;
    int price;
    int type;
    int imgHandle;
    bool isSoldOut;
};

class Shop {
public:
    Shop();
    ~Shop();

    void Init();
    void Update();
    void Draw();

    // シンプが開いているか（またはアニメーション中）を確認する関数
    bool IsOpen() const { return m_state != ShopState::Closed; }

    // ゲーム内変数へのアクセス
    int GetMoney() const { return m_money; }
    void SetMoney(int money) { m_money = money; }
    int GetKunai() const { return m_kunai; }
    void SetKunai(int kunai) { m_kunai = kunai; }

private:
    static const int MAX_ITEMS = 2;
    static const int PURCHASE_HOLD_TIME = 600;

    // クラス内で管理する状態
    ShopState m_state;
    int m_selectedItem;
    int m_enterPressTime;

    // アニメ用のタイマー変数
    float m_animeTimer;
    float m_itemOffsets[MAX_ITEMS];
    int m_purchaseSuccessFlash;
    float m_itemAngle;

    // アイテムデータ
    ShopItem m_items[MAX_ITEMS];

    // ゲーム内のお金とクナイ
    int m_money;
    int m_kunai;

    // 前フレームのキー入力状態
    int m_prevW;
    int m_prevS;
    int m_prevUp;
    int m_prevDown;
    int m_prevEsc;
};
