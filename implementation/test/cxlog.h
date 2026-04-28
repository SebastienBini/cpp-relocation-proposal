/// cxlog.h — constexpr event log for tracking ctor/dtor calls at compile time.
///
/// Usage:
///   struct S {
///       cxlog* log; char id; int val;
///       constexpr S(cxlog& l, char c, int v) : log(&l), id(c), val(v)
///           { log->record(id, cxlog::CTOR, this); }
///       constexpr S(S reloc src) : log(src.log), id(src.id), val(src.val)
///           { log->record(id, cxlog::RELOC, this, &src); }
///       constexpr ~S() { log->record(id, cxlog::DTOR, this); }
///   };
///   constexpr int test() {
///       cxlog log;
///       { S s(log, 'S', 10); S t = reloc s; }
///       return log.match("S() 0x1; ~S() 0x1");
///   }
///   static_assert(test() == 1, "");  // elided: same address, no reloc ctor
///
/// Events are separated by '; ' and formatted like snoop.h with normalized
/// addresses:
///   "S() 0x1"              — ctor, this=0x1
///   "S(S reloc) 0x2 <- 0x1" — reloc ctor, this=0x2, src=0x1
///   "~S() 0x1"             — dtor, this=0x1
///
/// Addresses are normalized to 0x1, 0x2, … in order of first appearance,
/// exactly like the test runner's normalize_addresses() for snoop output.
#pragma once

#ifndef __clang__
#include <iostream>
#endif

struct cxlog {
    enum Kind : char { CTOR, RELOC, MOVE, COPY, DTOR };

    struct Event {
        char id;
        Kind kind;
        const void* self;   // this pointer
        const void* src;    // source pointer (copy/move/reloc), nullptr otherwise
    };

    static constexpr int MAX_EVENTS = 64;
    static constexpr int MAX_ADDRS  = 32;

    Event events[MAX_EVENTS] = {};
    int count = 0;

    constexpr void record(char id, Kind k, const void* self,
                          const void* src = nullptr) {
        events[count++] = {id, k, self, src};
    }

    // --- Address normalization table (built during formatting) ---
    struct AddrMap {
        const void* ptrs[MAX_ADDRS] = {};
        int n = 0;

        // Return 1-based index for ptr, assigning a new one if unseen.
        constexpr int id_of(const void* ptr) {
            for (int i = 0; i < n; ++i)
                if (ptrs[i] == ptr) return i + 1;
            ptrs[n++] = ptr;
            return n;
        }
    };

    // Write "0x" followed by the decimal digits of num into buf.
    static constexpr int fmt_addr(int num, char* buf) {
        buf[0] = '0'; buf[1] = 'x';
        int n = 2;
        // Decimal digits (num is small, max ~32).
        if (num == 0) { buf[n++] = '0'; return n; }
        char tmp[8]; int tn = 0;
        while (num > 0) { tmp[tn++] = '0' + (num % 10); num /= 10; }
        for (int i = tn - 1; i >= 0; --i) buf[n++] = tmp[i];
        return n;
    }

    // Format one event WITHOUT addresses (for constexpr static_assert).
    static constexpr int fmt_op(const Event& e, char* buf) {
        int n = 0;
        bool has_src = (e.kind == RELOC || e.kind == MOVE || e.kind == COPY);
        if (e.kind == DTOR)
            buf[n++] = '~';
        buf[n++] = e.id;
        buf[n++] = '(';
        if (has_src) {
            buf[n++] = e.id;
            switch (e.kind) {
            case RELOC:
                for (const char* s = " reloc"; *s; ) buf[n++] = *s++;
                break;
            case MOVE:
                buf[n++] = '&'; buf[n++] = '&';
                break;
            case COPY:
                for (const char* s = " const&"; *s; ) buf[n++] = *s++;
                break;
            default: break;
            }
        }
        buf[n++] = ')';
        return n;
    }

    // Format one event WITH normalized addresses (for runtime dump).
    static constexpr int fmt(const Event& e, AddrMap& am, char* buf) {
        int n = 0;
        bool has_src = (e.kind == RELOC || e.kind == MOVE || e.kind == COPY);

        // "~" prefix for dtor
        if (e.kind == DTOR)
            buf[n++] = '~';

        // "X(" or "~X("
        buf[n++] = e.id;
        buf[n++] = '(';

        // Argument type for copy/move/reloc ctors
        if (has_src) {
            buf[n++] = e.id;
            switch (e.kind) {
            case RELOC:
                for (const char* s = " reloc"; *s; ) buf[n++] = *s++;
                break;
            case MOVE:
                buf[n++] = '&'; buf[n++] = '&';
                break;
            case COPY:
                for (const char* s = " const&"; *s; ) buf[n++] = *s++;
                break;
            default: break;
            }
        }

        buf[n++] = ')';
        buf[n++] = ' ';

        // "0xN" — this address
        n += fmt_addr(am.id_of(e.self), buf + n);

        // " <- 0xM" — source address
        if (has_src && e.src) {
            for (const char* s = " <- "; *s; ) buf[n++] = *s++;
            n += fmt_addr(am.id_of(e.src), buf + n);
        }

        return n;
    }

    // Build the full trace WITH addresses and compare against expected string.
    // Returns 1 on match, 0 on mismatch.
    constexpr int match(const char* expected) const {
        char buf[1024] = {};
        AddrMap am;
        int pos = 0;
        for (int i = 0; i < count; ++i) {
            if (i > 0) {
                buf[pos++] = ';';
                buf[pos++] = ' ';
            }
            pos += fmt(events[i], am, buf + pos);
        }
        buf[pos] = '\0';
        for (int i = 0; i <= pos; ++i)
            if (buf[i] != expected[i]) return 0;
        return 1;
    }

    // Match event sequence WITHOUT addresses (safe for constexpr static_assert).
    // Compares against strings like "S(); S(S reloc); ~S()" or "A(); B(); ~B(); ~A()".
    constexpr int match_ops(const char* expected) const {
        char buf[1024] = {};
        int pos = 0;
        for (int i = 0; i < count; ++i) {
            if (i > 0) {
                buf[pos++] = ';';
                buf[pos++] = ' ';
            }
            pos += fmt_op(events[i], buf + pos);
        }
        buf[pos] = '\0';
        for (int i = 0; i <= pos; ++i)
            if (buf[i] != expected[i]) return 0;
        return 1;
    }

    // Build trace string into caller-provided buffer.
    constexpr int trace(char* buf, int bufsize) const {
        AddrMap am;
        int pos = 0;
        for (int i = 0; i < count; ++i) {
            if (i > 0 && pos + 2 < bufsize) {
                buf[pos++] = ';';
                buf[pos++] = ' ';
            }
            if (pos + 40 < bufsize)
                pos += fmt(events[i], am, buf + pos);
        }
        if (pos < bufsize)
            buf[pos] = '\0';
        return pos;
    }

#ifndef __clang__
    void dump() const {
        char buf[1024];
        trace(buf, sizeof(buf));
        std::cout << buf;
    }
#else
    void dump() const {
        char buf[1024];
        trace(buf, sizeof(buf));
        __builtin_printf("%s", buf);
    }
#endif
};
