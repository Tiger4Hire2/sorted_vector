#include "Decorator.hpp"
#include <gtest/gtest.h>
#include <benchmark/benchmark.h>

namespace SingleLinkedList
{

WeaponState Bullet::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += .001f;
    retval.ammo += 1;
    return retval;
}

WeaponState HEBullet::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += .1f;
    retval.ammo += 1;
    retval.energy_damage *= 1.5f;
    return retval;
}

WeaponState ExtraBarrel::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight *= 2.5;
    retval.shots_per_use += 1;
    return retval;
}

WeaponState Scope::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += 2.5;
    retval.accuracy *= 1.5;
    return retval;
}


WeaponState Rifle::GetStats()
{
    WeaponState retval = initial_stats;
    for (WeaponDecorator* pd = accessories; pd; pd = pd->next)
        retval = pd->Decorate(retval);
    return retval;
}

// insert at the front
void Rifle::AddAccessory(WeaponDecorator* acc)
{
    acc->next = accessories;
    accessories = acc;
}

int count(WeaponDecorator* tgt)
{
    int retval{0};
    while(tgt)
    {
        retval++;
        tgt = tgt->next;
    }
    return retval;
}
// brute force find, using the "trailing pointer" technique. Does nothing if tgt not found
void Rifle::RemoveAccessory(WeaponDecorator* tgt)
{
    WeaponDecorator** trailing = &accessories;
    WeaponDecorator* current = accessories;

    while (current)
    {
        if (current==tgt)
        {
            *trailing = current->next;
            break;
        }
        trailing = &current->next;
        current = current->next;
    }
}



}

namespace StandardLib
{
    // mostly the same as above
WeaponState Bullet::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += .001f;
    retval.ammo += 1;
    return retval;
}

WeaponState HEBullet::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += .1f;
    retval.ammo += 1;
    retval.energy_damage *= 1.5f;
    return retval;
}

WeaponState ExtraBarrel::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight *= 2.5;
    retval.shots_per_use += 1;
    return retval;
}

WeaponState Scope::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += 2.5;
    retval.accuracy *= 1.5;
    return retval;
}


WeaponState Rifle::GetStats()
{
    WeaponState retval = initial_stats;
    for (const auto iter : accessories)
        retval = iter->Decorate(retval);
    return retval;
}

// insert at the front
void Rifle::AddAccessory(WeaponDecorator* acc)
{
    accessories.insert(acc);
}

// brute force find, using the "trailing pointer" technique. Does nothing if tgt not found
void Rifle::RemoveAccessory(WeaponDecorator* tgt)
{
    accessories.erase(tgt);
//    std::erase(accessories, [tgt](WeaponDecorator* wd) {return wd==tgt;});
}
} //StandardLub


namespace Modern
{
WeaponState Bullet::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += .001f;
    retval.ammo += 1;
    return retval;
}

WeaponState HEBullet::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += .1f;
    retval.ammo += 1;
    retval.energy_damage *= 1.5f;
    return retval;
}

WeaponState ExtraBarrel::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight *= 2.5;
    retval.shots_per_use += 1;
    return retval;
}

WeaponState Scope::Decorate(const WeaponState& prev)
{
    WeaponState retval = prev;
    retval.weight += 2.5;
    retval.accuracy *= 1.5;
    return retval;
}


WeaponState Rifle::GetStats()
{
    WeaponState retval = initial_stats;
    for (const auto iter : accessories)
            std::visit( [&retval](const auto ptr){retval = ptr->Decorate(retval);}, iter);
    return retval;
}

// insert at the front
void Rifle::AddAccessory(WeaponDecorator acc)
{
    accessories.push_back(acc);
    sorted = false;
}

// brute force find, using the "trailing pointer" technique. Does nothing if tgt not found
void Rifle::RemoveAccessory(WeaponDecorator tgt)
{
    if (!sorted)
        std::sort(accessories.begin(), accessories.end());
    sorted = true;
    const auto found = std::lower_bound(accessories.begin(), accessories.end(), tgt);
    if (found!=accessories.end())
        accessories.erase(found);
}

void Rifle::RemoveAccessories(std::vector<WeaponDecorator>& tgt)
{
    if (!sorted)
        std::sort(accessories.begin(), accessories.end());
    sorted = true;
    std::sort(tgt.begin(), tgt.end());

    auto new_start = accessories.begin();
    auto new_end = accessories.end();
    for(const auto& v: tgt)
    {
        const auto found = std::lower_bound(new_start, new_end, v);
        if (found!=new_end)
        {
            std::swap(*found, accessories.back());
            new_start = found+1;
            new_end = new_end-1;
        }
    }
    accessories.resize(new_end-accessories.begin());

    if (!sorted)
        std::sort(accessories.begin(), accessories.end());
    sorted = true;
}

} //Modern

TEST(SingleLinkedList, AllInOne)
{
    using namespace SingleLinkedList;
    Rifle rifle;
    Bullet bullets[1000];  
    HEBullet he_bullets;   
    Scope scope;

    const auto stats_none1 = rifle.GetStats();
    for (auto& acc: bullets)
       rifle.AddAccessory(&acc);   
    rifle.AddAccessory(&he_bullets);   
    rifle.AddAccessory(&scope);
    const auto stats_all = rifle.GetStats();
   
    EXPECT_EQ(stats_all, (WeaponState{
        .weight = 103.6,
        .ammo = 1001,
        .accuracy = .75f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .15f,
        .shots_per_use = 1
    }));

    rifle.RemoveAccessory(&scope);

    const auto stats_all_but_scope = rifle.GetStats();
    EXPECT_EQ(stats_all_but_scope, (WeaponState{
        .weight = 101.1,
        .ammo = 1001,
        .accuracy = .5f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .15f,
        .shots_per_use = 1
    }));

    for (auto& acc: bullets)
       rifle.RemoveAccessory(&acc);

    const auto stats_he = rifle.GetStats();
    EXPECT_EQ(stats_he, (WeaponState{
        .weight = 100.1,
        .ammo = 1,
        .accuracy = .5f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .15f,
        .shots_per_use = 1
    }));
    rifle.RemoveAccessory(&he_bullets);
    const auto stats_none2 = rifle.GetStats();
    EXPECT_EQ(stats_none1, stats_none2);
}

TEST(StandardLib, AllInOne)
{
    using namespace StandardLib;
    Rifle rifle;
    Bullet bullets[1000];  
    HEBullet he_bullets;   
    Scope scope;

    const auto stats_none1 = rifle.GetStats();
    for (auto& acc: bullets)
       rifle.AddAccessory(&acc);   
    rifle.AddAccessory(&he_bullets);   
    rifle.AddAccessory(&scope);
    const auto stats_all = rifle.GetStats();
   
    EXPECT_EQ(stats_all, (WeaponState{
        .weight = 103.6,
        .ammo = 1001,
        .accuracy = .75f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .15f,
        .shots_per_use = 1
    }));

    rifle.RemoveAccessory(&scope);
    for (auto& acc: bullets)
       rifle.RemoveAccessory(&acc);

    const auto stats_he = rifle.GetStats();
    EXPECT_EQ(stats_he, (WeaponState{
        .weight = 100.1,
        .ammo = 1,
        .accuracy = .5f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .15f,
        .shots_per_use = 1
    }));
    rifle.RemoveAccessory(&he_bullets);
    const auto stats_none2 = rifle.GetStats();
    EXPECT_EQ(stats_none1, stats_none2);
}

TEST(Modern, AllInOne)
{
    using namespace Modern;
    Rifle rifle;
    Bullet bullets[1000];  
    HEBullet he_bullets;   
    Scope scope;

    const auto stats_none1 = rifle.GetStats();
    for (auto& acc: bullets)
       rifle.AddAccessory(&acc);   
    rifle.AddAccessory(&he_bullets);   
    rifle.AddAccessory(&scope);
    const auto stats_all = rifle.GetStats();
   
    EXPECT_EQ(stats_all, (WeaponState{
        .weight = 103.6,
        .ammo = 1001,
        .accuracy = .75f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .15f,
        .shots_per_use = 1
    }));

    rifle.RemoveAccessory(&scope);

    const auto stats_all_but_scope = rifle.GetStats();
    EXPECT_EQ(stats_all_but_scope, (WeaponState{
        .weight = 101.1,
        .ammo = 1001,
        .accuracy = .5f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .15f,
        .shots_per_use = 1
    }));

    for (auto& acc: bullets)
       rifle.RemoveAccessory(&acc);

    const auto stats_he = rifle.GetStats();
    EXPECT_EQ(stats_he, (WeaponState{
        .weight = 100.1,
        .ammo = 1,
        .accuracy = .5f,
        .armour_peneration = 1.f,   //normal
        .energy_damage = .15f,
        .shots_per_use = 1
    }));
    rifle.RemoveAccessory(&he_bullets);
    const auto stats_none2 = rifle.GetStats();
    EXPECT_EQ(stats_none1, stats_none2);
}



// Benchmark section
template<class T>
static void Eval(benchmark::State &state)
{
    // build a rifle
    T rifle;
    typename T::Bullet bullets[1000];  
    typename T::HEBullet he_bullets;   
    typename T::Scope scope;

    for (auto& acc: bullets)
       rifle.AddAccessory(&acc);   
    rifle.AddAccessory(&he_bullets);   
    rifle.AddAccessory(&scope);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(rifle.GetStats());
    }
}

template<class T, bool stack_order>
static void AddRemove(benchmark::State &state)
{
    // build a rifle
    T rifle;
    std::vector<typename T::Bullet> bullets{1000*100};  
    typename T::HEBullet he_bullets;   
    typename T::Scope scope;

    for (auto _ : state)
    {
        //add/remove
        for (auto& acc: bullets)
            rifle.AddAccessory(&acc);   
        rifle.AddAccessory(&he_bullets);   
        rifle.AddAccessory(&scope);

        rifle.RemoveAccessory(&scope);
        if constexpr (stack_order)
            for (auto& acc: Reverse(bullets))
                rifle.RemoveAccessory(&acc);
        else   
            for (auto& acc: bullets)
                rifle.RemoveAccessory(&acc);
        rifle.RemoveAccessory(&he_bullets);   
    }
}

template<>
void AddRemove<Modern::Rifle, true>(benchmark::State &state)
{
    // build a rifle
    Modern::Rifle rifle;
    std::vector<Modern::Rifle::Bullet> bullets{1000*100};  
    typename Modern::Rifle::HEBullet he_bullets;   
    typename Modern::Rifle::Scope scope;

    for (auto _ : state)
    {
        //add/remove
        for (auto& acc: bullets)
            rifle.AddAccessory(&acc);   
        rifle.AddAccessory(&he_bullets);   
        rifle.AddAccessory(&scope);

        rifle.RemoveAccessory(&scope);
        std::vector<Modern::Rifle::WeaponDecorator> wrappers{bullets.size()};
        std::transform(bullets.begin(), bullets.end(), wrappers.begin(), [](auto& b){return Modern::Rifle::WeaponDecorator(&b);});
        rifle.RemoveAccessories(wrappers); 
        rifle.RemoveAccessory(&he_bullets);   
    }
}


static void EvalSingleLinkedList(benchmark::State &state)
{
    return Eval<SingleLinkedList::Rifle>(state);
}

static void EvalStandardLib(benchmark::State &state)
{
    return Eval<StandardLib::Rifle>(state);
}

static void EvalModern(benchmark::State &state)
{
    return Eval<Modern::Rifle>(state);
}

static void AddRemoveSLL(benchmark::State &state) {return AddRemove<SingleLinkedList::Rifle, true>(state);}
static void AddRemoveSLLNotStack(benchmark::State &state) {return AddRemove<SingleLinkedList::Rifle, false>(state);}
static void AddRemoveStd(benchmark::State &state) {return AddRemove<StandardLib::Rifle, true>(state);}
static void AddRemoveStdNotStack(benchmark::State &state) {return AddRemove<StandardLib::Rifle, false>(state);}
static void AddRemoveModernNotStackNotBatch(benchmark::State &state) {return AddRemove<Modern::Rifle, false>(state);}
static void AddRemoveModern(benchmark::State &state) {return AddRemove<Modern::Rifle, true>(state);}

BENCHMARK(EvalSingleLinkedList);
BENCHMARK(EvalStandardLib);
BENCHMARK(EvalModern);
BENCHMARK(AddRemoveSLL);
BENCHMARK(AddRemoveStd);
BENCHMARK(AddRemoveModern);
BENCHMARK(AddRemoveSLLNotStack);
BENCHMARK(AddRemoveStdNotStack);
BENCHMARK(AddRemoveModernNotStackNotBatch);
