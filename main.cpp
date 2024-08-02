#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

// プレイヤークラス
class Player {
public:
    Player(float startX, float startY) {
        shape.setSize(sf::Vector2f(50.0f, 50.0f));
        shape.setPosition(startX, startY);
        shape.setFillColor(sf::Color::Green);
        normalSpeed = 0.05f;
        speed = normalSpeed;
    }

    void move(float dx, float dy) {
        sf::Vector2f pos = shape.getPosition();
        pos.x += dx * speed;
        pos.y += dy * speed;

        // 画面内に制限
        if (pos.x < 0) pos.x = 0;
        if (pos.y < 0) pos.y = 0;
        if (pos.x + shape.getSize().x > 800) pos.x = 800 - shape.getSize().x;
        if (pos.y + shape.getSize().y > 600) pos.y = 600 - shape.getSize().y;

        shape.setPosition(pos);
    }

    void reduceSpeed() {
        speed *= 0.5f;
    }

    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }

    void resetSpeed() {
        speed = normalSpeed;
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

private:
    sf::RectangleShape shape;
    float speed;
    float normalSpeed;
};

// 敵クラス
class Enemy {
public:
    Enemy(float startX, float startY) {
        shape.setSize(sf::Vector2f(50.0f, 50.0f));
        shape.setPosition(startX, startY);
        shape.setFillColor(sf::Color::Red);
        normalSpeed = 0.025f;
        speed = normalSpeed;
        direction = 1.0f;
        originalColor = shape.getFillColor();
    }

    void move() {
        shape.move(0.0f, speed * direction);
    }

    void resetPosition() {
        float x = static_cast<float>(rand() % 800);
        float y = direction > 0 ? 0.0f : 600.0f; // 方向に応じて再配置
        shape.setPosition(x, y);
    }

    void reverseDirection() {
        direction *= -1.0f;
    }

    void flash() {
        shape.setFillColor(sf::Color::White);
        flashClock.restart();
    }

    void update() {
        if (flashClock.getElapsedTime().asSeconds() > 0.1f) {
            shape.setFillColor(originalColor);
        }
    }

    void setSpeed(float newSpeed) {
        speed = newSpeed;
    }

    void resetSpeed() {
        speed = normalSpeed;
    }

    sf::RectangleShape getShape() const {
        return shape;
    }

private:
    sf::RectangleShape shape;
    float speed;
    float normalSpeed;
    float direction;
    sf::Color originalColor;
    sf::Clock flashClock;
};

// 敵を生成する関数
std::vector<Enemy> createEnemies(int count) {
    std::vector<Enemy> enemies;
    for (int i = 0; i < count; ++i) {
        float x = static_cast<float>(rand() % 800);
        float y = static_cast<float>(rand() % 600);
        enemies.emplace_back(x, y);
    }
    return enemies;
}

// 衝突判定関数
bool isColliding(const sf::RectangleShape& shape1, const sf::RectangleShape& shape2) {
    return shape1.getGlobalBounds().intersects(shape2.getGlobalBounds());
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Simple Game");
    Player player(400.0f, 300.0f);
    std::vector<Enemy> enemies = createEnemies(5);

    sf::Clock clock;
    srand(static_cast<unsigned>(time(0)));

    int collisionCount = 0;
    bool isInvincible = false;
    sf::Clock invincibilityClock;
    sf::Clock directionChangeClock;
    sf::Clock flashClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // プレイヤーの移動
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            player.move(-1.0f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            player.move(1.0f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            player.move(0.0f, -1.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            player.move(0.0f, 1.0f);

        // 敵の移動
        for (auto& enemy : enemies) {
            enemy.move();
            if (enemy.getShape().getPosition().y > 600 || enemy.getShape().getPosition().y < 0) {
                enemy.resetPosition();
            }
            enemy.update(); // フラッシュエフェクトの更新
        }

        // 衝突判定
        if (!isInvincible) {
            for (auto& enemy : enemies) {
                if (isColliding(player.getShape(), enemy.getShape())) {
                    collisionCount++;
                    player.reduceSpeed();
                    enemy.flash(); // 敵をフラッシュさせる
                    isInvincible = true;
                    invincibilityClock.restart();
                    flashClock.restart();
                    if (collisionCount >= 3) {
                        window.close(); // 3回当たったらゲーム終了
                    }
                    break; // 一度衝突したら他の敵との判定は不要
                }
            }
        }

        // 無敵時間の処理
        if (isInvincible && invincibilityClock.getElapsedTime().asSeconds() > 0.5f) {
            isInvincible = false;
            player.resetSpeed();
            for (auto& enemy : enemies) {
                enemy.resetSpeed();
            }
        }

        // 敵の方向転換
        if (directionChangeClock.getElapsedTime().asSeconds() > 10.0f) {
            for (auto& enemy : enemies) {
                enemy.reverseDirection();
            }
            directionChangeClock.restart();
        }

        // 画面の点滅と速度調整
        if (isInvincible) {
            if (flashClock.getElapsedTime().asMilliseconds() % 100 < 50) {
                window.clear(sf::Color::Red);
            }
            else {
                window.clear();
            }
            player.setSpeed(0.0125f); // 点滅中はプレイヤーの速度を25%に
            for (auto& enemy : enemies) {
                enemy.setSpeed(0.00625f); // 点滅中は敵の速度を25%に
            }
        }
        else {
            window.clear();
        }

        window.draw(player.getShape());
        for (const auto& enemy : enemies) {
            window.draw(enemy.getShape());
        }
        window.display();
    }

    return 0;
}