#include <thread>
#include <archivarius/catalogue.h>
#include <archivarius/precomp.h>
#include <archivarius/platform.h>
#include "test.h"

/*
Directory structure:
~/atest-src/ - files for initial seed
~/atest-tmp/ - working dir, to where initial seed files will be copied
~/atest-add/ - from this dir files in working dir will be updated gradually, with each test step
~/atest-rmv  - list of files, which will be gradually removed with each test step
~/atest-arc/ - archive will be stored here
*/

int run(int argc, const char *argv[]);

using namespace std;
using namespace archi;
namespace fs = std::filesystem;

template <>
struct fmt::formatter<fs::file_time_type> {
	template <typename ParseContext>
	constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const fs::file_time_type &p, FormatContext &ctx) {
		auto cftime = fs::file_time_type::clock::to_time_t(p);
		return format_to(ctx.begin(), "{}", std::asctime(std::localtime(&cftime)));
	}
};

static const string password = "qwerty";
//static const string password = "";

struct File {
	enum Type{
		FILE,
		DIR,
		SYMLINK
	};
	using Filetime = fs::file_time_type;

	Type type;
	Filetime time;
	u64  size;
	fs::path symlink_target;
	fs::perms unix_permissions;
	string acl;
	string default_acl;
};

using Fs_state = map<fs::path, File>;

void recursive_walk(fs::path p, const function<void(fs::path)> &visitor){
	for(auto& di: fs::directory_iterator(p)){
		if (di.path().filename().string() == "ignore")
			continue;
		visitor(di.path());
		if (di.is_directory())
			recursive_walk(di.path(), visitor);
	}
}

Fs_state state_for(fs::path p){
	Fs_state ret;
	recursive_walk(p, [&ret](fs::path p){
		auto &f = ret[p];
		if (is_regular_file(p))
			f.type = File::FILE;
		if (is_directory(p))
			f.type = File::DIR;
		if (is_symlink(p))
			f.type = File::SYMLINK;

		f.time = fs::last_write_time(p);
		if (f.type == File::FILE)
			f.size = fs::file_size(p);
		if (f.type == File::SYMLINK)
			f.symlink_target = fs::read_symlink(p);
		f.unix_permissions = fs::symlink_status(p).permissions();
		f.acl = get_acl(p);
		if (f.type == File::DIR)
			f.default_acl = get_default_acl(p);
	});
	return ret;
}

void compare(Fs_state &a, Fs_state &b){
	for (auto &pa : a){
		if (b.find(pa.first) == b.end()){
			ASSERT(0);
			throw runtime_error(fmt::format("{0} does not exist in another state", pa.first));
		}
		auto &da = pa.second;
		auto &db = b[pa.first];
		try{
			if (da.type != db.type){
				ASSERT(0);
				throw runtime_error(fmt::format("type dont match\n{0}\n{1}", da.type, db.type));
			}
			if (da.time != db.time){
				ASSERT(0);
				throw runtime_error(fmt::format("times dont match\n{0}\n{1}", da.time, db.time));
			}
			if (da.type == File::FILE && da.size != db.size){
				ASSERT(0);
				throw runtime_error(fmt::format("fsize dont match\n{0}\n{1}", da.size, db.size));
			}
			if (da.unix_permissions != db.unix_permissions){
				ASSERT(0);
				throw runtime_error(fmt::format("this time permissions are the problem\n{0}\n{1}",(u16) da.unix_permissions, (u16)db.unix_permissions));
			}
			if (da.acl != db.acl){
				ASSERT(0);
				throw runtime_error(fmt::format("acl dont match\n{0}\n{1}", da.acl, db.acl));
			}
			if (da.default_acl != db.default_acl){
				ASSERT(0);
				throw runtime_error(fmt::format("default acl dont match\n{0}\n{1}", da.default_acl, db.default_acl));
			}
		}
		catch(...){
			throw_with_nested(fmt::format("file {0}:", pa.first));
		}
		b.erase(pa.first);
	}
	ASSERT(b.size() == 0);
}

void run(vector<const char*> params){
	params.insert(params.begin(), "archivarius");
	if (run(params.size(), params.data())){
		ASSERT(0);
		throw runtime_error("run() returned non zero code");
	}
}

void extract(size_t i, fs::path arc, fs::path to){
	fs::remove_all(to);
	auto a = "archive={}"_format(arc);
	auto t = "target-dir={}"_format(to);
	auto id = "id={}"_format(i);
	auto pwrd = "password={}"_format(password);
	vector<const char *> params{"restore", a.c_str(), t.c_str(), id.c_str()};
	if (!password.empty())
		params.push_back(pwrd.c_str());
	run(params);
}

#pragma GCC diagnostic ignored "-Wunused-result"

void test()
{
	fs::path hf = getenv("HOME");
	ASSERT(!hf.empty());
	fs::path atest_src = hf / "atest-src";
	fs::path atest_tmp = hf / "atest-tmp";
	fs::path atest_add = hf / "atest-add";
	fs::path atest_rmv = hf / "atest-rmv";
	fs::path atest_arc = hf / "atest-arc";
	fs::remove_all(atest_tmp);
	fs::remove_all(atest_arc);
	fs::create_directory(atest_tmp);
	fmt::print("cp --reflink -a \"{}\"/* \"{}\"\n", atest_src, atest_tmp);
	system(fmt::format("cp --reflink -a \"{}\"/* \"{}\"", atest_src, atest_tmp).c_str());
	vector<string> rmv_list;
	{
		ifstream i = ifstream(atest_rmv);
		ASSERT(i.good());
		string line;
		while (i.good()){
			getline(i, line);
			if (!line.empty())
				rmv_list.push_back(line);
		}
	}
	auto to_add = fs::directory_iterator(atest_add);
	auto to_remove = rmv_list.begin();
	vector<Fs_state> states;
	for (bool quit = false; !quit; ){
		fmt::print("Iteration {}\n", states.size());
		states.push_back(state_for(atest_tmp));
		run({"archive", "cfg-file=test.conf"});
		quit = true;
		if (to_add != end(fs::directory_iterator())){
			fmt::print("cp --reflink -a \"{}\" \"{}\"/\n", to_add->path(), atest_tmp);
			system(fmt::format("cp --reflink -a \"{}\" \"{}\"/", to_add->path(), atest_tmp).c_str());
			to_add++;
			quit = false;
		}
		if (to_remove != end(rmv_list)){
			fmt::print("removing {}\n", atest_tmp / *to_remove);
			fs::remove_all(atest_tmp / *to_remove);
			to_remove++;
			quit = false;
		}
	}
	auto last_state = states.back();
	for (size_t i = 0; i < states.size(); i++){
		fmt::print("{}%\r", i * 100 /states.size());
		fflush(stdout);
		auto j = states.size() - 1 - i;
		extract(j, atest_arc, atest_tmp);
		auto fs = state_for(atest_tmp);
		compare(fs, states[i]);
	}
	this_thread::sleep_for(2s);
	run({"archive", "cfg-file=test-1s.conf"});
	{
		Catalogue cat(atest_arc, password);
		ASSERT(cat.num_states() == 1);
		if (cat.num_states() != 1)
			throw runtime_error("GC test failed");
		// lock test
		bool failed = false;
		try{
			Catalogue cat1(atest_arc, password);
		}
		catch(...){
			failed = true;
		}
		ASSERT(failed);
		if (!failed)
			throw runtime_error("lock test failed");
	}
	extract(0, atest_arc, atest_tmp);
	auto fs = state_for(atest_tmp);
	compare(fs, last_state);
	print(fg(fmt::terminal_color::bright_green), "All green! All shiny!\n");
	fs::remove_all(atest_tmp);
	fs::remove_all(atest_arc);
}
