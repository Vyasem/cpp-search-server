#pragma once
#include <iostream>
#include <vector>
#include <iterator>

template <typename Iterator>
class IteratorRange{
public:
	IteratorRange(Iterator begin, Iterator end){
		_begin = begin;
		_end = end;
		_size = std::distance(begin, end);
	}

	Iterator begin(){
		return _begin;
	}

	Iterator end(){
		return _end;
	}

	std::size_t size(){
		return _size;
	}
private:
	Iterator _begin;
	Iterator _end;
	std::size_t _size;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& os, IteratorRange<Iterator> range){
	for(auto it = range.begin(); it != range.end(); ++it){
		os << *it;
	}
	return os;
}

template <typename Iterator>
class Paginator{
public:
	Paginator(Iterator begin, Iterator end, std::size_t pageCount){
		std::size_t size = std::distance(begin, end);
		std::size_t pages = (size % pageCount == 0) ? size / pageCount : size / pageCount + 1;
		Iterator pageIt = begin;
		for(unsigned i = 0; i < pages; i++){
			Iterator bias = std::next(pageIt, pageCount);
			if(i == (pages - 1)){
				bias = end;
			}
			IteratorRange range(pageIt, bias);
			list.push_back(range);
			pageIt = std::next(pageIt, pageCount);
		}
	}

	auto begin() const{
		return list.begin();
	}

	auto end() const{
		return list.end();
	}

	std::size_t size(){
		return list.size();
	}

private:
	std::vector<IteratorRange<Iterator>> list;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

