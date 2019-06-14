#include "property_tree.h"
#include "globals.h"
#include "exception.h"

using namespace std;

property_tree::Property property_tree::from_file(std::filesystem::path &filepath)
{
	try{
		auto is = ifstream();
		is.exceptions(ios_base::badbit | ios_base::failbit);
		is.open(filepath);
		auto orig_fn = make_shared<string>(filepath);
		int ln = 0;
		is.exceptions(ios_base::badbit);
		try{
			vector<Property> stack;
			stack.emplace_back();
			string pre_ws("");
			string line;
			while (!is.eof()){
				getline(is, line);
				if (is.eof() and line.empty())
					break;
				ln++;
				auto li = find_if(line.begin(), line.end(), [](int ch) {
					return !std::isspace(ch);
				});
				pre_ws.append(line.begin(), li);
				if (*li == '#'){
					pre_ws.append(li, line.end());
					pre_ws += "\n";
					continue;
				}
				trim(line);
				if (line.empty()){
					pre_ws += "\n";
					continue;
				}
				if (line == "}"){
					if (stack.size() == 1)
						throw Exception("} does not match any opened bracket {");
					Property p = move(stack.back());
					stack.pop_back();
					p.post_kids_whitespaces_ = pre_ws;
					pre_ws = "\n";
					stack.back().add_sub(move(p));
					continue;
				}
				bool has_kids = false;
				if (line.back() == '{'){
					line.pop_back();
					has_kids = true;
				}
				Property p;
				p.text(string(line));
				p.pre_whitestaces_ = pre_ws;
				pre_ws = "\n";
				p.origin_fn_ = orig_fn;
				p.origin_ln_ = ln;
				if (has_kids)
					stack.emplace_back(move(p));
				else
					stack.back().add_sub(move(p));
			}
			if (stack.size() != 1)
				throw Exception("Expecting }");
			return move(stack.back());
		}
		catch (...){
			throw_with_nested( Exception("Line: {0}")(ln) );
		}
	}
	catch (...){
		throw_with_nested( Exception("File: {0}")(filepath) );
	}
}

property_tree::Property::Property()
{

}

std::string_view property_tree::Property::value_str()
{
	if (val_.empty())
		throw Exception("Property '{0}' should have a value.\nProperty came from {1} line {2}")(key_, *origin_fn_, origin_ln_);
	return val_;
}

u64 property_tree::Property::value_u64()
{
	try{
		return stoull(val_);
	}
	catch(...){
		throw_with_nested(Exception("Value for '{0}' must be unsigned integer.\nProperty came from {1} line {2}")(key_, *origin_fn_, origin_ln_));
	}
}

void property_tree::Property::add_sub(property_tree::Property &&p)
{
	kids_.emplace_back(move(p));
}

void property_tree::Property::text(string &&txt)
{
	text_ = move(txt);
	key_ = string();
	val_ = string();
	trim(text_);
	if (text_.empty())
		return;
	auto ke = find_if(text_.begin(), text_.end(), [](auto &c){ return isspace(c); });
	key_ = string(text_.begin(), ke);
	if (ke == text_.end())
		return;
	auto vb = find_if(ke, text_.end(), [](auto &c){ return !isspace(c); });
	val_ = string(vb, text_.end());
}
