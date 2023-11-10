#include <benchmark/benchmark.h>

#include "utils/array.h"

#include <string>

struct Date
{
    int day, month, year;
};

struct Person
{
    ::std::string name;
    Date date_of_birth;
};

template<typename T>
class ArrayFixture : public ::benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State& state) override
    {
        //array.Reserve(state.range(0));
    }

public:
    ::Utils::Array<T> array;
};

template<typename T>
class VectorFixture : public ::benchmark::Fixture
{
public:
    void SetUp(const ::benchmark::State& state)
    {
        //vector.reserve(state.range(0));
    }

public:
    ::std::vector<T> vector;
};

BENCHMARK_TEMPLATE_DEFINE_F(ArrayFixture, ArrayIntPushDifferentTest, int)(::benchmark::State& st)
{
    for(auto _ : st)
    {
        for(int i = 0; i < st.range(0); i++)
            array.Push(i);

        //array.Clear();
    }
}

BENCHMARK_TEMPLATE_DEFINE_F(ArrayFixture, ArrayPersonPushDifferentTest, Person)(::benchmark::State& st)
{
    for(auto _ : st)
    {
        for(int i = 0; i < st.range(0); i++)
            array.Push("Jackson", Date{ .day = i, .month = i / 2, .year = i / 4 });
    
        //array.Clear();
    }
}

BENCHMARK_TEMPLATE_DEFINE_F(VectorFixture, VectorIntPushDifferentTest, int)(::benchmark::State& st)
{
    for(auto _ : st)
    {
        for(int i = 0; i < st.range(0); i++)
            vector.emplace_back(i);

        //vector.clear();
    }
}

BENCHMARK_TEMPLATE_DEFINE_F(VectorFixture, VectorPersonPushDifferentTest, Person)(::benchmark::State& st)
{
    for(auto _ : st)
    {
        for(int i = 0; i < st.range(0); i++)
            vector.emplace_back("Jackson", Date{ .day = i, .month = i / 2, .year = i / 4 });

        //vector.clear();
    }
}

BENCHMARK_REGISTER_F(ArrayFixture, ArrayIntPushDifferentTest)->Arg(1'000'000);
BENCHMARK_REGISTER_F(VectorFixture, VectorIntPushDifferentTest)->Arg(1'000'000);

BENCHMARK_REGISTER_F(ArrayFixture, ArrayPersonPushDifferentTest)->Arg(100'000);
BENCHMARK_REGISTER_F(VectorFixture, VectorPersonPushDifferentTest)->Arg(100'000);

BENCHMARK_MAIN();

