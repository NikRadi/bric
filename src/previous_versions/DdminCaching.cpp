#include "DeltaDebugging.hpp"
#include <cassert>
#include <algorithm>


#define NO_CACHE 0
#define PERFECT_HASH 1
#define COLLIDING_HASH 2
#define HASH_TYPE COLLIDING_HASH


static int num_tests = 0;
static int num_cache_hits = 0;
static int num_cache_collisions = 0;


#if HASH_TYPE == PERFECT_HASH
// vectors are pointers to avoid warnings
static std::vector<std::vector<bool>> *caches;
static std::vector<bool> *cache_results;
#elif HASH_TYPE == COLLIDING_HASH
static std::vector<int> *caches;
static std::vector<bool> *cache_results;
static std::vector<std::vector<bool>> *combinations;
#endif

#if HASH_TYPE != NO_CACHE
static const std::vector<Ast *> *all;
#endif

#if HASH_TYPE == PERFECT_HASH
static std::vector<bool> CalculateHash() {
    std::vector<bool> hash;
    for (auto unit : *all) {
        hash.push_back(unit->is_active);
    }

    return hash;
}
#elif HASH_TYPE == COLLIDING_HASH
static int CalculateHash() {
    int hash = 1;
    for (auto unit : *all) {
        hash += (hash << 1) | unit->is_active;
        hash = hash % ((1 << 31) - 1);
    }

    return hash;
}
#endif

#if HASH_TYPE == PERFECT_HASH
static int IsCached(std::vector<bool> &hash) {
    for (size_t i = 0; i < caches->size(); ++i) {
        if (hash == (*caches)[i]) {
            num_cache_hits += 1;
            return (*cache_results)[i];
        }
    }

    return -1;
}
#elif HASH_TYPE == COLLIDING_HASH
static int IsCached(int hash) {
    // printf("\nsearching\n");
    for (size_t i = 0; i < caches->size(); ++i) {
        // printf("%d %d\n", hash, (*caches)[i]);
        if (hash == (*caches)[i]) {
            std::vector<bool> current_combination;
            for (auto unit : *all) {
                current_combination.push_back(unit->is_active);
            }

            for (auto combination : *combinations) {
                if (combination == current_combination) {
                    num_cache_hits += 1;
                    return (*cache_results)[i];
                }
            }

            num_cache_collisions += 1;
            break;
        }
    }

    return -1;
}
#endif

#if HASH_TYPE != NO_CACHE
static bool _IsTestSuccessful(Ast *r, std::string f, std::string p) {
    num_tests += 1;
#if HASH_TYPE == PERFECT_HASH
    std::vector<bool> hash = CalculateHash();
#else // COLLIDING_HASH
    int hash = CalculateHash();
#endif
    int result = IsCached(hash);
    if (result != -1) {
        return result;
    }

    result = IsTestSuccessful(r, f, p);
    caches->push_back(hash);
    cache_results->push_back(result);
#if HASH_TYPE == COLLIDING_HASH
    std::vector<bool> combination;
    for (auto unit : *all) {
        combination.push_back(unit->is_active);
    }

    combinations->push_back(combination);
#endif

    return result;
}
#else
static bool _IsTestSuccessful(Ast *r, std::string f, std::string p) {
    num_tests += 1;
    return IsTestSuccessful(r, f, p);
}
#endif // HASH_TYPE != NO_CACHE

static size_t Max(size_t a, size_t b) {
    return (a > b) ? a : b;
}

static size_t Min(size_t a, size_t b) {
    return (a > b) ? b : a;
}

static std::vector<std::vector<Ast *>> Partition(std::vector<Ast *> units, size_t n) {
    assert(units.size() >= n);
    std::vector<std::vector<Ast *>> result;

    size_t size = units.size() / n;
    size_t remain = units.size() % n;

    long long begin = 0;
    long long end = 0;

    for (size_t i = 0; i < n; ++i) {
        end += (remain > 0) ? (size + !!(remain--)) : size;
        std::vector<Ast *> part(units.begin() + begin, units.begin() + end);
        result.push_back(part);
        begin = end;
    }

    return result;
}

static void SetActive(std::vector<Ast *> &part, bool is_active) {
    for (auto unit : part) {
        unit->is_active = is_active;
    }
}

static void DeltaDebugging(std::vector<Ast *> units, size_t n, Ast *r, std::string f, std::string p) {
    if (units.size() == 1) {
        units[0]->is_active = false;
        if (!_IsTestSuccessful(r, f, p)) {
            units[0]->is_active = true;
        }

        return;
    }

    std::vector<std::vector<Ast *>> partitions = Partition(units, n);
    for (auto part : partitions) {
        SetActive(part, false);
    }

    for (auto part : partitions) {
        SetActive(part, true);
        if (_IsTestSuccessful(r, f, p)) {
            DeltaDebugging(part, 2, r, f, p);
            return;
        }

        SetActive(part, false);
    }

    for (auto part : partitions) {
        SetActive(part, true);
    }

    if (n > 2) {
        for (size_t i = 0; i < partitions.size(); ++i) {
            std::vector<Ast *> &part = partitions[i];
            SetActive(part, false);
            if (_IsTestSuccessful(r, f, p)) {
                std::vector<Ast *> new_partitions;
                for (size_t j = 0; j < partitions.size(); ++j) {
                    if (j == i) continue;
                    for (auto unit : partitions[j]) {
                        new_partitions.push_back(unit);
                    }
                }

                n = Max(n - 1, 2ull);
                DeltaDebugging(new_partitions, n, r, f, p);
                return;
            }

            SetActive(part, true);
        }
    }

    if (n < units.size()) {
        DeltaDebugging(units, Min(units.size(), 2 * n), r, f, p);
    }
}

void DeltaDebugging(std::vector<Ast *> units, Ast *r, std::string f, std::string p) {
#if HASH_TYPE == PERFECT_HASH
    std::vector<std::vector<bool>> _caches;
#elif HASH_TYPE == COLLIDING_HASH
    std::vector<int> _caches;
    std::vector<std::vector<bool>> _combinations;
    combinations = &_combinations;
#endif

#if HASH_TYPE != NO_CACHE
    std::vector<bool> _caches_results;
    cache_results = &_caches_results;
    caches = &_caches;
    all = &units;
#endif

    DeltaDebugging(units, 2, r, f, p);
    WriteToFile(r, f);
    printf("cache enabled: %d\n", HASH_TYPE);
    printf("units:         %zu\n", units.size());
    printf("tests:         %d\n", num_tests);
    printf("cache hits:    %d\n", num_cache_hits);
    printf("coliisions:    %d\n", num_cache_collisions);
}
