#pragma once
#include <tuple>
#include <list>
#include <cmath>
#include <variant>
#include <vector>
#include <set>
// this is ectually failry ridiculous system, but it's designed to show how the chosen solution for "decorator" problem
// has changed over time. A decortator problem is one which would fit an inheretance pattern, in that there a number of 
// "configuarations", each inheriting from a common base. However,the problem is really a run-time problem, as the user
// can select a very large number of configurations, making iterating all possible configurations impractical. 
// This makes it impractical to solve this problem through inheritence. Instead, we solve it by attaching a list of 
// "decorators"

bool Compare(int v1, int v2) noexcept { return v1==v2;}
bool Compare(float v1, float v2) noexcept { return std::abs(v1-v2) < 0.01f;}

template<class T>
bool Compare(const T& v1, const T& v2)
{
    return std::apply([&](auto&&... x){
        return std::apply([&](auto&&... y){
            return (Compare(x, y)&&...);
        }, v2);
    }, v1);
}


struct WeaponState
{
    float weight;
    float ammo;
    float accuracy;
    float armour_peneration;
    float energy_damage; 
    int shots_per_use;

    auto as_tuple() const noexcept
    {
        return std::make_tuple(weight, ammo, accuracy, 
                        armour_peneration, energy_damage, shots_per_use);
    }
    bool operator==(const WeaponState&other) const
    { 
        return Compare(as_tuple(),other.as_tuple());
    }
};

template<class T>
struct Reverse
{
    T& cont;
    Reverse(T& c):cont(c) {}
    auto begin() { return cont.rbegin();}
    auto end() { return cont.rend();}
};

namespace SingleLinkedList // common in older Games code-bases. 
{
// pure virtual base-class iterator
struct WeaponDecorator
{
    WeaponDecorator* next{nullptr};
    // neither assignable nor copyable
    WeaponDecorator() = default;
    WeaponDecorator(const WeaponDecorator&) noexcept = delete;
    WeaponDecorator& operator=(const WeaponDecorator&) noexcept = default;
    WeaponDecorator(WeaponDecorator&&) noexcept = delete;
    WeaponDecorator& operator=(WeaponDecorator&&) noexcept = delete;

    virtual WeaponState Decorate(const WeaponState&) = 0;
//    WeaponDecorator* Clone() = 0;
};


struct Bullet : public WeaponDecorator
{
    WeaponState Decorate(const WeaponState&) override;
};

struct HEBullet : public WeaponDecorator
{
    WeaponState Decorate(const WeaponState&) override;
};

struct ExtraBarrel : public WeaponDecorator
{
    WeaponState Decorate(const WeaponState&) override;
};

struct Scope : public WeaponDecorator
{
    WeaponState Decorate(const WeaponState&) override;
};

struct Rifle
{
    WeaponState initial_stats{
        .weight = 100,
        .ammo = 0,
        .accuracy = .5f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .1f,
        .shots_per_use = 1
    };
    WeaponDecorator* accessories{nullptr};
public:
    void AddAccessory(WeaponDecorator*);
    void RemoveAccessory(WeaponDecorator*);
    WeaponState GetStats();

    using Bullet = SingleLinkedList::Bullet;
    using HEBullet = SingleLinkedList::HEBullet;
    using Scope = SingleLinkedList::Scope;
};

}// Sngle list


// the sort of code that occured once standard library containers became common. Still reliant on virtual functions
// this stops us moving "ownership" into the container
namespace StandardLib 
{
// pure virtual base-class iterator
struct WeaponDecorator
{
    // neither assignable nor copyable
    WeaponDecorator() = default;
    WeaponDecorator(const WeaponDecorator&) noexcept = delete;
    WeaponDecorator& operator=(const WeaponDecorator&) noexcept = default;
    WeaponDecorator(WeaponDecorator&&) noexcept = delete;
    WeaponDecorator& operator=(WeaponDecorator&&) noexcept = delete;

    virtual WeaponState Decorate(const WeaponState&) = 0;
//    WeaponDecorator* Clone() = 0;
};


struct Bullet : public WeaponDecorator
{
    WeaponState Decorate(const WeaponState&) override;
};

struct HEBullet : public WeaponDecorator
{
    WeaponState Decorate(const WeaponState&) override;
};

struct ExtraBarrel : public WeaponDecorator
{
    WeaponState Decorate(const WeaponState&) override;
};

struct Scope : public WeaponDecorator
{
    WeaponState Decorate(const WeaponState&) override;
};

struct Rifle
{
    WeaponState initial_stats{
        .weight = 100,
        .ammo = 0,
        .accuracy = .5f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .1f,
        .shots_per_use = 1
    };
    std::set<WeaponDecorator*> accessories;
public:
    using Bullet = StandardLib::Bullet;
    using HEBullet = StandardLib::HEBullet;
    using Scope = StandardLib::Scope;

    void AddAccessory(WeaponDecorator*);
    void RemoveAccessory(WeaponDecorator*);
    WeaponState GetStats();
};

} //std::list

// Modern style, no pointers, no virtual functions
namespace Modern 
{
struct Bullet
{
    WeaponState Decorate(const WeaponState&);
};

struct HEBullet
{
    WeaponState Decorate(const WeaponState&);
};

struct ExtraBarrel
{
    WeaponState Decorate(const WeaponState&);
};

struct Scope
{
    WeaponState Decorate(const WeaponState&);
};

struct Rifle
{
    WeaponState initial_stats{
        .weight = 100,
        .ammo = 0,
        .accuracy = .5f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .1f,
        .shots_per_use = 1
    };
    using Bullet = Modern::Bullet;
    using HEBullet = Modern::HEBullet;
    using Scope = Modern::Scope;
    using WeaponDecorator = std::variant<Bullet*, HEBullet*, ExtraBarrel*,Scope*>;
    std::vector<WeaponDecorator> accessories;
    bool sorted{false};
public:
    void AddAccessory(WeaponDecorator);
    void RemoveAccessory(WeaponDecorator);
    void RemoveAccessories(std::vector<WeaponDecorator>&);

    WeaponState GetStats();
    void Sort();
};

} //Modern