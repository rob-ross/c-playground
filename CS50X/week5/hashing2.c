// hashing2.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/13 12:52:02 PDT

//
// Testing performance of various hashing methods for strings
//

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_methods.h"

#include "../../roblib/dev_utils.h"

typedef struct PermuteParams {
    const HashFunc hash_func;
    const size_t num_chars;
    char * const buffer;
    const int  range_start;
    const int  range_end;
} PermuteParams;


//tracking distribution of hash function output
// bucket count, bucket array[bucket count],
// bucket index function : hash & (m -1) if power of 2, else hash % m if prime.
// metric : reduced chi-square
typedef struct HashDistribution {
    size_t   num_entries;
    double   mean_bucket_count; // average number of hashes to all buckets
    double   red_chi_squares;
    unsigned num_buckets;
    unsigned num_unused_buckets;
    unsigned max_bucket_count; // most number of hashes to a bucket
    unsigned min_bucket_count; // least number of hashes to a bucket
    //reduced chi-squares value of bucket index distribution.
    // close to 1.0 : ideal random-like distribution
    // much larger than 1.0: worse than expected, uneven
    // much smaller than 1.0: suspiciously "too smooth" (usually indicates falsified data)

    bool is_powerof2;

    unsigned bucket_counts[]; // Flexible array.
} HashDistribution;

typedef enum BucketCounts {
    _1024, _4096, _16384, _65536, _1009, _4093
} BucketCounts;

unsigned constexpr NUM_DISTRIBUTION_TESTS = 6;

static HashDistribution **distributions;



void permute_chars_and_hash(const PermuteParams params[restrict], unsigned range_index, size_t wh_index[restrict]) {

    if (range_index >= params->num_chars - 1 ) {
        //last position in permutation
        for (int c = params->range_start; c < params->range_end; ++ c) {
            params->buffer[params->num_chars - 1] = c;
            // printf("word: '%s'\n", buffer);
            size_t result = params->hash_func(params->buffer, params->num_chars);
            *wh_index += 1;
            if (*wh_index == 1'000'000'000 ) {
                printf("\n");
            }
            if (!(*wh_index % 1'000'000'000)) {
                printf("computed %'zu hashes...\n", *wh_index);
            }
        }
    } else {
        //i-th position in permutation
        for (int c = params->range_start; c < params->range_end; ++ c) {
            params->buffer[range_index] = c;
            permute_chars_and_hash(params, range_index + 1, wh_index);
        }
    }
}

//caller must call dispose_distributions() to free memory.
HashDistribution ** create_distributions(void ) {
    HashDistribution **temp = malloc( 6 * sizeof(HashDistribution*));

    unsigned bucket_sizes[] = { 1024, 4096, 16384, 65536};
    for (unsigned i = 0; i < 4; ++i ) {
        HashDistribution *temp_dist = calloc(1, sizeof(HashDistribution) + sizeof(unsigned) * bucket_sizes[i]);
        temp_dist->num_buckets = bucket_sizes[i];
        temp_dist->is_powerof2 = true;
        temp[i] = temp_dist;
    }
    size_t prime_bucket_sizes[] = { 1009, 4093 };
    for (unsigned i = 0; i < 2; ++i ) {
        HashDistribution *temp_dist = calloc( 1, sizeof(HashDistribution) + sizeof(unsigned) * prime_bucket_sizes[i]);
        temp_dist->num_buckets = prime_bucket_sizes[i];
        temp_dist->is_powerof2 = false;
        temp[i + 4] = temp_dist;
    }

    return temp;
}

void clear_distributions(HashDistribution **dist) {
    for (unsigned i = 0; i < NUM_DISTRIBUTION_TESTS; ++i ) {
        dist[i]->max_bucket_count = 0;
        dist[i]->num_unused_buckets = 0;
        dist[i]->num_entries = 0;
        dist[i]->min_bucket_count = 0;
        dist[i]->mean_bucket_count = 0;
        dist[i]->red_chi_squares = 0;
        memset(dist[i]->bucket_counts, 0, sizeof(unsigned) * dist[i]->num_buckets);
    }
}

void dispoose_distributions(HashDistribution **dist) {
    for (unsigned i = 0; i < NUM_DISTRIBUTION_TESTS; ++i ) {
        free(dist[i]);
    }
    free(dist);
}

void repr_distribution(HashDistribution *dist) {
    printf("num_entries: %'zu, num_buckets: %'6u, num_unused_buckets: %5u, max_bucket_count: %'8u, min_bucket_count: %5u, mean_bucket_count: %'9.2f, "
           "red_chi_squares: %g\n",
        dist->num_entries, dist->num_buckets, dist->num_unused_buckets, dist->max_bucket_count, dist->min_bucket_count, dist->mean_bucket_count,
        dist->red_chi_squares);

}

void calc_distribution_stats(HashDistribution *dist) {
    dist->mean_bucket_count = (double)dist->num_entries / (double)dist->num_buckets;
    unsigned min_count = dist->max_bucket_count;
    double chi_sqr_sum = 0;
    const double expected = dist->mean_bucket_count;
    const unsigned num_buckets = dist->num_buckets;
    unsigned num_used_buckets = 0;
    // printf("\nZERO count buckets: ");
    for ( unsigned i = 0; i < num_buckets; ++i ) {
        const double observed = dist->bucket_counts[i];
        const double observed_minus_expected = observed - expected;
        chi_sqr_sum = chi_sqr_sum + ( ( observed_minus_expected * observed_minus_expected ) / expected  );

        if (min_count > observed) {
            min_count = dist->bucket_counts[i];
            // printf(" %u, ", i);
        }
        if (dist->bucket_counts[i] > 0 ) {
            num_used_buckets ++;
        }
    }
    dist->num_unused_buckets = num_buckets - num_used_buckets;
    dist->min_bucket_count = min_count;
    dist->red_chi_squares = chi_sqr_sum / (num_buckets - 1);
    // printf("\n");

    // other metrics to consider:
    // coefficient of variation (stddev / mean)
    // max load relative to mean (max / E)
    //Bit-frequency tests

}


//instead of creating dummy strings, most of which are not actual words that would ever be hashed,
// we can use the wiktionary word data and assemble a list of actual English words of various sizes and
// use those for more realistic testing scenarios.
void hash_all_permutations_with(const HashFunc func, const unsigned num_chars) {
    char word[num_chars + 1];
    word[num_chars] = '\0';
    CharRange range = {.start = 97, .end = 123}; //only testing lower case letters
    size_t range_size = range.end - range.start;
    size_t num_permutations = power(range_size, num_chars);

    printf("All %u-char permutations picked from %zu chars. ", num_chars, range_size);
    printf("  %'12zu permutations. ", num_permutations);
    //set up function's parameters
    size_t wh_index = 0;
    PermuteParams params = {
        .hash_func = func,
        .num_chars = num_chars,
        .range_start = range.start,
        .range_end = range.end,
        .buffer = word,
    };
    permute_chars_and_hash(&params, 0, &wh_index);
    printf("  Hashed %'zu words. ", wh_index);
}



unsigned calc_bucket_index(size_t hash_code, unsigned num_buckets, bool is_powerof2) {
    if (is_powerof2) {
        return hash_code & ( num_buckets -1 );
    } else {
        return hash_code % num_buckets;
    }
}

void permute_chars_and_hash_gather_stats(const PermuteParams params[restrict], unsigned range_index, size_t wh_index[restrict]) {
    // like, permute_chars_and_hash2, but gathers data on hash bucket distributions
    if (range_index >= params->num_chars - 1 ) {
        //last position in permutation
        for (int c = params->range_start; c < params->range_end; ++ c) {
            params->buffer[params->num_chars - 1] = c;
            // printf("word: '%s'\n", buffer);
            size_t result = params->hash_func(params->buffer, params->num_chars);
            *wh_index += 1;
            for (unsigned i = 0; i < NUM_DISTRIBUTION_TESTS; ++i ) {
                HashDistribution *hd = distributions[i];
                unsigned bindex = calc_bucket_index( result, hd->num_buckets, hd->is_powerof2);
                hd->bucket_counts[bindex] += 1;
                hd->num_entries += 1;
                if (hd->bucket_counts[bindex] > hd->max_bucket_count ) {
                    hd->max_bucket_count = hd->bucket_counts[bindex];
                }
            }
            // if (*wh_index == 1'000'000'000 ) {
            //     printf("\n");
            // }
            // if (!(*wh_index % 1'000'000'000)) {
            //     printf("computed %'zu hashes...\n", *wh_index);
            // }
        }
    } else {
        //i-th position in permutation
        for (int c = params->range_start; c < params->range_end; ++ c) {
            params->buffer[range_index] = c;
            permute_chars_and_hash_gather_stats(params, range_index + 1, wh_index);
        }
    }

}



void hash_distribution_stats_for(const HashFunc func, const unsigned num_chars) {
    char word[num_chars + 1];
    word[num_chars] = '\0';
    CharRange range = {.start = 97, .end = 123}; //only testing lower case letters
    size_t range_size = range.end - range.start;
    size_t num_permutations = power(range_size, num_chars);

    printf("All %u-char permutations picked from %zu chars. ", num_chars, range_size);
    printf("  %'12zu permutations.\n", num_permutations);
    //set up function's parameters
    size_t wh_index = 0;
    PermuteParams params = {
        .hash_func = func,
        .num_chars = num_chars,
        .range_start = range.start,
        .range_end = range.end,
        .buffer = word,
    };


    distributions = create_distributions();

    permute_chars_and_hash_gather_stats(&params, 0, &wh_index);

    // printf("  Hashed %'zu words. ", wh_index);


    for (unsigned i = 0; i < NUM_DISTRIBUTION_TESTS; ++i ) {
        calc_distribution_stats(distributions[i]);
        repr_distribution(distributions[i]);
    }
    //compute chi squares, display, then free distributions

    dispoose_distributions(distributions);

}






















void time_all_hash_methods(void) {
    // Macro to pair the function pointer with its string name
    #define HASH_ENTRY(f) { f, #f }

    struct {
        HashFunc func;
        const char *name;
    } funcs[] = {
        HASH_ENTRY(fnv1a_hash),
        HASH_ENTRY(crc32_hash),
        HASH_ENTRY(djb2_hash_string),
        HASH_ENTRY(murmur_hash3),
        // HASH_ENTRY(rob_hash)
    };

    constexpr unsigned num_funcs = sizeof(funcs) / sizeof(funcs[0]);

    for (unsigned f = 0; f < num_funcs; ++f ) {
        printf("\n\nTesting function: %s\n", funcs[f].name);
        printf("----------------------------------------------------------------------\n");
        double total_elapsed = 0;
        for (int i = 1; i < 6; ++i ) {
            // total_elapsed += TIMEIT(hash_all_permutations_with2(funcs[f].func, i));
            hash_distribution_stats_for(funcs[f].func, i);
            printf("\n");
        }
        printf("Total elapsed time: %.6f s\n", total_elapsed);
    }
}


// make:

//DEBUG:
//  clang -std=c23 -o ./hashing2.out hashing2.c hash_methods.c ../../roblib/dev_utils.c
//
//RELEASE:
// clang -std=c23 -O3 -march=native -DNDEBUG -o ./hashing2.out hashing2.c hash_methods.c ../../roblib/dev_utils.c

int main(int argc, char *argv[]) {
    setlocale(LC_NUMERIC, "en_US.UTF-8");   // use user's system locale

    time_all_hash_methods();

}
