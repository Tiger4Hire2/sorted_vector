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
    // used for searching for a specific key value
    friend bool operator<(const Elem &a, int b) noexcept { return a.k < b; }
    friend bool operator==(const Elem &a, int b) noexcept { return a.k == b; }
};

using MyVector = std::vector<Elem>;

// could use lower_bound for speed, but this is used for testing, so avoid checking assumptions with the same assumptions
bool Contains(const MyVector &vector, int val)
{
    const auto found = std::find(vector.begin(), vector.end(), val);
    return found != vector.end();
}


// batch erase a set of pre-found pointers
static void BatchErase(MyVector &vector, const std::vector<MyVector::iterator>& found_list)
{
    assert(std::is_sorted(found_list.begin(), found_list.end()));
    MyVector::iterator copy_from = found_list[0]+1;
    MyVector::iterator copy_to = found_list[0];
    auto next_to_skip = found_list.begin()+1;
    while (true)
    {
        while (next_to_skip!=found_list.end() && *next_to_skip==copy_from)
        {
            ++next_to_skip;
            ++copy_from;
        }
        if (copy_from!=vector.end())
            *copy_to++ = *copy_from++;
        else break;
    }
    vector.resize(vector.size()-found_list.size());
}
/*
BatchDelete: 
 remove a set of elements from a sorted vector as a single op
 Uses 
*/
// Precondition: all elements of selection must exist in vector
static void BatchDelete(MyVector &vector, std::vector<int> selection)
{
    assert(!selection.empty());
    std::sort(selection.begin(), selection.end());
    MyVector::iterator start = vector.begin();
    MyVector::iterator end = vector.end();
    std::vector<MyVector::iterator> found_list;
    found_list.reserve(selection.size());
    for (const int rnd : selection)
    {
        const auto found = std::lower_bound(start, end, rnd);
        found_list.emplace_back(found);
        start = found + 1;
    }
    BatchErase(vector, found_list);
}

static void BatchInsert(MyVector &vector, std::vector<Elem> selection)
{
    vector.insert(vector.end(), selection.begin(), selection.end());
    std::sort(vector.begin(), vector.end());
}

static void BatchInsertMagic(MyVector &vector, std::vector<Elem> selection)
{
    vector.insert(vector.end(), selection.begin(), selection.end());
    std::make_heap(vector.begin(), vector.end());
    std::sort(vector.begin(), vector.end());
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
        retval.insert(std::make_pair(num, std::to_string(num % 100)));
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
        retval.emplace_back(Elem{num, std::to_string(num % 100)});
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
    benchmark::ClobberMemory();
}

static void MapLookup(benchmark::State &state)
{
    static auto map = CreateMap(state.range(0));
    if (map.size() != (size_t)state.range(0))
        map = CreateMap(state.range(0));
    const auto random_Selection = RandomSelection(state.range(0), 100);
    for (auto _ : state)
    {
        for (const int rnd : random_Selection)
            benchmark::DoNotOptimize(map.lower_bound(rnd));
    }
}

static void VectorLookup(benchmark::State &state)
{
    auto vector = CreateVector(state.range(0));
    if (vector.size() != (size_t)state.range(0))
        vector = CreateVector(state.range(0));
    const auto random_Selection = RandomSelection(state.range(0), 100);
    for (auto _ : state)
    {
        for (const int rnd : random_Selection)
            benchmark::DoNotOptimize(std::lower_bound(vector.begin(), vector.end(), rnd));
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
        }
    }
}

static void MapDeleteHalf(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto map = CreateMap(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), state.range(0) / 2);
        state.ResumeTiming();
        for (const int rnd : random_Selection)
        {
            const MyMap::iterator pos = map.lower_bound(rnd);
            map.erase(pos);
        }
    }
}

static void VectorMultiDelete(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto vector = CreateVector(state.range(0));
        const auto random_selection = RandomSelection(state.range(0), 100);
        state.ResumeTiming();
        BatchDelete(vector, random_selection);
    }
}

static void VectorMultiDeleteHalf(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto vector = CreateVector(state.range(0));
        const auto random_selection = RandomSelection(state.range(0), state.range(0) / 2);
        state.ResumeTiming();
        BatchDelete(vector, random_selection);
    }
}


static void MapInsert(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto map = CreateMap(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), 100);
        for (const int rnd : random_Selection)
        {
            const MyMap::iterator pos = map.lower_bound(rnd);
            map.erase(pos);
        }
        state.ResumeTiming();
        for (const int rnd : random_Selection)
            map.insert(std::make_pair(rnd, ""s));
    }
}


static void VectorBatchInsert(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto vector = CreateVector(state.range(0));
        const auto random_selection = RandomSelection(state.range(0), 100);
        BatchDelete(vector, random_selection);
        std::vector<Elem> new_elems{random_selection.size()};
        std::transform(random_selection.begin(), random_selection.end(), new_elems.begin(), [](int r)
                       { return Elem{r, std::to_string(r)}; });
        state.ResumeTiming();

        BatchInsert(vector, new_elems);
    }
}

static void MapInsertHalf(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto map = CreateMap(state.range(0));
        const auto random_Selection = RandomSelection(state.range(0), state.range(0) / 2);
        for (const int rnd : random_Selection)
        {
            const MyMap::iterator pos = map.lower_bound(rnd);
            map.erase(pos);
        }
        state.ResumeTiming();
        for (const int rnd : random_Selection)
            map.insert(std::make_pair(rnd, ""s));
    }
}

static void VectorBatchInsertHalf(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto vector = CreateVector(state.range(0));
        const auto random_selection = RandomSelection(state.range(0), state.range(0) / 2);
        BatchDelete(vector, random_selection);
        std::vector<Elem> new_elems{random_selection.size()};
        std::transform(random_selection.begin(), random_selection.end(), new_elems.begin(), [](int r)
                       { return Elem{r, std::to_string(r)}; });
        state.ResumeTiming();

        BatchInsert(vector, new_elems);
    }
}

static void VectorBatchInsertHalfMagic(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        auto vector = CreateVector(state.range(0));
        const auto random_selection = RandomSelection(state.range(0), state.range(0) / 2);
        BatchDelete(vector, random_selection);
        std::vector<Elem> new_elems{random_selection.size()};
        std::transform(random_selection.begin(), random_selection.end(), new_elems.begin(), [](int r)
                       { return Elem{r, std::to_string(r)}; });
        state.ResumeTiming();

        BatchInsertMagic(vector, new_elems);
    }
}

constexpr int MAX = 16;
BENCHMARK(MapCreation)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
BENCHMARK(VectorCreation)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);

BENCHMARK(MapLookup)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
BENCHMARK(VectorLookup)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);

BENCHMARK(MapDelete)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
BENCHMARK(VectorDelete)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
BENCHMARK(VectorMultiDelete)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);

BENCHMARK(MapDeleteHalf)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
BENCHMARK(VectorMultiDeleteHalf)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);

BENCHMARK(MapInsert)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
BENCHMARK(VectorBatchInsert)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);

BENCHMARK(MapInsertHalf)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
BENCHMARK(VectorBatchInsertHalf)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
BENCHMARK(VectorBatchInsertHalfMagic)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024);
//BENCHMARK(VectorDeleteHalf)->RangeMultiplier(4)->Range(1024 * 256, MAX * 1024 * 1024); this is just masocistic

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
        EXPECT_FALSE(Contains(vector, rnd));
    // check size changed
    EXPECT_EQ(vector.size(), 90);
}

TEST(SortedVector, BatchErase)
{
    // delete end
    {
        auto vector = CreateVector(5);
        using DeleteVector = std::vector<MyVector::iterator>;
        const DeleteVector to_delete= { 
                                vector.begin(),
                                vector.begin()+2,
                                vector.begin()+4 
                            };
        BatchErase(vector, to_delete);
        EXPECT_EQ(vector.size(), 2);
        EXPECT_EQ(vector[0], 1);
        EXPECT_EQ(vector[1], 3);
    }
    // don't delete end
    {
        auto vector = CreateVector(5);
        using DeleteVector = std::vector<MyVector::iterator>;
        const DeleteVector to_delete= { 
                                vector.begin(),
                                vector.begin()+2,
                                vector.begin()+3 
                            };
        BatchErase(vector, to_delete);
        EXPECT_EQ(vector.size(), 2);
        EXPECT_EQ(vector[0], 1);
        EXPECT_EQ(vector[1], 4);
    }
}

TEST(SortedVector, MultiDelete)
{
    auto vector = CreateVector(100);
    const auto random_selection = RandomSelection(vector.size(), 10);
    BatchDelete(vector, random_selection);
    // check deleted items removed
    EXPECT_TRUE(std::is_sorted(vector.begin(), vector.end()));
    for (const int rnd : random_selection)
        EXPECT_FALSE(Contains(vector, rnd));
    // check size changed
    EXPECT_EQ(vector.size(), 90);
}

TEST(SortedVector, MultiDeleteBug)
{
    auto vector = CreateVector(100);
    const auto selection = std::vector{1,10,98,99};
    BatchDelete(vector, selection);
    // check deleted items removed
    EXPECT_TRUE(std::is_sorted(vector.begin(), vector.end()));
    for (const int rnd : selection)
        EXPECT_FALSE(Contains(vector, rnd));
    // check size changed
    EXPECT_EQ(vector.size(), 96);
}

TEST(SortedVector, Insert)
{
    auto vector = CreateVector(100);
    const auto random_selection = RandomSelection(vector.size(), 10);
    BatchDelete(vector, random_selection);
    std::vector<Elem> new_elems{random_selection.size()};
    std::transform(random_selection.begin(), random_selection.end(), new_elems.begin(), [](int r)
                    { return Elem{r, std::to_string(r)}; });
    BatchInsert(vector, new_elems);
   // check deleted items removed
    EXPECT_TRUE(std::is_sorted(vector.begin(), vector.end()));
    for (const int rnd : random_selection)
        EXPECT_TRUE(Contains(vector, rnd));
    // check size changed
    EXPECT_EQ(vector.size(), 100);
}

TEST(SortedVector, InsertMagic)
{
    auto vector = CreateVector(100);
    const auto random_selection = RandomSelection(vector.size(), 10);
    BatchDelete(vector, random_selection);
    std::vector<Elem> new_elems{random_selection.size()};
    std::transform(random_selection.begin(), random_selection.end(), new_elems.begin(), [](int r)
                    { return Elem{r, std::to_string(r)}; });
    BatchInsertMagic(vector, new_elems);
   // check deleted items removed
    EXPECT_TRUE(std::is_sorted(vector.begin(), vector.end()));
    for (const int rnd : random_selection)
        EXPECT_TRUE(Contains(vector, rnd));
    // check size changed
    EXPECT_EQ(vector.size(), 100);
}