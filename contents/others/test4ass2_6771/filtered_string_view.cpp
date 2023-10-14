#include "./filtered_string_view.h"

// Implement here

namespace fsv{
	filter filtered_string_view::default_predicate{[](const char &)->bool { return true; }};
	// default constructors
	filtered_string_view::filtered_string_view() noexcept= default;
	// implicit string constructor
	filtered_string_view::filtered_string_view(const std::string &str) noexcept : ptr_{str.data()}, len_{str.size()} {}
	// predicate constructor
	filtered_string_view::filtered_string_view(const std::string &str, filter predicate) noexcept :
	    ptr_{str.data()}, len_{str.size()}, predicate_func_{predicate}{}
	// implicit Null-terminated string constructor
	filtered_string_view::filtered_string_view(const char *str) noexcept : ptr_{str}, len_{std::strlen(str)} {}
	// Null-terminated constructor
	filtered_string_view::filtered_string_view(const char *str, filter predicate) noexcept :
	    ptr_{str}, len_{std::strlen(str)}, predicate_func_{predicate} {}
    // copy constructor
    filtered_string_view::filtered_string_view(const filtered_string_view &other) noexcept {
        ptr_ = other.ptr_;
        len_ = other.len_;
        predicate_func_ = other.predicate_func_;
    };
    // move constructor
    filtered_string_view::filtered_string_view(filtered_string_view &&other) noexcept {
        ptr_ = other.ptr_;
        len_ = other.len_;
        predicate_func_ = other.predicate_func_;
        other.ptr_ = nullptr;
        other.len_ = 0;
        other.predicate_func_ = default_predicate;
    };

	// copy assignment
	auto filtered_string_view::operator=(const filtered_string_view & other) noexcept -> filtered_string_view & = default;
	// move assignment
	auto filtered_string_view::operator=(filtered_string_view && other) noexcept -> filtered_string_view & {
        ptr_ = other.ptr_;
        len_ = other.len_;
        predicate_func_ = other.predicate_func_;
        other.ptr_ = nullptr;
        other.len_ = 0;
        other.predicate_func_ = default_predicate;
        return *this;
    };

	// destructor
	filtered_string_view::~filtered_string_view() noexcept = default;

	// Subscript
	auto filtered_string_view::operator[](int n)  const -> const char &{
		if (n >= static_cast<int>(len_) || n < 0 || ptr_ == nullptr) {
			throw std::domain_error{"filtered_string_view::at(<index>): invalid index"};
		}
		std::size_t index_ = 0;
		for (std::size_t i = 0; i < len_; ++i) {
			if (predicate_func_(ptr_[i])) {
				if (static_cast<int>(index_) == n) {
					const char &res_ = ptr_[i];
					return res_;
				}
				++index_;
			}
		}
		throw std::domain_error{"filtered_string_view::at(<index>): invalid index"};
	}

	// std::string conversion
	[[nodiscard]] filtered_string_view::operator std::string() const noexcept {
		if (ptr_ == nullptr) {
			return std::string{};
		}
		std::string res_;
		for (std::size_t i = 0; i < len_; ++i) {
			if (predicate_func_(ptr_[i])) {
				res_ += ptr_[i];
			}
		}
		return res_;
	}

	// member functions
	// Throws: a std::domain_error{"filtered_string_view::at(<index>): invalid index"},
	// where <index> should be replaced with the actual index passed in if the index is invalid.
	[[nodiscard]] auto filtered_string_view::at(int n) const -> const char&{
		if (n >= static_cast<int>(len_) || n < 0 || ptr_ == nullptr) {
			throw std::domain_error{"filtered_string_view::at(<index>): invalid index"};
		}
		std::size_t index_ = 0;
		for (std::size_t i = 0; i < len_; ++i) {
			if (predicate_func_(ptr_[i])) {
				if (static_cast<int>(index_) == n) {
					const char &res_ = ptr_[i];
					return res_;
				}
				++index_;
			}
		}
		throw std::domain_error{"filtered_string_view::at(<index>): invalid index"};
	}

	[[nodiscard]] auto filtered_string_view::size() const noexcept-> std::size_t{
		if (ptr_ == nullptr) {
			return 0;
		}
		std::size_t res_ = 0;
		for (std::size_t i = 0; i < len_; ++i) {
			if (predicate_func_(ptr_[i])) {
				++res_;
			}
		}
		return res_;
	}

	auto filtered_string_view::empty() const noexcept -> bool{
		if (ptr_ == nullptr) {
			return true;
		}
		for (std::size_t i = 0; i < len_; ++i) {
			if (predicate_func_(ptr_[i])) {
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] auto filtered_string_view::data() const noexcept -> const char*{
		return ptr_;
	}

	[[nodiscard]] auto filtered_string_view::predicate() const noexcept -> const filter &{
		const filter & res_ = predicate_func_;
		return res_;
	}

    auto operator==(const filtered_string_view &lhs, const filtered_string_view &rhs) -> bool{
        if (lhs.size() != rhs.size()) {
            return false;
        }
        for (int i = 0; i < static_cast<int>(lhs.size()); ++i) {
            if (lhs.at(i) != rhs.at(i)) {
                return false;
            }
        }
        return true;
    }

	auto operator<=>(const filtered_string_view &lhs, const filtered_string_view &rhs) -> std::strong_ordering{
		auto size_ = std::min(lhs.size(), rhs.size());
		for (int i = 0; i < static_cast<int>(size_); ++i) {
			if (lhs.at(i) != rhs.at(i)) {
				return lhs.at(i) <=> rhs.at(i);
			}
		}
		if (lhs.size() != rhs.size()) {
			return lhs.size() <=> rhs.size();
		}
		return std::strong_ordering::equal;
	}

	auto operator<<(std::ostream &os, const filtered_string_view &fsv) -> std::ostream &{
		if (fsv.empty()) {
			return os;
		}
		for (int i = 0; i < static_cast<int>(fsv.size()); ++i) {
			os << fsv.at(i);
		}
		return os;
	}

	auto compose(const filtered_string_view &fsv, const std::vector<std::function<bool(char)>> &filts) -> filtered_string_view{
		auto pred_compose_ = [filts](char c) -> bool{
			for (const auto &filt_ : filts) {
				if (!filt_(c)) {
					return false;
				}
			}
			return true;
		};
		filtered_string_view res_{fsv.data(), pred_compose_};
		return res_;
	}

	auto split(const filtered_string_view &fsv, const filtered_string_view &tok) -> std::vector<filtered_string_view>{
		std::vector<filtered_string_view> res_;
		if (fsv.empty() || tok.empty()) {
			res_.push_back(fsv);
			return res_;
		}
		int start_ = 0;
		int end_ = 0;
		while (end_ < static_cast<int>(fsv.size())) {
			if (fsv.at(end_) == tok.at(0)) {
				bool flag_ = true;
				for (int i = 1; i < static_cast<int>(tok.size()); ++i) {
					if (fsv.at(end_ + i) != tok.at(i)) {
						flag_ = false;
						break;
					}
				}
				if (flag_) {
					std::string piece_;
					if (end_ - start_ == 0) piece_ = "";
					else piece_ = std::string{fsv.data() + start_, static_cast<std::size_t>(end_ - start_)};
					filtered_string_view temp_{piece_, fsv.predicate()};
					res_.push_back(temp_);
					start_ = end_ + static_cast<int>(tok.size());
					end_ = start_;
				} else {
					++end_;
				}
			} else {
				++end_;
			}
		}
		std::string piece_;
		if (end_ - start_ == 0) piece_ = "";
		else piece_ = std::string{fsv.data() + start_, static_cast<std::size_t>(end_ - start_)};
		filtered_string_view temp_{piece_, fsv.predicate()};
		res_.push_back(temp_);
		return res_;
	}

	auto substr(const filtered_string_view &fsv, int pos, int count) -> filtered_string_view{
		if (pos < 0 || pos >= static_cast<int>(fsv.size()) || count < 0) {
			throw std::domain_error{"filtered_string_view::substr(<start>, <len>): invalid start or len"};
		}
		std::string res_;
		for (int i = pos; i < std::min(static_cast<int>(fsv.size()) - pos, pos + count); ++i) {
			if (fsv.predicate()(fsv.at(i))) {
				res_ += fsv.at(i);
			}
		}
		filtered_string_view res_fsv_{res_, fsv.predicate()};
		return res_fsv_;
	}

	auto filtered_string_view::substr(int pos, int count) const -> filtered_string_view {
		return fsv::substr(*this, pos, count);
	}

	fsv::filtered_string_view::iter::iter() = default;
	fsv::filtered_string_view::iter::iter(const char *ptr, const filter &pred, const std::size_t len, bool ending) noexcept{
		iter_ptr_ = std::remove_cv_t<char *>(ptr);
		begin_ = ptr;
		end_ = ptr;
		pred_ = pred;
		auto fsv_ptr = ptr;
		for (auto i = ptr; i < ptr + len * sizeof(char); ++i) {
			if (pred_(*i)) {
				end_ = fsv_ptr;
			}
		}
		if (ending) {
			iter_ptr_ = std::remove_cv_t<char *>(end_)+ 1 * sizeof(char);
		}
	}

	auto fsv::filtered_string_view::iter::operator*() const -> reference {
		return *iter_ptr_;
	}

	auto fsv::filtered_string_view::iter::operator->() const -> pointer {
		return iter_ptr_;
	}

	auto fsv::filtered_string_view::iter::operator++() -> iter & {
		for (auto i = iter_ptr_ + 1 * sizeof(char); i <= end_; ++i) {
			if (pred_(*i)) {
				iter_ptr_ = i;
				break;
			}
		}
		return *this;
	}

	auto fsv::filtered_string_view::iter::operator++(int) -> iter {
		for (auto i = iter_ptr_ + 1 * sizeof(char); i <= end_; ++i) {
			if (pred_(*i)) {
				iter_ptr_ = i;
				break;
			}
		}
		return *this;
	}

	auto fsv::filtered_string_view::iter::operator--() -> iter & {
		for (auto i = iter_ptr_ - 1 * sizeof(char); i >= begin_; --i) {
			if (pred_(*i)) {
				iter_ptr_ = i;
				break;
			}
		}
		return *this;
	}

	auto fsv::filtered_string_view::iter::operator--(int) -> iter {
		for (auto i = iter_ptr_ - 1 * sizeof(char); i >= begin_; --i) {
			if (pred_(*i)) {
				iter_ptr_ = i;
				break;
			}
		}
		return *this;
	}

	auto operator==(const fsv::filtered_string_view::iterator &lhs, const fsv::filtered_string_view::iterator &rhs) -> bool {
		return lhs.iter_ptr_ == rhs.iter_ptr_;
	}

	auto operator!=(const fsv::filtered_string_view::iterator &lhs, const fsv::filtered_string_view::iterator &rhs) -> bool {
		return !(lhs == rhs);
	}

	auto fsv::filtered_string_view::begin() const noexcept-> iterator {
		return iterator{data(), predicate(), len_};
	}

	auto fsv::filtered_string_view::end() const noexcept-> iterator {
		return iterator{data(), predicate(), len_, true};
	}

	auto fsv::filtered_string_view::cbegin() const noexcept-> const_iterator {
		return iterator{data(), predicate(), len_};
	}

	auto fsv::filtered_string_view::cend() const noexcept-> const_iterator {
		return iterator{data(), predicate(), len_, true};
	}

	auto fsv::filtered_string_view::rbegin() const noexcept-> reverse_iterator {
		return reverse_iterator{end()};
	}

	auto fsv::filtered_string_view::rend() const noexcept-> reverse_iterator {
		return reverse_iterator{begin()};
	}

	auto fsv::filtered_string_view::crbegin() const noexcept-> const_reverse_iterator {
		return reverse_iterator{cend()};
	}

	auto fsv::filtered_string_view::crend() const noexcept-> const_reverse_iterator {
		return reverse_iterator{cbegin()};
	}

}