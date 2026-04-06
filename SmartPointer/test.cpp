#include "Shared_ptr.h"

#include <cassert>
#include <string>

struct Counter {
    static int destroyed;
    int value;
    std::string tag;

    Counter(int v, std::string t) : value(v), tag(std::move(t)) {}
    ~Counter() { ++destroyed; }
};

int Counter::destroyed = 0;

int main() {
    Counter::destroyed = 0;

    {
        Ref_count<Counter> p(new Counter(7, "hello"));
        assert(p.get() != nullptr);
        assert(p.get_count() == 1);
        assert((*p).value == 7);
        assert(p->tag == "hello");

        {
            Ref_count<Counter> p2(p);
            assert(p.get_count() == 2);
            assert(p2.get_count() == 2);
            assert(p2->value == 7);
        }

        assert(p.get_count() == 1);
        assert(Counter::destroyed == 0);
    }

    assert(Counter::destroyed == 1);

    Counter::destroyed = 0;
    {
        Ref_count<Counter> a(new Counter(1, "a"));
        Ref_count<Counter> b(new Counter(2, "b"));

        b = a;
        assert(a.get_count() == 2);
        assert(b.get_count() == 2);
        assert(b->value == 1);
        assert(Counter::destroyed == 1);

        a = a;
        assert(a.get_count() == 2);
    }
    assert(Counter::destroyed == 2);

    {
        Ref_count<Counter> empty(nullptr);
        assert(empty.get() == nullptr);
        assert(empty.get_count() == 0);

        Ref_count<Counter> empty2(empty);
        assert(empty2.get() == nullptr);
        assert(empty2.get_count() == 0);
    }

    return 0;
}

