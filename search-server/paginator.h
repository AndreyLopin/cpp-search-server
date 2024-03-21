#pragma once

#include <iostream>
#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator range_begin, Iterator range_end) :
                  begin_(range_begin),
                  end_(range_end),
                  size_(distance(range_begin, range_end)) {

    };

    Iterator begin() const {
        return begin_;
    };

    Iterator end() const {
        return end_;
    };

    size_t size() const {
        return size_;
    };
private:
    Iterator begin_;
    Iterator end_;
    size_t size_;
};

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator range_begin, Iterator range_end, size_t page_size) {
        Iterator start_page = range_begin;
        while(start_page != range_end ) {
            pages_.push_back({start_page, start_page + (static_cast<size_t>(distance(start_page, range_end))
                            < page_size ? distance(start_page, range_end) : page_size)});
            advance(start_page, static_cast<size_t>(distance(start_page, range_end))
                    < page_size ? distance(start_page, range_end) : page_size);
        }
    };

    auto begin() const {
        return pages_.begin();
    };

    auto end() const {
        return pages_.end();
    };

    auto size() const {
        return pages_.size();
    };
private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template<typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range) {
    for (Iterator it = range.begin(); it != range.end(); ++it) {
        out << *it;
    }
    return out;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}