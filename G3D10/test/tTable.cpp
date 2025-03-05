/**
  \file test/tTable.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D/G3D.h"
#include "testassert.h"
#include "printhelpers.h"

using G3D::uint8;
using G3D::uint32;
using G3D::uint64;
#include <map>


class TableKey {
public:
    int value;
    
    inline bool operator==(const TableKey& other) const {
        return value == other.value;
    }
    
    inline bool operator!=(const TableKey& other) const {
        return value != other.value;
    }

    inline unsigned int hashCode() const {
        return 0;
    }
};

template <>
struct HashTrait<TableKey*> {
    static size_t hashCode(const TableKey* key) { return key->hashCode(); }
};

class TableKeyWithCustomHashStruct {
public:
    int data;
    TableKeyWithCustomHashStruct() { }
    TableKeyWithCustomHashStruct(int data) : data(data) { }
};

struct TableKeyCustomHashStruct {
    static size_t hashCode(const TableKeyWithCustomHashStruct& key) { 
        return static_cast<size_t>(key.data); 
    }
};

bool operator==(const TableKeyWithCustomHashStruct& lhs, const TableKeyWithCustomHashStruct& rhs) {
    return (lhs.data == rhs.data);
}


void testTable() {

    printf("G3D::Table  ");

    // Test ops involving HashCode / lookup for a table with a key
    // that uses a custom hashing struct
    {
        Table<TableKeyWithCustomHashStruct, int, TableKeyCustomHashStruct> table;
        bool exists;
        int val;

        table.set(1, 1);
        table.set(2, 2);
        table.set(3, 3);

        table.remove(2);

        val = table.get(3);
        testAssert(val == 3);

        exists = table.get(1, val);
        testAssert(exists && val == 1);
        exists = table.get(2, val);
        testAssert(!exists);
        exists = table.get(3, val);
        testAssert(exists && val == 3);

        exists = table.containsKey(1);
        testAssert(exists);
        exists = table.containsKey(2);
        testAssert(!exists);

        table.remove(1);
        table.remove(3);

        testAssert(table.size() == 0);
    }


    // Basic get/set
    {
        Table<int, int> table;
    
        table.set(10, 20);
        table.set(3, 1);
        table.set(1, 4);

        testAssert(table[10] == 20);
        testAssert(table[3] == 1);
        testAssert(table[1] == 4);
        testAssert(table.containsKey(10));
        testAssert(!table.containsKey(0));

        testAssert(table.debugGetDeepestBucketSize() == 1);
    }

    // Test hash collisions
    {
        TableKey        x[6];
        Table<TableKey*, int> table;
        for (int i = 0; i < 6; ++i) {
            x[i].value = i;
            table.set(x + i, i);
        }

        testAssert(table.size() == 6);
        testAssert(table.debugGetDeepestBucketSize() == 6);
        testAssert(table.debugGetNumBuckets() == 10);
    }

    // Test that all supported default key hashes compile
    {
        Table<int, int> table;
    }
    {
        Table<uint32, int> table;
    }
    {
        Table<uint64, int> table;
    }
    {
        Table<void*, int> table;
    }
    {
        Table<String, int> table;
    }
    {
        Table<const String, int> table;
    }

    {
        Table<GEvent, int> table;
    }
    {
        Table<GKey, int> table;
    }
    {
        Table<Sampler, int> table;
    }
    {
        Table<VertexBuffer*, int> table;
    }
    {
        Table<AABox, int> table;
    }
    {
        typedef _internal::Indirector<AABox> AABoxIndrctr;
        Table<AABoxIndrctr, int> table;
    }
    {
        Table<NetAddress, int> table;
    }
    {
        Table<Sphere, int> table;
    }
    {
        Table<Triangle, int> table;
    }
    {
        Table<Vector2, int> table;
    }
    {
        Table<Vector3, int> table;
    }
    {
        Table<Vector4, int> table;
    }
    {
        Table<Vector4int8, int> table;
    }
    {
        Table<WrapMode, int> table;
    }

    printf("passed\n");
}


template<class K, class V>
void perfTest(const char* description, const K* keys, const V* vals, int M) {
    Stopwatch stopwatch;
    chrono::nanoseconds tableSet, tableGet, tableRemove;
    chrono::nanoseconds mapSet, mapGet, mapRemove;

    chrono::nanoseconds overhead;

    // Run many times to filter out startup behavior
    for (int j = 0; j < 3; ++j) {

        // There's a little overhead just for the loop and reading 
        // the values from the arrays.  Take this into account when
        // counting cycles.
        stopwatch.tick();
        K k; V v;
        for (int i = 0; i < M; ++i) {
            k = keys[i];
            v = vals[i];
        }
        stopwatch.tock();
        overhead = stopwatch.elapsedDuration();

        {Table<K, V> t;
        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            t.set(keys[i], vals[i]);
        }
        stopwatch.tock();
        tableSet = stopwatch.elapsedDuration();
        
        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            v=t[keys[i]];
        }
        stopwatch.tock();
        tableGet = stopwatch.elapsedDuration();

        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            t.remove(keys[i]);
        }
        stopwatch.tock();
        tableRemove = stopwatch.elapsedDuration();
        }

        /////////////////////////////////

        {std::map<K, V> t;
        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            t[keys[i]] = vals[i];
        }
        stopwatch.tock();
        mapSet = stopwatch.elapsedDuration();
        
        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            v=t[keys[i]];
        }
        stopwatch.tock();
        mapGet = stopwatch.elapsedDuration();

        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            t.erase(keys[i]);
        }
        stopwatch.tock();
        mapRemove = stopwatch.elapsedDuration();
        }
    }

    tableSet -= overhead;
    if (tableGet < overhead) {
        tableGet = tableGet.zero();
    } else {
        tableGet -= overhead;
    }
    tableRemove -= overhead;

    mapSet -= overhead;
    mapGet -= overhead;
    mapRemove -= overhead;

    PRINT_HEADER(description);
    PRINT_TEXT("", "insert", "fetch", "remove");
    PRINT_MICRO("Table", "(us)", tableSet / M, tableGet / M, tableRemove / M);
    PRINT_MICRO("std::map", "(us)", mapSet / M, mapGet / M, mapRemove / M);
    PRINT_TEXT("Outcome", tableSet <= mapSet ? "ok" : "FAIL", tableGet <= mapGet ? "ok" : "FAIL", tableRemove < mapRemove ? "ok" : "FAIL");
}


void perfTable() {
    PRINT_SECTION("Table", "Checks performance of Table against standard library");

    const int M = 300;
    {
        int keys[M];
        int vals[M];
        for (int i = 0; i < M; ++i) {
            keys[i] = i * 2;
            vals[i] = i;
        }
        perfTest<int, int>("int,int", keys, vals, M);
    }

    {
        String keys[M];
        int vals[M];
        for (int i = 0; i < M; ++i) {
            keys[i] = format("%d", i * 2);
            vals[i] = i;
        }
        perfTest<String, int>("string, int", keys, vals, M);
    }

    {
        int keys[M];
        String vals[M];
        for (int i = 0; i < M; ++i) {
            keys[i] = i * 2;
            vals[i] = format("%d", i);
        }
        perfTest<int, String>("int, string", keys, vals, M);
    }

    {
        String keys[M];
        String vals[M];
        for (int i = 0; i < M; ++i) {
            keys[i] = format("%d", i * 2);
            vals[i] = format("%d", i);
        }
        perfTest<String, String>("string, string", keys, vals, M);
    }
}
