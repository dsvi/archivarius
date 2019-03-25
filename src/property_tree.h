#pragma once
#include "precomp.h"

namespace property_tree{

class Property {
public:
	Property();
	std::string_view name();
	/// throws if no value
	std::string_view value_str();
	/// doesn't throw
	std::string_view opt_value_str();
	u64 value_u64();
	/// raw text, not divided to name and value. gets trimmed inside. might be empty()
	std::string_view text();
	/// will be divide to name and value. can be empty()
	void text(std::string &&txt);

	auto
	subs(){
		return  kids_ | ranges::view::all;
	}
	bool has_subs(){
		return !kids_.empty();
	}
	void add_sub(Property &&p);

	/// name of file property came from
	std::string orig_name();
	int orig_line();

private:
	std::vector<Property> kids_;
	std::string       post_kids_whitespaces_; // whitespaces before '}'
	std::string       pre_whitestaces_; // preceding whitespaces. may include commented out lines
	std::string       text_;
	std::string       key_;
	std::string       val_;
	std::shared_ptr<std::string> origin_fn_;
	int               origin_ln_;

	friend
	Property from_file(std::filesystem::path &filepath);
};

inline
std::string_view Property::name()
{
	return key_;
}

inline
std::string_view Property::opt_value_str()
{
	return val_;
}

inline
std::string_view Property::text()
{
	return text_;
}

inline
std::string property_tree::Property::orig_name()
{
	return *origin_fn_;
}

inline
int Property::orig_line()
{
	return origin_ln_;
}


Property from_file(std::filesystem::path &filepath);

}
