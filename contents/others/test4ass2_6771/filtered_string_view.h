#ifndef COMP6771_ASS2_FSV_H
#define COMP6771_ASS2_FSV_H

#include <compare>
#include <exception>
#include <functional>
#include <iterator>
#include <string>
#include <vector>
#include <iostream>

namespace fsv {
	using filter = std::function<bool(const char &)>;

	//default predicate
	class filtered_string_view {
		class iter {
		 public:
			using MEMBER_TYPEDEFS_GO_HERE = void;

			using difference_type = std::ptrdiff_t;
			using value_type = char;
			using pointer = const char *;
			using reference = const char &;
			using iterator_category = std::bidirectional_iterator_tag;

			iter();
			iter(const char *p, const filter& pred, const std::size_t len, bool ending = false) noexcept;

			auto operator*() const -> reference; // change this
			auto operator->() const -> pointer; // change this

			auto operator++() -> iter&;
			auto operator++(int) -> iter;
			auto operator--() -> iter&;
			auto operator--(int) -> iter;

			friend auto operator==(const iter &, const iter &) -> bool;
			friend auto operator!=(const iter &, const iter &) -> bool;

		 private:
			/* Implementation-specific private members */
			char *iter_ptr_{nullptr};
			const char *begin_{nullptr};
			const char *end_{nullptr};
			filter& pred_{default_predicate};
		};
	 public:
		static filter default_predicate;
		using iterator = iter;
		using const_iterator = iter;
		using reverse_iterator = std::reverse_iterator<iter>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		//Range member function
		[[nodiscard]] iterator begin() const noexcept;
		[[nodiscard]] iterator end() const noexcept;
		[[nodiscard]] const_iterator cbegin() const noexcept;
		[[nodiscard]] const_iterator cend() const noexcept;

		[[nodiscard]] reverse_iterator rbegin() const noexcept;
		[[nodiscard]] reverse_iterator rend() const noexcept;
		[[nodiscard]] const_reverse_iterator crbegin() const noexcept;
		[[nodiscard]] const_reverse_iterator crend() const noexcept;


		// constructor && destructor
		explicit filtered_string_view() noexcept; // default constructors
		filtered_string_view(const std::string &str) noexcept; // implicit string constructor
		explicit filtered_string_view(const std::string &str, filter predicate) noexcept; // predicate constructor
		filtered_string_view(const char *str) noexcept; // implicit Null-terminated string constructor
		explicit filtered_string_view(const char *str, filter predicate) noexcept; // Null-terminated constructor
		filtered_string_view(const filtered_string_view &other) noexcept; //Copy constructor
		filtered_string_view(filtered_string_view &&other) noexcept; // Move constructor

		~filtered_string_view() noexcept; //default_destructor

		// member operators
		filtered_string_view& operator=(const filtered_string_view &other) noexcept;
		filtered_string_view& operator=(filtered_string_view &&other) noexcept;
		auto operator[](int n)  const -> const char &;
		[[nodiscard]] explicit operator std::string () const noexcept;

		// member functions
		[[nodiscard]] auto at(int n) const -> const char &;
		[[nodiscard]] auto size() const noexcept-> std::size_t ;
		[[nodiscard]] auto empty() const noexcept-> bool;
		[[nodiscard]] auto data() const noexcept-> const char *;
		[[nodiscard]] auto predicate() const noexcept -> const filter&;
		[[nodiscard]] auto substr(int pos = 0, int count = 0) const -> filtered_string_view;
		// friend functions
        friend auto operator==(const filtered_string_view &lhs, const filtered_string_view &rhs) -> bool;
        friend auto operator<=>(const filtered_string_view &lhs, const filtered_string_view &rhs) -> std::strong_ordering;
		friend auto operator<<(std::ostream &os, const filtered_string_view &fsv) -> std::ostream&;
		friend auto compose(const filtered_string_view &fsv, const std::vector<std::function<bool(char)>> &filts) -> filtered_string_view;
		friend auto split(const filtered_string_view &fsv, const filtered_string_view &tok) -> std::vector<filtered_string_view>;

	 private:
		const char* ptr_{nullptr};
		std::size_t len_{0};
		filter predicate_func_{default_predicate};
	};

	auto substr(const filtered_string_view &fsv, int pos = 0, int count = 0) -> filtered_string_view;

}

#endif // COMP6771_ASS2_FSV_H
