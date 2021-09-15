#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <map>
#include <string>
#include <numeric>
#include <algorithm>
using namespace std::string_literals;
using MyMap = std::map<int, std::string>;
struct Elem
{
    int k;
    std::string v;
    friend bool operator<(const Elem &a, const Elem &b) noexcept { return a.k < b.k; }
    friend bool operator<(const Elem &a, int b) noexcept { return a.k < b; }
};

using MyVector = std::vector<Elem>;

bool Contains( const MyVector& vector, int val)
{
    const auto found = std::lower_bound(vector.begin(), vector.end(), val);
    return (found!=vector.end()) && 
            (found->k==val);
}

static MyMap CreateMap(int size)
{
    MyMap retval;
    // to be fair, make the map in a psuedo random order
    std::vector<int> nums;
    nums.resize(size);
    std::iota(nums.begin(), nums.end(), 0);
    std::random_shuffle(nums.begin(), nums.end());
    for (auto num : nums)
        retval.insert(std::make_pair(num, std::to_string(num)));
    return retval;
}

static MyVector CreateVector(int size)
{
    // to be fair, make the map in a psuedo random order
    std::vector<int> nums;
    nums.resize(size);
    std::iota(nums.begin(), nums.end(), 0);
    MyVector retval;
    retval.reserve(size);
    std::random_shuffle(nums.begin(), nums.end());
    for (auto num : nums)
        retval.emplace_back(Elem{num, std::to_string(num)});
    std::sort(retval.begin(), retval.end());
    return retval;
}

static std::vector<int> RandomSelection(int size, int select_size)
{
    assert(select_size < size);
    std::vector<int> nums;
    nums.resize(size);
    std::iota(nums.begin(), nums.end(), 0);
    std::random_shuffle(nums.begin(), nums.end());
    return std::vector<int>{nums.begin(), nums.begin() + select_size};
}

static void MapCreation(benchmark::State &state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(CreateMap(state.range(0)));
        state.PauseTiming();
        benchmark::ClobberMemory();
        state.ResumeTiming();
    }
}

static void VectorCreation(benchmark::State &state)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(CreateVector(state.range(0)));
        state.PauseTiming();
        benchmark::ClobberMemory();
        state.ResumeTiming();
    }
}

static void MapLookup(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        const auto map = CreateMap(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), 100);
        state.ResumeTiming();
        for (const int rnd : random_Selection)
        {
            benchmark::DoNotOptimize(map.lower_bound(rnd));
            state.PauseTiming();
            benchmark::ClobberMemory();
            state.ResumeTiming();
        }
    }
}

static void VectorLookup(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        const auto vector = CreateVector(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), 100);
        state.ResumeTiming();
        for (const int rnd : random_Selection)
        {
            benchmark::DoNotOptimize(std::lower_bound(vector.begin(), vector.end(), rnd));
            state.PauseTiming();
            benchmark::ClobberMemory();
            state.ResumeTiming();
        }
    }
}

static void MapDelete(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto map = CreateMap(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), 100);
        state.ResumeTiming();
        for (const int rnd : random_Selection)
        {
            const MyMap::iterator pos = map.lower_bound(rnd);
            map.erase(pos);
            state.PauseTiming();
            benchmark::ClobberMemory();
            state.ResumeTiming();
        }
    }
}

static void VectorDelete(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto vector = CreateVector(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), 100);
        state.ResumeTiming();
        for (const int rnd : random_Selection)
        {
            vector.erase(std::lower_bound(vector.begin(), vector.end(), rnd));
            state.PauseTiming();
            benchmark::ClobberMemory();
            state.ResumeTiming();
        }
    }
}

static void VectorMultiDelete(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto vector = CreateVector(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), 100);
        state.ResumeTiming();
        for (const int rnd : random_Selection)
        {
            const auto found = std::lower_bound(vector.begin(), vector.end(), rnd);
            std::swap(*found, vector.back());
            vector.pop_back();
        }
        std::sort(vector.begin(), vector.end());
    }
}

static void VectorMultiDeleteMagic(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto vector = CreateVector(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), 100);
        state.ResumeTiming();
        for (const int rnd : random_Selection)
        {
            const auto found = std::lower_bound(vector.begin(), vector.end(), rnd);
            std::swap(*found, vector.back());
            vector.pop_back();
        }
        std::make_heap(vector.begin(), vector.end());
        std::sort(vector.begin(), vector.end());
    }
}

constexpr int MAX = 8;
BENCHMARK(MapCreation)->RangeMultiplier(2)->Range(1024 * 128, MAX * 1024 * 1024);
;
BENCHMARK(VectorCreation)->RangeMultiplier(2)->Range(1024 * 128, MAX * 1024 * 1024);
;
BENCHMARK(MapLookup)->RangeMultiplier(2)->Range(1024 * 128, MAX * 1024 * 1024);
;
BENCHMARK(VectorLookup)->RangeMultiplier(2)->Range(1024 * 128, MAX * 1024 * 1024);
;
BENCHMARK(MapDelete)->RangeMultiplier(2)->Range(1024 * 128, MAX * 1024 * 1024);
;
BENCHMARK(VectorDelete)->RangeMultiplier(2)->Range(1024 * 128, MAX * 1024 * 1024);
;
BENCHMARK(VectorMultiDelete)->RangeMultiplier(2)->Range(1024 * 128, MAX * 1024 * 1024);
;
BENCHMARK(VectorMultiDeleteMagic)->RangeMultiplier(2)->Range(1024 * 128, MAX * 1024 * 1024);
;


// Module tests section.
// It doesn't matter how fast it is, if it doesnt work

TEST(SortedVector, Create)
{
    // check head tail and midpoint
    auto vector = CreateVector(100);
    EXPECT_EQ(vector.size(), 100);
    EXPECT_EQ(vector[0].v, "0"s);
    EXPECT_EQ(vector[50].v, "50"s);
    EXPECT_EQ(vector[99].v, "99"s);
}

TEST(SortedVector, Lookup)
{
    const auto vector = CreateVector(100);
    const auto random_Selection = RandomSelection(vector.size(), 10);
    for (const int rnd : random_Selection)
    {
        const auto found = std::lower_bound(vector.begin(), vector.end(), rnd);
        EXPECT_EQ(found->v, std::to_string(rnd));
    }
}

TEST(SortedVector, Delete)
{
    auto vector = CreateVector(100);
    const auto random_Selection = RandomSelection(vector.size(), 10);
    for (const int rnd : random_Selection)
    {
        const auto found = std::lower_bound(vector.begin(), vector.end(), rnd);
        vector.erase(found);
    }
    // check deleted items removed
    for (const int rnd : random_Selection)
        EXPECT_TRUE(!Contains(vector, rnd));
    // check size changed
    EXPECT_EQ(vector.size(), 90);
}

TEST(SortedVector, MultiDelete)
{
    auto vector = CreateVector(100);
    const auto random_Selection = RandomSelection(vector.size(), 10);
    for (const int rnd : random_Selection)
    {
        const auto found = std::lower_bound(vector.begin(), vector.end(), rnd);
        std::swap(*found, vector.back());
        vector.pop_back();
    }
    std::make_heap(vector.begin(), vector.end());
    std::sort(vector.begin(), vector.end());

    // check deleted items removed
    for (const int rnd : random_Selection)
        EXPECT_TRUE(!Contains(vector, rnd));
    // check size changed
    EXPECT_EQ(vector.size(), 90);
}