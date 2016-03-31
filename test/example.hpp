/* 
 * Copyright (c) 2012 ~ 2019 zaxbbun <zaxbbun@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string>

enum EventType {
    kCreatureBorn,
    kPlayerLogin,
    kPlayerLogout,
    kPlayerKill,
    kCreatureHurt,
};

class Creature
{
    public:
        Creature(const char *name, int x, int y, int hp):
            m_name(name),
            m_x(x),
            m_y(y),
            m_hp(hp) { }
        ~Creature() = default;

        std::string Name() const
        {
            return m_name;
        }

        int GetX() const
        {
            return m_x;
        }

        int GetY() const
        {
            return m_y;
        }

        int GetHP() const
        {
            return m_hp;
        }

        void Damage(int hp)
        {
            if (hp < 0) {
                return;
            }

            m_hp = m_hp > hp ? m_hp - hp : 0;
        }

    private:
        std::string m_name;
        int m_x;
        int m_y;
        int m_hp;
};

class Player: public Creature
{
    public:
        Player(const char *name, int x, int y, int hp, int ap, int ad):
            Creature(name, x, y, hp),
            m_ability_power(ap),
            m_attack_damage(ad) { }
        ~Player() = default;

        void Attack(Creature &creature);
        void SpellHit(Creature &creature);

        int honor;

    private:
        int m_ability_power;
        int m_attack_damage;
};
