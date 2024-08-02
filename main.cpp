#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>

// プレイヤークラス
class Player {
public:
    Player(float startX, float startY) {
        shape.setPointCount(3);
        shape.setPoint(0, sf::Vector2f(0, -25));
        shape.setPoint(1, sf::Vector2f(-20, 25));
        shape.setPoint(2, sf::Vector2f(20, 25));
        shape.setPosition(startX, startY);
        shape.setFillColor(sf::Color::Yellow);
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color::White);
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
        if (pos.x > 800) pos.x = 800;
        if (pos.y > 600) pos.y = 600;

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

    sf::ConvexShape getShape() const {
        return shape;
    }

private:
    sf::ConvexShape shape;
    float speed;
    float normalSpeed;
};

// 敵クラス
class Enemy {
public:
    Enemy(float startX, float startY) {
        shape.setRadius(20);
        shape.setPosition(startX, startY);
        shape.setFillColor(sf::Color::Red);
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color::White);
        normalSpeed = 0.025f;
        speed = normalSpeed;
        direction = 1.0f;
        originalColor = shape.getFillColor();
        animationClock.restart();
    }

    void move() {
        shape.move(0.0f, speed * direction);
        // アニメーション
        float scale = 1.0f + 0.1f * std::sin(animationClock.getElapsedTime().asSeconds() * 5);
        shape.setScale(scale, scale);
    }

    void resetPosition() {
        float x = static_cast<float>(rand() % 800);
        float y = direction > 0 ? 0.0f : 600.0f;
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

    void increaseSpeed(float increment) {
        speed += increment;
    }

    void resetSpeed() {
        speed = normalSpeed;
    }

    sf::CircleShape getShape() const {
        return shape;
    }

private:
    sf::CircleShape shape;
    float speed;
    float normalSpeed;
    float direction;
    sf::Color originalColor;
    sf::Clock flashClock;
    sf::Clock animationClock;
};

// 背景クラス
class Background {
public:
    Background() {
        texture.create(800, 600);
        sf::Image image;
        image.create(800, 600, sf::Color::Black);

        // 星を描画
        for (int i = 0; i < 200; ++i) {
            int x = rand() % 800;
            int y = rand() % 600;
            image.setPixel(x, y, sf::Color::White);
        }

        texture.update(image);
        sprite.setTexture(texture);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

private:
    sf::Texture texture;
    sf::Sprite sprite;
};

// メイン関数
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "esc@pe");
    Player player(400.0f, 500.0f);
    std::vector<Enemy> enemies;
    for (int i = 0; i < 5; ++i) {
        enemies.emplace_back(rand() % 800, rand() % 300);
    }
    Background background;

    sf::Clock clock;
    srand(static_cast<unsigned>(time(0)));

    int collisionCount = 0;
    bool isInvincible = false;
    sf::Clock invincibilityClock;
    sf::Clock directionChangeClock;
    sf::Clock flashClock;
    sf::Clock speedIncreaseClock;
    sf::Clock gameClock; // ゲーム開始からの経過時間を計測

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
                if (player.getShape().getGlobalBounds().intersects(enemy.getShape().getGlobalBounds())) {
                    collisionCount++;
                    player.reduceSpeed();
                    enemy.flash();
                    isInvincible = true;
                    invincibilityClock.restart();
                    flashClock.restart();
                    if (collisionCount >= 3) {
                        window.close();
                    }
                    break;
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

        // 敵の速度増加
        if (speedIncreaseClock.getElapsedTime().asSeconds() > 1.0f) {
            for (auto& enemy : enemies) {
                enemy.increaseSpeed(0.005f); // 1秒ごとに速度を増加
            }
            speedIncreaseClock.restart();
        }

        // 描画
        window.clear();
        background.draw(window);
        window.draw(player.getShape());
        for (const auto& enemy : enemies) {
            window.draw(enemy.getShape());
        }

        // 画面の点滅と速度調整
        if (isInvincible) {
            if (flashClock.getElapsedTime().asMilliseconds() % 100 < 50) {
                sf::RectangleShape flashRect(sf::Vector2f(800, 600));
                flashRect.setFillColor(sf::Color(255, 0, 0, 100));
                window.draw(flashRect);
            }
            player.setSpeed(0.0125f);
            for (auto& enemy : enemies) {
                enemy.setSpeed(0.00625f);
            }
        }

        window.display();
    }

    // ゲーム終了時に経過時間を表示
    float elapsedTime = gameClock.getElapsedTime().asSeconds();
    std::cout << "Your Score: " << std::fixed << std::setprecision(2) << elapsedTime << std::endl;

    return 0;
}
