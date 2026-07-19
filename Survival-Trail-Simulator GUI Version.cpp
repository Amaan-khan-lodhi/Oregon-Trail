
#include <raylib.h>
#include <vector>
#include <string>
#include <memory>
#include <random>
#include <fstream>
#include <algorithm>
 
using namespace std;
 
// ================= GAME CONSTANTS =================
namespace GameConstants {
    constexpr int MAX_HEALTH = 100;
    constexpr int MAX_FOOD = 100;
    constexpr int MAX_DISTANCE = 100;
    constexpr int MAX_INVENTORY = 10;
    constexpr int MAX_STICKS = 3;
    
    // Travel costs
    constexpr int TRAVEL_HEALTH_COST = 10;
    constexpr int TRAVEL_FOOD_COST = 10;
    constexpr int TRAVEL_DISTANCE_GAIN = 10;
    
    // Hunt gains
    constexpr int HUNT_HEALTH_COST = 15;
    constexpr int HUNT_FOOD_GAIN = 20;
    
    // Rest gains
    constexpr int REST_HEALTH_GAIN = 15;
    
    // Food/Money
    constexpr int FOOD_EAT_COST = 10;
    constexpr int HEALTH_EAT_GAIN = 15;
    constexpr int FOOD_SELL_AMOUNT = 10;
    constexpr int FOOD_SELL_PRICE = 12;
    
    // Tent
    constexpr int TENT_COST = 35;
    
    // Storm damage
    constexpr int STORM_DAMAGE_DAY_WITH_TENT = 5;
    constexpr int STORM_DAMAGE_DAY_NO_TENT = 20;
    constexpr int STORM_DAMAGE_NIGHT_NO_TENT = 35;
    constexpr int STORM_FOOD_LOSS = 10;
    constexpr int STORM_DISTANCE_LOSS = 5;
    
    // Wolf attack damage
    constexpr int WOLF_DAMAGE_DAY = 20;
    constexpr int WOLF_DAMAGE_NIGHT = 35;
    constexpr int WOLF_ESCAPE_DAMAGE_DAY = 10;
    constexpr int WOLF_ESCAPE_DAMAGE_NIGHT = 20;
    constexpr int WOLF_ESCAPE_FOOD_LOSS = 10;
    constexpr int WOLF_STICK_DAMAGE = 5;
    
    // Probabilities (0-100)
    constexpr int STICK_FIND_CHANCE = 25;
    constexpr int STORM_BASE_CHANCE = 15;
    constexpr int WOLF_BASE_CHANCE = 15;
    constexpr int WOLF_DAY_CHANCE_OFFSET = 15;
    constexpr int WOLF_NIGHT_CHANCE_OFFSET = 5;
    constexpr int STORM_DAY_CHANCE_OFFSET = 15;
}
 
// ================= RANDOM NUMBER GENERATOR =================
class RandomGenerator {
private:
    static mt19937 generator;
    static uniform_int_distribution<int> distribution;
    
public:
    static int getRandomInt(int min, int max) {
        uniform_int_distribution<int> dist(min, max);
        return dist(generator);
    }
    
    static bool checkProbability(int percentage) {
        return getRandomInt(0, 99) < percentage;
    }
};
 
mt19937 RandomGenerator::generator(random_device{}());
uniform_int_distribution<int> RandomGenerator::distribution(0, 99);
 
// ================= PLAYER =================
class Player {
private:
    int health;
    int food;
    int distance;
    int money;
    bool hasTent;
    vector<string> inventory;
    int sticksFound;
    static int totalPlayers;
    
    void clampValues() {
        health = max(0, min(health, GameConstants::MAX_HEALTH));
        food = max(0, min(food, GameConstants::MAX_FOOD));
        distance = max(0, distance);
    }
    
public:
    Player() : health(GameConstants::MAX_HEALTH), 
               food(GameConstants::MAX_FOOD), 
               distance(0), 
               money(0), 
               hasTent(false), 
               sticksFound(0) {
        totalPlayers++;
    }
    
    ~Player() { totalPlayers--; }
    
    void updateStats(int healthDelta, int foodDelta, int distanceDelta) {
        health += healthDelta;
        food += foodDelta;
        distance += distanceDelta;
        clampValues();
    }
    
    void addMoney(int amount) {
        money = max(0, money + amount);
    }
    
    void buildTent() {
        hasTent = true;
    }
    
    int getHealth() const { return health; }
    int getFood() const { return food; }
    int getDistance() const { return distance; }
    int getMoney() const { return money; }
    int getStickCount() const { return sticksFound; }
    bool hasTentBuilt() const { return hasTent; }
    size_t getInventorySize() const { return inventory.size(); }
    vector<string> getInventory() const { return inventory; }
    
    bool addItem(const string& item) {
        if (inventory.size() >= GameConstants::MAX_INVENTORY) {
            return false;
        }
        inventory.push_back(item);
        return true;
    }
    
    bool useItem(int index) {
        if (index < 0 || (size_t)index >= inventory.size()) {
            return false;
        }
        inventory.erase(inventory.begin() + index);
        return true;
    }
    
    bool hasWeapon() const {
        return find(inventory.begin(), inventory.end(), "Stick") != inventory.end();
    }
    
    bool giveStick() {
        if (sticksFound >= GameConstants::MAX_STICKS) {
            return false;
        }
        addItem("Stick");
        sticksFound++;
        return true;
    }
    
    bool useStick() {
        auto it = find(inventory.begin(), inventory.end(), "Stick");
        if (it != inventory.end()) {
            inventory.erase(it);
            return true;
        }
        return false;
    }
    
    bool eatFood() {
        if (food < GameConstants::FOOD_EAT_COST) {
            return false;
        }
        food -= GameConstants::FOOD_EAT_COST;
        health += GameConstants::HEALTH_EAT_GAIN;
        if (health > GameConstants::MAX_HEALTH) {
            health = GameConstants::MAX_HEALTH;
        }
        return true;
    }
    
    void saveToFile(const string& filename) {
        ofstream fout(filename);
        if (!fout) {
            return;
        }
        fout << health << " " << food << " " << distance << " " 
             << money << " " << hasTent << " " << sticksFound << " " 
             << inventory.size() << "\n";
        for (const auto& item : inventory) {
            fout << item << "\n";
        }
        fout.close();
    }
    
    bool loadFromFile(const string& filename) {
        ifstream fin(filename);
        if (!fin) {
            return false;
        }
        
        size_t itemCount;
        fin >> health >> food >> distance >> money >> hasTent >> sticksFound >> itemCount;
        fin.ignore();
        
        inventory.clear();
        for (size_t i = 0; i < itemCount; ++i) {
            string item;
            getline(fin, item);
            inventory.push_back(item);
        }
        fin.close();
        clampValues();
        return true;
    }
    
    static int getTotalPlayers() { return totalPlayers; }
};
 
int Player::totalPlayers = 0;
 
// ================= GUI BUTTON CLASS =================
struct Button {
    Rectangle rect;
    string text;
    Color color;
    Color hoverColor;
    bool isHovered;
    
    Button(float x, float y, float w, float h, const string& txt, Color col, Color hcol)
        : rect{x, y, w, h}, text(txt), color(col), hoverColor(hcol), isHovered(false) {}
    
    void update() {
        Vector2 mouse = GetMousePosition();
        isHovered = CheckCollisionPointRec(mouse, rect);
    }
    
    bool isClicked() {
        return isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }
    
    void draw(Font& font) {
        Color currentColor = isHovered ? hoverColor : color;
        
        DrawRectangleRec(rect, currentColor);
        Color borderColor = isHovered ? Color{255, 255, 255, 255} : Color{100, 100, 100, 255};
        DrawRectangleLinesEx(rect, 2, borderColor);
        
        Vector2 textSize = MeasureTextEx(font, text.c_str(), 14, 1);
        float textX = rect.x + (rect.width - textSize.x) / 2;
        float textY = rect.y + (rect.height - textSize.y) / 2;
        
        DrawTextEx(font, text.c_str(), {textX + 1, textY + 1}, 14, 2, Color{0, 0, 0, 100});
        DrawTextEx(font, text.c_str(), {textX, textY}, 14, 2, WHITE);
    }
};

// Forward declarations, defined later

void DrawStormCloud(int x, int y, float scale, unsigned char alpha);
// ================= VISUAL EFFECTS CLASS =================
class VisualEffects {
private:
    float stormOverlayAlpha = 0;
    float lightningTimer = 0;
    int rainIntensity = 0;
    float rainTimer = 0;  // ADD THIS
    
    float wolfFlashTimer = 0;
    Vector2 shakeOffset = {0, 0};
    
public:
    void triggerStorm(float duration) {
        stormOverlayAlpha = 150;
        rainIntensity = 40;
        rainTimer = 0;  // ADD THIS
    }
    
    void triggerWolfAttack(float duration) {
        wolfFlashTimer = duration;
    }
    
    void update(float deltaTime) {
        stormOverlayAlpha *= 0.97f;
        rainIntensity = (int)(stormOverlayAlpha / 150.0f * 40);
        rainTimer += deltaTime;  // ADD THIS
        
        wolfFlashTimer -= deltaTime;
        
        if (wolfFlashTimer > 0) {
            shakeOffset.x = (float)(rand() % 10 - 5) * 0.3f;
            shakeOffset.y = (float)(rand() % 10 - 5) * 0.3f;
        } else {
            shakeOffset = {0, 0};
        }
    }
    void drawStormEffects() {
    if (stormOverlayAlpha <= 1) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                 Color{50, 50, 80, (unsigned char)stormOverlayAlpha});

    // Drifting storm clouds
   unsigned char cloudAlpha = (unsigned char)min(255.0f, stormOverlayAlpha * 1.5f);
int driftX = (int)(rainTimer * 20) % (GetScreenWidth() + 300) - 150;

DrawStormCloud(driftX, 60, 1.0f, cloudAlpha);
DrawStormCloud(driftX + 250, 40, 1.2f, cloudAlpha);
DrawStormCloud(driftX + 500, 80, 0.9f, cloudAlpha);
DrawStormCloud(driftX + 750, 55, 1.3f, cloudAlpha);
DrawStormCloud(driftX - 250, 70, 1.1f, cloudAlpha);
DrawStormCloud(driftX - 500, 45, 0.8f, cloudAlpha);
DrawStormCloud(driftX, 60, 1.0f, cloudAlpha);
DrawStormCloud(driftX + 250, 40, 1.2f, cloudAlpha);
DrawStormCloud(driftX + 500, 80, 0.9f, cloudAlpha);
DrawStormCloud(driftX + 750, 55, 1.3f, cloudAlpha);
DrawStormCloud(driftX - 250, 70, 1.1f, cloudAlpha);
DrawStormCloud(driftX - 500, 45, 0.8f, cloudAlpha);
DrawStormCloud(driftX, 60, 1.0f, cloudAlpha);
DrawStormCloud(driftX + 850, 40, 1.2f, cloudAlpha);
DrawStormCloud(driftX + 1000, 80, 0.9f, cloudAlpha);
DrawStormCloud(driftX + 1250, 55, 1.3f, cloudAlpha);
DrawStormCloud(driftX - 1500, 70, 1.1f, cloudAlpha);
DrawStormCloud(driftX - 1750, 45, 0.8f, cloudAlpha);

        
        // Rain animation 
        for (int i = 0; i < rainIntensity; i++) {
            int rainX = ((int)(rainTimer * 100) + i * 50) % (GetScreenWidth() + 100);
            int rainY = ((int)(rainTimer * 200) + i * 30) % (GetScreenHeight() + 100);
            
            DrawLine(rainX, rainY, rainX + 15, rainY + 30,
                    Color{200, 200, 220, 200});
        }
        
        // Lightning flashes
        if (RandomGenerator::checkProbability(8)) {
            lightningTimer = 0.15f;
        }
        
        if (lightningTimer > 0) {
            int alpha = (int)(lightningTimer * 255 * 4);
            alpha = min(255, alpha);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                         Color{255, 255, 255, (unsigned char)alpha});
            lightningTimer -= GetFrameTime();
        }
    }
    
   void drawWolfEffects() {
    if (wolfFlashTimer <= 0) return;

    float intensity = wolfFlashTimer * 2;
    intensity = min(1.0f, intensity);

  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
             Color{255, 0, 0, (unsigned char)(intensity * 40)}); // was 80


    int eyeX = GetScreenWidth() / 2;
    int eyeY = 150;

    //subtle glow ring behind each eye
   DrawCircle(eyeX - 80, eyeY, 45, Color{255, 0, 0, (unsigned char)(intensity * 30)}); 
DrawCircle(eyeX + 80, eyeY, 45, Color{255, 0, 0, (unsigned char)(intensity * 30)});

DrawCircle(eyeX - 80, eyeY, 30, Color{255, 50, 50, 140}); // was 200
DrawCircle(eyeX + 80, eyeY, 30, Color{255, 50, 50, 140});

    DrawRectangleLinesEx({0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
                        10, Color{255, 0, 0, (unsigned char)(intensity * 150)});
}
    Vector2 getShakeOffset() const { return shakeOffset; }
    float getWolfTimer() const { return wolfFlashTimer; }
};
 
// ================= UI RENDERER (GUI ONLY) =================
class UIRenderer {
private:
    Font font;
    
public:
      UIRenderer() {
    font = LoadFont("Assets/Rough One/The Foregen Rough One.ttf");
}
    
    void drawStatsPanel(const Player& player) {
    float panelX = 20, panelY = 70;
    DrawRectangleRounded({panelX, panelY, 200, 350}, 0.08f, 6, Color{20, 45, 20, 255});

    DrawTextEx(font, "Status", {panelX + 14, panelY + 12}, 20, 1, Color{144, 238, 144, 255});

    float y = panelY + 42;
    auto drawBar = [&](const char* label, float percent, Color barColor, int value, int maxVal) {
        DrawTextEx(font, label, {panelX + 14, y}, 13, 1, barColor);
        DrawRectangle(panelX + 14, y + 15, 172, 10, Color{20, 20, 20, 150});
        DrawRectangle(panelX + 14, y + 15, 172 * percent, 10, barColor);
        string val = to_string(value);
        DrawTextEx(font, val.c_str(), {panelX + 178 - MeasureTextEx(font, val.c_str(), 11, 1).x, y + 15}, 11, 1, WHITE);
    };

    drawBar("Health", player.getHealth() / 100.0f, Color{255, 100, 100, 255}, player.getHealth(), 100);
    y += 34;
    drawBar("Food", player.getFood() / 100.0f, Color{255, 180, 0, 255}, player.getFood(), 100);
    y += 34;
    drawBar("Distance", player.getDistance() / 100.0f, Color{100, 200, 100, 255}, player.getDistance(), 100);

    // Money / Tent / Items / Sticks as a compact 2x2 grid
    float gy = y + 40;
    auto drawChip = [&](float cx, float cy, const char* label, const string& value, Color labelColor, Color valueColor) {
        DrawRectangleRounded({cx, cy, 84, 40}, 0.15f, 4, Color{35, 65, 35, 255});
        DrawTextEx(font, label, {cx + 8, cy + 5}, 10, 1, labelColor);
        DrawTextEx(font, value.c_str(), {cx + 8, cy + 20}, 14, 1, valueColor);
    };

    drawChip(panelX + 14, gy, "Money", "Rs" + to_string(player.getMoney()), Color{184, 134, 11, 255}, Color{218, 165, 32, 255});
    drawChip(panelX + 102, gy, "Tent", player.hasTentBuilt() ? "Built" : "None",
             Color{100, 200, 100, 255}, player.hasTentBuilt() ? Color{100, 255, 100, 255} : Color{255, 100, 100, 255});
    drawChip(panelX + 14, gy + 46, "Items", to_string(player.getInventorySize()) + "/10", Color{150, 150, 255, 255}, Color{200, 200, 255, 255});
    drawChip(panelX + 102, gy + 46, "Sticks", to_string(player.getStickCount()) + "/3", Color{200, 200, 100, 255}, Color{255, 255, 100, 255});
}
    void drawEventMessage(const string& message, float& duration, int screenWidth) {
    if (duration <= 0) return;

    Vector2 textSize = MeasureTextEx(font, message.c_str(), 16, 1);
    float boxWidth = min(650.0f, max(300.0f, textSize.x + 50));
    float boxX = (screenWidth - boxWidth) / 2;
    float boxY = 150; // sits below the storm cloud 

    DrawRectangleRounded({boxX, boxY, boxWidth, 44}, 0.5f, 8, Color{34, 102, 34, 230});
    DrawTextEx(font, message.c_str(), {boxX + (boxWidth - textSize.x) / 2, boxY + 14}, 16, 1, Color{200, 255, 200, 255});

    duration -= GetFrameTime();
}
    
    void drawGameOver(const string& message, const Player& player) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 200});
        
        Vector2 screenSize = {(float)GetScreenWidth(), (float)GetScreenHeight()};
        float boxWidth = 600;
        float boxHeight = 300;
        float boxX = (screenSize.x - boxWidth) / 2;
        float boxY = (screenSize.y - boxHeight) / 2;
        
        DrawRectangle(boxX, boxY, boxWidth, boxHeight, Color{40, 40, 40, 255});
        DrawRectangleLinesEx({boxX, boxY, boxWidth, boxHeight}, 4, YELLOW);
        
        DrawTextEx(font,message.c_str(),(Vector2){ boxX + 50, boxY + 50}, 28,2, YELLOW);
        
        string finalStats = "Final Distance: " + to_string(player.getDistance()) + "/100";
        DrawTextEx(font,finalStats.c_str(),(Vector2){ boxX + 50, boxY + 150}, 20,2, WHITE);
        
        DrawTextEx(font,"Close window to exit",(Vector2){ boxX + 100, boxY + 230}, 18,2, WHITE);
    }
};
 
// ================= ACTION SYSTEM =================
class Action {
public:
    virtual void execute(Player& player) = 0;
    virtual ~Action() = default;
};
 
class TravelAction : public Action {
public:
    void execute(Player& player) override {
        player.updateStats(-GameConstants::TRAVEL_HEALTH_COST, 
                          -GameConstants::TRAVEL_FOOD_COST, 
                          GameConstants::TRAVEL_DISTANCE_GAIN);
    }
};
 
class HuntAction : public Action {
public:
    void execute(Player& player) override {
        player.updateStats(-GameConstants::HUNT_HEALTH_COST, 
                          GameConstants::HUNT_FOOD_GAIN, 0);
    }
};
 
class RestAction : public Action {
public:
    void execute(Player& player) override {
        player.updateStats(GameConstants::REST_HEALTH_GAIN, 0, 0);
    }
};
 
class SellFoodAction : public Action {
public:
    void execute(Player& player) override {
        if (player.getFood() >= GameConstants::FOOD_SELL_AMOUNT) {
            player.updateStats(0, -GameConstants::FOOD_SELL_AMOUNT, 0);
            player.addMoney(GameConstants::FOOD_SELL_PRICE);
        }
    }
};
 
class BuildTentAction : public Action {
public:
    void execute(Player& player) override {
        if (player.getMoney() >= GameConstants::TENT_COST) {
            player.addMoney(-GameConstants::TENT_COST);
            player.buildTent();
        }
    }
};
 
class EatFoodAction : public Action {
public:
    void execute(Player& player) override {
        player.eatFood();
    }
};
 
// ================= ACTION FACTORY =================
class ActionFactory {
public:
    static unique_ptr<Action> createAction(int choice) {
        switch (choice) {
            case 1: return make_unique<TravelAction>();
            case 2: return make_unique<RestAction>();
            case 3: return make_unique<HuntAction>();
            case 4: return make_unique<SellFoodAction>();
            case 5: return make_unique<BuildTentAction>();
            case 6: return make_unique<EatFoodAction>();
            default: return nullptr;
        }
    }
};
 
// ================= GAME ENGINE =================
class Game {
private:
    Player player;
    UIRenderer uiRenderer;
    VisualEffects effects; 
    bool isRunning;
    bool isDay;
    int turnCount;
    string eventMessage;
    float eventTimer;
    bool gameOver;
    bool playerWon;
    string gameOverMessage;
    string lastAction = "";
    float actionAnimTimer = 0;


    
    void handleFindStick() {
        if (RandomGenerator::checkProbability(GameConstants::STICK_FIND_CHANCE)) {
            if (player.getStickCount() < GameConstants::MAX_STICKS) {
                player.giveStick();
                eventMessage = "You found a Stick!";
                eventTimer = 2.0f;
            }
        }
    }
    
    void handleStorm() {
        int chance = isDay ? 
            GameConstants::STORM_BASE_CHANCE + GameConstants::STORM_DAY_CHANCE_OFFSET :
            GameConstants::STORM_BASE_CHANCE;
            
        if (RandomGenerator::checkProbability(chance)) {
            if (!player.hasTentBuilt()) {
                int damage = isDay ? GameConstants::STORM_DAMAGE_DAY_NO_TENT : 
                                    GameConstants::STORM_DAMAGE_NIGHT_NO_TENT;
                player.updateStats(-damage, -GameConstants::STORM_FOOD_LOSS, 
                                 -GameConstants::STORM_DISTANCE_LOSS);
                                 effects.triggerStorm(2.5f);
                eventMessage = "STORM! You took " + to_string(damage) + " damage!";
            } else {
                player.updateStats(-GameConstants::STORM_DAMAGE_DAY_WITH_TENT, 0, 0);
                effects.triggerStorm(2.5f);
                eventMessage = "Storm hit but your tent protected you!";
            }
            eventTimer = 2.5f;
            
        }
    }
    
    void handleWolfAttack() {
        int chance = isDay ? 
            GameConstants::WOLF_BASE_CHANCE + GameConstants::WOLF_DAY_CHANCE_OFFSET :
            GameConstants::WOLF_BASE_CHANCE + GameConstants::WOLF_NIGHT_CHANCE_OFFSET;
            
        if (RandomGenerator::checkProbability(chance)) {
            if (player.hasWeapon()) {
                player.updateStats(-GameConstants::WOLF_STICK_DAMAGE, 0, 0);
                player.useStick();
                effects.triggerWolfAttack(1.5f);
                eventMessage = "Wolf attacked! You fought it off with your stick!";
            } else {
                int damage = isDay ? GameConstants::WOLF_DAMAGE_DAY : 
                                    GameConstants::WOLF_DAMAGE_NIGHT;
                player.updateStats(-damage, 0, 0);
                effects.triggerWolfAttack(2.0f);
                eventMessage = "WOLF! You took " + to_string(damage) + " damage!";
            }
            eventTimer = 2.5f;
        }
    }
    
    void checkGameStatus() {
        if (player.getHealth() <= 0) {
            gameOver = true;
            playerWon = false;
            gameOverMessage = "GAME OVER - You Died!";
        } else if (player.getDistance() >= GameConstants::MAX_DISTANCE) {
            gameOver = true;
            playerWon = true;
            gameOverMessage = "YOU WIN! Reached the end!";
        }
    }
    
public:
    Game() : isRunning(true), isDay(true), turnCount(0), eventTimer(0), 
             gameOver(false), playerWon(false) {}


    
    void updateEffects(float deltaTime) { effects.update(deltaTime); }
    VisualEffects& getEffects() { return effects; }
    float getWolfTimer() const { return effects.getWolfTimer(); }
    
    
    void updateTime() {
        turnCount++;
        if (turnCount % 2 == 0) {
            isDay = !isDay;
        }
    }
    
    void travel() {
        player.updateStats(-GameConstants::TRAVEL_HEALTH_COST, 
                          -GameConstants::TRAVEL_FOOD_COST, 
                          GameConstants::TRAVEL_DISTANCE_GAIN);
                          triggerActionAnim("travel");
        eventMessage = "You traveled 1.5 km";
        eventTimer = 1.5f;
        processNextTurn();
    }
    
    void hunt() {
        player.updateStats(-GameConstants::HUNT_HEALTH_COST, 
                          GameConstants::HUNT_FOOD_GAIN, 0);
                          triggerActionAnim("hunt");
        eventMessage = "You hunted and found food!";
        eventTimer = 1.5f;
        processNextTurn();
    }
    
    void rest() {
        player.updateStats(GameConstants::REST_HEALTH_GAIN, 0, 0);
        triggerActionAnim("rest");
        eventMessage = "You rested and recovered health";
        eventTimer = 1.5f;
        processNextTurn();
    }
    
    void sellFood() {
        if (player.getFood() >= GameConstants::FOOD_SELL_AMOUNT) {
            player.updateStats(0, -GameConstants::FOOD_SELL_AMOUNT, 0);
            player.addMoney(GameConstants::FOOD_SELL_PRICE);
            eventMessage = "Sold food for 150 Rs";
            eventTimer = 1.5f;
            processNextTurn();
        } else {
            eventMessage = "Not enough food to sell!";
            eventTimer = 1.5f;
        }
    }
    
    void buildTent() {
        if (player.getMoney() >= GameConstants::TENT_COST) {
            player.addMoney(-GameConstants::TENT_COST);
            player.buildTent();
            eventMessage = "Tent built! You're protected from storms";
            eventTimer = 1.5f;
            processNextTurn();
        } else {
            eventMessage = "Not enough money! Need 300 Rs";
            eventTimer = 1.5f;
        }
    }
    
    void eatFood() {
        if (player.eatFood()) {
            eventMessage = "You ate food and gained health!";
            eventTimer = 1.5f;
            triggerActionAnim("eat");
            processNextTurn();
        } else {
            eventMessage = "Not enough food to eat!";
            eventTimer = 1.5f;
        }
    }
    
    void saveGame() {
        player.saveToFile("save.txt");
        eventMessage = "Game saved successfully!";
        eventTimer = 2.0f;
    }
    
    void loadGame() {
        if (player.loadFromFile("save.txt")) {
            eventMessage = "Game loaded successfully!";
        } else {
            eventMessage = "No save file found!";
        }
        eventTimer = 2.0f;
    }
   void triggerActionAnim(const string& action) {
    lastAction = action;
    actionAnimTimer = 0.6f;
}
string getLastAction() const { return lastAction; }
float getActionAnimTimer() const { return actionAnimTimer; }
void updateActionAnim(float dt) {
    if (actionAnimTimer > 0) actionAnimTimer -= dt;
}
    void processNextTurn() {
        updateTime();
        handleFindStick();
        handleStorm();
        handleWolfAttack();
        checkGameStatus();
    }
    
    const Player& getPlayer() const { return player; }
    bool isGameOver() const { return gameOver; }
    bool hasPlayerWon() const { return playerWon; }
    string getGameOverMessage() const { return gameOverMessage; }
    bool isDayTime() const { return isDay; }
    string getEventMessage() const { return eventMessage; }
    float getEventTimerDuration() const { return eventTimer; }
};

void DrawWolfCharacter(int playerX, int groundY, float alpha)
{
    int cx = playerX + 110;

    // Grey wolf palette
    Color furBack   = Color{120, 125, 135, (unsigned char)alpha}; // steel grey
    Color furBody   = Color{165, 170, 178, (unsigned char)alpha}; // lighter grey
    Color furBelly  = Color{225, 225, 225, (unsigned char)alpha}; // near white
    Color furDark   = Color{80, 82, 90, (unsigned char)alpha};    // shadow/legs/ears tips
    Color eyeColor  = Color{230, 200, 90, (unsigned char)alpha};  // amber eyes
    Color noseColor = Color{25, 25, 25, (unsigned char)alpha};
//Tail
DrawEllipse(cx + 58, groundY - 40, 16, 9, furBack);
DrawEllipse(cx + 63, groundY - 45, 10, 6, furBelly);

//Body
DrawEllipse(cx, groundY - 36, 38, 18, furBack);
DrawEllipse(cx, groundY - 24, 30, 10, furBelly);

    // Legs
    DrawRectangle(cx - 38, groundY - 18, 10, 20, furDark);
    DrawRectangle(cx - 14, groundY - 18, 10, 20, furDark);
    DrawRectangle(cx + 14, groundY - 18, 10, 20, furDark);
    DrawRectangle(cx + 34, groundY - 18, 10, 20, furDark);

// Head
int headX = cx - 55;
int headY = groundY - 50;
DrawRectangleRounded({(float)headX - 16, (float)headY - 16, 32, 32}, 0.15f, 4, furBody);

// Snout
DrawTriangle(
    {(float)headX - 12, (float)headY - 5},
    {(float)headX - 36, (float)headY + 3},
    {(float)headX - 12, (float)headY + 11},
    furBelly
);
DrawCircle(headX - 34, headY + 3, 3, noseColor);

// Ears 
DrawTriangle({(float)headX - 14, (float)headY - 14}, {(float)headX - 20, (float)headY - 34}, {(float)headX - 2, (float)headY - 16}, furBack);
DrawTriangle({(float)headX + 4, (float)headY - 16}, {(float)headX, (float)headY - 36}, {(float)headX + 14, (float)headY - 18}, furBack);

// Eyes
DrawCircle(headX - 8, headY - 2, 3, eyeColor);
DrawEllipse(headX - 8, headY - 4, 3, 1.2f, Color{50,40,20,(unsigned char)alpha});
}
void DrawPlayer(int x, int groundY, bool hasWeapon, const string& action, float animTimer, float wolfTimer)
{
    float t = GetTime();
    float bob = sinf(t * 4.0f) * 3.0f;
    int baseY = groundY - (int)bob;

    Color skin = Color{224, 172, 128, 255};
    Color shirt = Color{60, 90, 150, 255};
    Color pants = Color{50, 50, 60, 255};

    bool animating = animTimer > 0;
    float progress = animating ? (0.6f - animTimer) / 0.6f : 0; // 0 to 1 over the anim

    int legOffsetL = 0, legOffsetR = 0;
    int armAngleL = 0, armAngleR = 0;
    int bodyLean = 0;
    int knockback = 0;
    if (wolfTimer > 0) {
        float knockProgress = min(1.0f, wolfTimer * 2.0f);
        knockback = (int)(-20 * knockProgress);
    }

    if (animating && action == "travel") {
        float walk = sinf(t * 14.0f);
        legOffsetL = (int)(walk * 10);
        legOffsetR = (int)(-walk * 10);
        armAngleL = (int)(-walk * 8);
        armAngleR = (int)(walk * 8);
        bodyLean = (int)(15 * sinf(progress * PI));
    }
    else if (animating && action == "hunt") {
        armAngleR = (int)(-40 * sinf(progress * PI)); // arm swings forward and back
        bodyLean = (int)(5 * sinf(progress * PI));
    }
    else if (animating && action == "rest") {
        bob = 0; 
        baseY = groundY;
    }
    else if (animating && action == "eat") {
        armAngleR = -50; 
    }

    int headY = baseY - 70;
    int bodyTopY = baseY - 55;
    int bodyBottomY = baseY - 20;

    // Legs
    DrawRectangle(x - 12 + (bodyLean + knockback), bodyBottomY + legOffsetL / 4, 8, 20 - legOffsetL / 4, pants);
    DrawRectangle(x + 4 + (bodyLean + knockback), bodyBottomY + legOffsetR / 4, 8, 20 - legOffsetR / 4, pants);

    // Body
    DrawRectangle(x - 15 +(bodyLean + knockback), bodyTopY, 30, 35, shirt);

    // Arms
    Vector2 armLStart = {(float)(x - 15 + (bodyLean + knockback)), (float)(bodyTopY + 8)};
    Vector2 armLEnd = {armLStart.x - 12, armLStart.y + 20 + armAngleL / 3};
    DrawLineEx(armLStart, armLEnd, 7, skin);

    Vector2 armRStart = {(float)(x + 15 +(bodyLean + knockback)), (float)(bodyTopY + 8)};
    Vector2 armREnd = {armRStart.x + 12, armRStart.y + 20 + armAngleR / 2};
    if (action == "eat" && animating) {
        armREnd = {(float)(x + 5 + (bodyLean + knockback)), (float)(headY + 5)};
    }
    DrawLineEx(armRStart, armREnd, 7, skin);

    // Head
    DrawCircle(x + (bodyLean + knockback), headY, 14, skin);
    DrawRectangle(x - 14 + (bodyLean + knockback), headY - 14, 28, 6, Color{40, 30, 20, 255});
    // Face features
int faceX = x + (bodyLean + knockback);
int faceY = headY;

// Eyes
DrawCircle(faceX - 5, faceY - 2, 2, Color{30, 30, 30, 255});
DrawCircle(faceX + 5, faceY - 2, 2, Color{30, 30, 30, 255});

// Eyebrows 
DrawLine(faceX - 8, faceY - 8, faceX - 2, faceY - 7, Color{60, 40, 25, 255});
DrawLine(faceX + 2, faceY - 7, faceX + 8, faceY - 8, Color{60, 40, 25, 255});

// Mouth 
if (action == "eat" && animating) {
    DrawCircle(faceX, faceY + 6, 2, Color{100, 40, 40, 255}); // open mouth chewing
} else if (action == "hunt" && animating) {
    DrawLine(faceX - 4, faceY + 6, faceX + 4, faceY + 6, Color{100, 40, 40, 255}); // flat/tense
} else {
    DrawLine(faceX - 3, faceY + 6, faceX + 3, faceY + 5, Color{100, 40, 40, 255}); // neutral/slight smile
}

// Nose (tiny hint)
DrawLine(faceX, faceY, faceX, faceY + 3, Color{200, 150, 110, 255});

    // Stick, if carrying one
    if (hasWeapon) {
        DrawLine(x + 20 + (bodyLean + knockback), bodyTopY + 10, x + 35 + (bodyLean + knockback), bodyTopY - 15, Color{101, 67, 33, 255});
    }
    if (wolfTimer > 0) {
    float bloodAlpha = min(1.0f, wolfTimer * 2.0f) * 255.0f;
    Color blood = Color{180, 20, 20, (unsigned char)bloodAlpha};
    int bx = x + bodyLean + knockback;
    DrawCircle(bx + 10, headY + 10, 3, blood);
    DrawCircle(bx + 15, headY + 18, 2, blood);
    DrawCircle(bx + 6, headY + 22, 2, blood);
    DrawCircle(bx + 20, headY + 30, 3, blood);
    DrawCircle(bx + 25, headY + 34, 2, blood);
    DrawCircle(bx + 16, headY + 42, 2, blood);
    DrawLine(bx + 8, headY + 8, bx + 12, headY + 20, blood);
}
}    
void DrawForestTreeBack(int x, int groundY, int size)
{
    // Same shape as DrawForestTree but muted colors, smaller, drawn first
    int trunkWidth = size / 5;
    int trunkHeight = size;
    int trunkX = x - trunkWidth / 2;
    int trunkY = groundY - trunkHeight;

    DrawRectangle(trunkX, trunkY, trunkWidth, trunkHeight, Color{60, 45, 30, 180});

    Color leafDark = Color{20, 70, 20, 180};
    Color leafMain = Color{35, 90, 35, 180};

    float foliageX = x;
    float foliageY = trunkY - size / 4;

    DrawTriangle(
        {foliageX - size / 2, foliageY + size / 3},
        {foliageX + size / 2, foliageY + size / 3},
        {foliageX, foliageY - size / 3},
        leafDark
    );
    DrawTriangle(
        {foliageX - size / 2.5f, foliageY - size / 6},
        {foliageX + size / 2.5f, foliageY - size / 6},
        {foliageX, foliageY - size / 2},
        leafMain
    );
}



void DrawStormCloud(int x, int y, float scale, unsigned char alpha)
{
    Color cloudDark = Color{60, 60, 75, alpha};
    Color cloudMid  = Color{90, 90, 105, alpha};

    DrawCircle(x, y, 35 * scale, cloudDark);
    DrawCircle(x + 35 * scale, y - 10 * scale, 40 * scale, cloudMid);
    DrawCircle(x + 75 * scale, y, 32 * scale, cloudDark);
    DrawCircle(x + 40 * scale, y + 10 * scale, 38 * scale, cloudMid);
}
 void DrawForestTree(int x, int groundY, int size)
{
    // ============= TRUNK =============
    int trunkWidth = size / 5;    
    int trunkHeight = size ;
    int trunkX = x - trunkWidth / 2;
    int trunkY = groundY - trunkHeight;

    DrawRectangle(trunkX, trunkY, trunkWidth, trunkHeight, Color{101, 67, 33, 255});

    // ============= FOLIAGE - TRIANGULAR =============
    Color leafDark   = Color{35, 120, 35, 255};
    Color leafMain   = Color{60, 150, 60, 255};
    Color leafLight  = Color{90, 180, 90, 255};

    float foliageX = x;
    float foliageY = trunkY - size / 4;

    // Triangle 1 (bottom - widest)
    DrawTriangle(
        {foliageX - size / 2, foliageY + size / 3},      // Left
        {foliageX + size / 2, foliageY + size / 3},      // Right
        {foliageX, foliageY - size / 3},                 // Top
        leafDark
    );

    // Triangle 2 (middle)
    DrawTriangle(
        {foliageX - size / 2.5f, foliageY - size / 6},
        {foliageX + size / 2.5f, foliageY - size / 6},
        {foliageX, foliageY - size / 2},
        leafMain
    );

    // Triangle 3 (top - narrow)
    DrawTriangle(
        {foliageX - size / 3.5f, foliageY - size / 2.5f},
        {foliageX + size / 3.5f, foliageY - size / 2.5f},
        {foliageX, foliageY - size},
        leafLight
    );

    // ============= BUSHES =============
    int bushRadius = size / 5;
    int bushY = groundY - bushRadius;

    // Left bushes
    DrawCircle(x - size / 2 - 10, bushY, bushRadius, leafDark);
    DrawCircle(x - size / 2 + 10, bushY - 5, bushRadius - 3, leafMain);

    // Center bushes
    DrawCircle(x - 15, bushY + 2, bushRadius + 2, leafMain);
    DrawCircle(x, bushY, bushRadius + 3, leafMain);
    DrawCircle(x + 15, bushY + 2, bushRadius + 2, leafMain);

    // Right bushes
    DrawCircle(x + size / 2 - 10, bushY - 5, bushRadius - 3, leafMain);
    DrawCircle(x + size / 2 + 10, bushY, bushRadius, leafDark);
}
    
// ================= MAIN GUI APPLICATION =================
int main() {
    const int screenWidth = 1450;
    const int screenHeight = 800;
    
    InitWindow(screenWidth, screenHeight, "Survival Trail Simulator - Forest Adventure");
    SetTargetFPS(60);
    
    Game game;
   Font gameFont = LoadFont("Assets/Rough One/The Foregen Rough One.ttf");
    UIRenderer renderer;
    float parallaxOffset = 0;
    int lastDistance = 0;
    
    // ===== BUTTON LAYOUT - HORIZONTAL BOTTOM ROW =====
    vector<Button> actionButtons;
    int buttonWidth = 130;
    int buttonHeight = 55;
    int startX = 20;
    int startY = 730;  // Bottom of screen
    int spacing = 10;
    
    // All 10 buttons in horizontal row at bottom
    actionButtons.push_back(Button(startX + 0*(buttonWidth + spacing), startY, buttonWidth, buttonHeight, 
                                   "TRAVEL", Color{34, 177, 76, 255}, Color{52, 211, 94, 255}));
    actionButtons.push_back(Button(startX + 1*(buttonWidth + spacing), startY, buttonWidth, buttonHeight,
                                   "REST", Color{102, 153, 102, 255}, Color{153, 204, 102, 255}));
    actionButtons.push_back(Button(startX + 2*(buttonWidth + spacing), startY, buttonWidth, buttonHeight,
                                   "HUNT", Color{139, 69, 19, 255}, Color{184, 92, 24, 255}));
    actionButtons.push_back(Button(startX + 3*(buttonWidth + spacing), startY, buttonWidth, buttonHeight,
                                   "SELL", Color{184, 134, 11, 255}, Color{218, 165, 32, 255}));
    actionButtons.push_back(Button(startX + 4*(buttonWidth + spacing), startY, buttonWidth, buttonHeight,
                                   "TENT", Color{70, 140, 70, 255}, Color{100, 180, 100, 255}));
    actionButtons.push_back(Button(startX + 5*(buttonWidth + spacing), startY, buttonWidth, buttonHeight, 
                                   "EAT", Color{200, 100, 50, 255}, Color{240, 150, 80, 255}));
    actionButtons.push_back(Button(startX + 6*(buttonWidth + spacing), startY, buttonWidth, buttonHeight,
                                   "INV", Color{100, 150, 200, 255}, Color{150, 200, 255, 255}));
    actionButtons.push_back(Button(startX + 7*(buttonWidth + spacing), startY, buttonWidth, buttonHeight,
                                   "SAVE", Color{100, 100, 150, 255}, Color{150, 150, 200, 255}));
    actionButtons.push_back(Button(startX + 8*(buttonWidth + spacing), startY, buttonWidth, buttonHeight,
                                   "LOAD", Color{100, 100, 150, 255}, Color{150, 150, 200, 255}));
    actionButtons.push_back(Button(startX + 9*(buttonWidth + spacing), startY, buttonWidth, buttonHeight,
                                   "EXIT", Color{150, 50, 50, 255}, Color{200, 100, 100, 255}));

    
    
    // Main game loop
    while (!WindowShouldClose() && !game.isGameOver()) {
        // Update buttons
        for (auto& btn : actionButtons) {
            btn.update();
        }
        
        // Handle button clicks
        if (actionButtons[0].isClicked()) game.travel();
        if (actionButtons[1].isClicked()) game.rest();
        if (actionButtons[2].isClicked()) game.hunt();
        if (actionButtons[3].isClicked()) game.sellFood();
        if (actionButtons[4].isClicked()) game.buildTent();
        if (actionButtons[5].isClicked()) game.eatFood();
        if (actionButtons[6].isClicked()) {} 
        if (actionButtons[7].isClicked()) game.saveGame();
        if (actionButtons[8].isClicked()) game.loadGame();
        if (actionButtons[9].isClicked()) break;


         game.updateEffects(GetFrameTime());
game.updateActionAnim(GetFrameTime());
         int currentDistance = game.getPlayer().getDistance();
        if (currentDistance != lastDistance) {
            parallaxOffset += (currentDistance - lastDistance) * 8.0f; 
            lastDistance = currentDistance;
}
        
        // ===== DRAW PHASE =====
        BeginDrawing();
        // Background based on day/night
        if (game.isDayTime()) {
            ClearBackground(Color{135, 206, 235, 255});
            
            // Sunlight
            DrawCircle(1350, 50, 80, Color{255, 255, 150, 80});
            DrawCircle(1350, 50, 60, Color{255, 255, 200, 100});
            

                int backOffset = (int)parallaxOffset % 300;
                DrawForestTreeBack(250 - backOffset, 700, 70);
                DrawForestTreeBack(550 - backOffset, 700, 70);
                DrawForestTreeBack(850 - backOffset, 700, 70);
                DrawForestTreeBack(1150 - backOffset, 700, 70);
                DrawForestTreeBack(1450 - backOffset, 700, 70);
                            
                DrawForestTree(350, 700, 100);   // Tree 1
                DrawForestTree(600, 700, 100);   // Tree 2
                DrawForestTree(900, 700, 100);   // Tree 3
                DrawForestTree(1200, 700, 100);  // Tree 4
                
               DrawPlayer(700, 700, game.getPlayer().hasWeapon(), game.getLastAction(), game.getActionAnimTimer(), game.getWolfTimer());
              if (game.getPlayer().hasTentBuilt()) {
    // Main tent triangle
    DrawTriangle({480, 700}, {620, 700}, {550, 540}, Color{120, 80, 40, 255});
    // Front flap (darker, for depth)
    DrawTriangle({550, 700}, {620, 700}, {550, 540}, Color{90, 60, 30, 255});
    // Entrance opening
DrawTriangle({510, 700}, {560, 700}, {550, 610}, Color{40, 25, 15, 255});
}
if (game.getWolfTimer() > 0) {
    float alpha = min(1.0f, game.getWolfTimer() * 2.0f) * 255.0f;
    DrawWolfCharacter(700, 700, alpha);
}
             game.getEffects().drawStormEffects();
             game.getEffects().drawWolfEffects();
            
            
            // Ground
            DrawRectangle(50, 700, 2000 , 100, Color{34, 139, 34, 255});
            
        } else {
            // Night time
            ClearBackground(Color{10, 10, 20, 255});
            
            // Moon
            DrawCircle(1300, 80, 70, Color{255, 255, 150, 255});
            DrawCircle(1300, 80, 70, Color{200, 200, 100, 150});
            
            // Stars
            for (int i = 0; i < 15; i++) {
                int starX = (250 + i * 75) % (screenWidth - 200) + 220;
                int starY = 50 + (i * 20) % 200;
                int starSize = (i % 3 == 0) ? 2 : 1;
                Color starColor = (i % 2 == 0) ? Color{255, 255, 255, 255} : Color{200, 200, 255, 255};
                DrawCircle(starX, starY, starSize, starColor);
            }
                int backOffset = (int)parallaxOffset % 300;
            DrawForestTreeBack(250 - backOffset, 700, 70);
            DrawForestTreeBack(550 - backOffset, 700, 70);
            DrawForestTreeBack(850 - backOffset, 700, 70);
            DrawForestTreeBack(1150 - backOffset, 700, 70);
            DrawForestTreeBack(1450 - backOffset, 700, 70);
                DrawForestTree(350, 700, 100);   // Tree 1
                DrawForestTree(600, 700, 100);   // Tree 2
                DrawForestTree(900, 700, 100);   // Tree 3
                DrawForestTree(1200, 700, 100);  // Tree 4
               
              DrawPlayer(700, 700, game.getPlayer().hasWeapon(), game.getLastAction(), game.getActionAnimTimer(), game.getWolfTimer());
              if (game.getPlayer().hasTentBuilt()) {
    // Main tent triangle 
    DrawTriangle({480, 700}, {620, 700}, {550, 540}, Color{120, 80, 40, 255});
    // Front flap (darker, for depth)
    DrawTriangle({550, 700}, {620, 700}, {550, 540}, Color{90, 60, 30, 255});
    // Entrance opening
DrawTriangle({510, 700}, {560, 700}, {550, 610}, Color{40, 25, 15, 255});
}
if (game.getWolfTimer() > 0) {
    float alpha = min(1.0f, game.getWolfTimer() * 2.0f) * 255.0f;
    DrawWolfCharacter(700, 700, alpha);
}
                game.getEffects().drawStormEffects();
                game.getEffects().drawWolfEffects();

             
            
            // Dark ground
            DrawRectangle(100, 700, 2500 , 100, Color{20, 30, 20, 255});
        }
        
        // ===== TITLE BAR =====
        DrawRectangleRounded({15, 10, 280, 40}, 0.3f, 6, Color{34, 102, 34, 255});
DrawTextEx(gameFont, "Survival trail simulator", {30, 22}, 20, 1, Color{144, 238, 144, 255});


        
        // ===== DRAW UI PANELS =====
       renderer.drawStatsPanel(game.getPlayer());
float msgTimer = game.getEventTimerDuration();
renderer.drawEventMessage(game.getEventMessage(), msgTimer, screenWidth);
        
        // ===== DRAW BOTTOM BUTTONS =====
        Font defaultFont = GetFontDefault();
        DrawRectangle(10, 720, screenWidth - 20, 80, Color{40, 70, 40, 150});
        DrawRectangleLinesEx({10, 720, screenWidth - 20, 80}, 2, Color{100, 200, 100, 255});
        
        for (auto btn : actionButtons) {
            btn.draw(defaultFont);
        }
        
        EndDrawing();
    }
    
    // Game over screen
    if (game.isGameOver()) {
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(Color{20, 20, 30, 255});
            
            renderer.drawGameOver(game.getGameOverMessage(), game.getPlayer());
            
            EndDrawing();
        }
    }
    
    CloseWindow();
    return 0;
}
