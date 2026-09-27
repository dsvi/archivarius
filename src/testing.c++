#include <thread>
#include "globals.h"
#include "exception.h"
#include "catalogue.h"
#include "precomp.h"
#include "time.h"
#include "platform.h"
#include "coformat.h"
#include "testing.h"

/*
The self test. See test/readme.md for the description of the seed data.

Everything is done under /tmp/archivarius-test, which is a tmpfs on most systems, so that
the test runs mostly in ram. The seed data is copied there before the long test starts.

Three parts:

	- filters_test(). Small hand made trees. Checks the 'root' 'include' and 'exclude'
	  config logic: what gets into the archive, and what comes out of it. Fast.
	- multi_version_test(). Several versions of the same tree, archived with different
	  filters. Fast.
	- full_test(). The long one. A big seed tree, many archive versions, every version is
	  restored and compared with the expected state. Then the GC and the lock tests.
*/

int run(int argc, const char *argv[]);

using namespace coformat;
using namespace std;
using namespace archi;
namespace fs = std::filesystem;

// the password of the archives the filters test makes
static const string password = "qwerty";
// the long test archives are not encrypted, so that it runs fast. See test/test.conf
static const string long_test_password = "";

// The test does all its work here. Wiped out at the start of the long test.
static const fs::path test_dir = "/tmp/archivarius-test";

// Where the seed for the long test lives. See test/readme.md
static fs::path seed_dir()
{
	if (auto env = getenv("ARCHIVARIUS_TEST_SEED"); env and *env)
		return env;
	auto hf = getenv("HOME");
	if (!hf)
		throw runtime_error("Can't determine HOME. Set ARCHIVARIUS_TEST_SEED to where the seed data is.");
	return fs::path(hf) / "temp/atest";
}

// The test configs live in the 'test' dir of the project.
// Search next to the current dir, then up from the location of the executable.
static fs::path test_conf(string_view name)
{
	vector<fs::path> where;
	if (auto env = getenv("ARCHIVARIUS_TEST_DIR"); env and *env)
		where.push_back(env);
	where.push_back("test");
	try{
		auto dir = fs::read_symlink("/proc/self/exe").parent_path();
		for (int i = 0; i < 8 and dir != dir.parent_path(); i++){
			where.push_back(dir / "test");
			dir = dir.parent_path();
		}
	}
	catch(...){}
	for (auto &w : where)
		if (fs::exists(w / name))
			return w / name;
	throw runtime_error("Can't find the test config '"s + string(name)
	                    + "'. Run me from the project dir, or set ARCHIVARIUS_TEST_DIR.");
}

using Filetime = fs::file_time_type;
struct File {
	enum Type{
		FILE,
		DIR,
		SYMLINK
	};

	Type type;
	Filetime time;
	u64  size;
	fs::path symlink_target;
	fs::perms unix_permissions;
	string acl;
	string default_acl;
};

template <>
struct std::formatter<File::Type> : std::formatter<std::string>{
	template<class FormatContext>
	auto format(File::Type &p, FormatContext &ctx) const {
		using Type = File::Type;
		if (p == Type::FILE)
			return std::formatter<std::string>::format("File", ctx);
		if (p == Type::DIR)
			return std::formatter<std::string>::format("Dir", ctx);
		if (p == Type::SYMLINK)
			return std::formatter<std::string>::format("Symlink", ctx);
		throw format_error(std::format("What is this File::Type? {}", (size_t) p));
	}
};

using chrono_base_formatter = std::formatter<std::chrono::sys_time<Time::Accuracy>>;
template <>
struct std::formatter<Filetime> : chrono_base_formatter{
	template<class FormatContext>
	auto format(const Filetime &p, FormatContext &ctx) const {
		auto sys_time = Filetime::clock::to_sys(p);
		return chrono_base_formatter::format(sys_time, ctx);
	}
};

using Fs_state = map<fs::path, File>;

void recursive_walk(fs::path p, const function<void(fs::path)> &visitor){
	for(auto& di: fs::directory_iterator(p)){
		if (di.path().filename().string() == "ignore")
			continue;
		visitor(di.path());
		// a symlink is stored as a link, it may not even lead anywhere. Don't follow it
		if (!di.is_symlink() and di.is_directory())
			recursive_walk(di.path(), visitor);
	}
}

Fs_state state_for(fs::path p){
	Fs_state ret;
	recursive_walk(p, [&ret](fs::path p){
		auto &f = ret[p];
		auto sts = fs::symlink_status(p);
		if (fs::is_regular_file(sts))
			f.type = File::FILE;
		if (fs::is_directory(sts))
			f.type = File::DIR;
		if (fs::is_symlink(sts))
			f.type = File::SYMLINK;

		f.unix_permissions = sts.permissions();
		if (f.type == File::SYMLINK)
			f.symlink_target = fs::read_symlink(p);

		if (f.type == File::SYMLINK and !fs::exists(p))
			return; // a dangling link. There is nothing to get the attributes from, and
		            // archivarius stores a symlink without its attributes anyway

		f.time = fs::last_write_time(p);
		if (f.type == File::FILE)
			f.size = fs::file_size(p);
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
			throw runtime_error(format("{0} does not exist in another state", pa.first));
		}
		auto &da = pa.second;
		auto &db = b[pa.first];
		try{
			if (da.type != db.type){
				ASSERT(0);
				throw runtime_error(cformat("type dont match\n{}\n{}", da.type, db.type));
			}
			if (da.time != db.time){
				ASSERT(0);
				throw runtime_error(cformat("times dont match\n{}\n{}", da.time, db.time));
			}
			if (da.type == File::FILE && da.size != db.size){
				ASSERT(0);
				throw runtime_error(format("fsize dont match\n{}\n{}", da.size, db.size));
			}
			if (da.unix_permissions != db.unix_permissions){
				ASSERT(0);
				throw runtime_error(format("this time permissions are the problem\n{0}\n{1}",(u16) da.unix_permissions, (u16)db.unix_permissions));
			}
			if (da.acl != db.acl){
				ASSERT(0);
				throw runtime_error(format("acl dont match\n{0}\n{1}", da.acl, db.acl));
			}
			if (da.default_acl != db.default_acl){
				ASSERT(0);
				throw runtime_error(format("default acl dont match\n{0}\n{1}", da.default_acl, db.default_acl));
			}
		}
		catch(...){
			throw_with_nested(format("file {0}:", pa.first));
		}
		b.erase(pa.first);
	}
	ASSERT(b.size() == 0);
}

int run_raw(vector<const char*> params)
{
	params.insert(params.begin(), "archivarius");
	return run(params.size(), params.data());
}

void run(vector<const char*> params)
{
	if (run_raw(params))
		throw runtime_error("run() returned non zero code");
}

void extract(size_t i, fs::path arc, fs::path to, string_view pwd){
	fs::remove_all(to);
	auto a = format("archive={}", arc.string());
	auto t = format("target-dir={}", to.string());
	auto id = format("id={}", i);
	auto pwrd = format("password={}", pwd);
	vector<const char *> params{"restore", a.c_str(), t.c_str(), id.c_str()};
	if (!pwd.empty())
		params.push_back(pwrd.c_str());
	run(params);
}

// quotes a path so that the shell takes it as is, whatever is inside it
static string shell_quote(string_view s)
{
	string ret("'");
	for (char c : s)
		if (c == '\'')
			ret += "'\\''";
		else
			ret += c;
	ret += "'";
	return ret;
}

static
void run_command(std::string &&cmd){
	println("{}", cmd);
	if (system(cmd.c_str())){
		ASSERT(0);
		throw runtime_error("the command returned non zero code: " + cmd);
	}
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- filters test -=-=-=-=-=-=-=-=-=-=-=-=-=- */

struct Expected{
	File::Type type;
	string data; // contents of a regular file, target of a symlink, empty for a dir
};

// key is a relative path
using Tree = map<fs::path, Expected>;

static
void make_file(const fs::path &p, string_view content)
{
	fs::create_directories(p.parent_path());
	ofstream out(p);
	out << content;
	out.flush();
	if (!out.good())
		throw runtime_error("Can't create the file " + p.string());
}

static
void make_symlink(const fs::path &p, string_view target)
{
	fs::create_directories(p.parent_path());
	error_code ec;
	fs::create_symlink(target, p, ec);
	if (ec)
		throw runtime_error("Can't create the symlink " + p.string() + ": " + ec.message());
}

// doesn't skip anything, and doesn't follow symlinks
static
void walk_tree(const fs::path &dir, const fs::path &relative, Tree &to)
{
	for (auto &e : fs::directory_iterator(dir)){
		auto r = relative / e.path().filename();
		auto st = fs::symlink_status(e.path());
		if (fs::is_symlink(st)){
			to[r] = {File::SYMLINK, fs::read_symlink(e.path()).string()};
			continue;
		}
		if (fs::is_directory(st)){
			to[r] = {File::DIR, ""};
			walk_tree(e.path(), r, to);
			continue;
		}
		if (fs::is_regular_file(st)){
			ifstream in(e.path());
			string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
			if (!in)
				throw runtime_error("Can't read " + e.path().string());
			to[r] = {File::FILE, content};
			continue;
		}
		throw runtime_error("Don't know what to do with " + e.path().string());
	}
}

static
Tree walk_of(const fs::path &dir)
{
	Tree ret;
	walk_tree(dir, fs::path(), ret);
	return ret;
}

// The tree the filters test archives. Every file has unique content, so a mixup is detectable.
static
void make_sample_tree(const fs::path &root)
{
	make_file(root / "a.txt", "a");
	make_file(root / "empty.txt", "");
	make_file(root / "big.bin", string(4096, 'B'));
	make_file(root / "excl.txt", "excl");
	make_file(root / "skip/ignored.txt", "ignored1");
	make_file(root / "dir1/f1.txt", "f1");
	make_file(root / "dir1/skip/ignored.txt", "ignored2");
	make_file(root / "dir1/nested/f2.txt", "f2");
	make_file(root / "dir2/deep/f3.txt", "f3");
	make_symlink(root / "link", "a.txt");           // a relative link to a file
	make_symlink(root / "dirlink", "dir1");         // a relative link to a directory
	make_symlink(root / "excl_link", "big.bin");    // a link to be excluded
	make_symlink(root / "badlink", "no-such-file"); // a dangling link
}

// The paths of an archive version, as they are stored in it
static
Tree state_tree(Catalogue &cat, size_t id)
{
	Tree ret;
	auto state = cat.fs_state(id);
	for (auto &f : state.files()){
		Expected e;
		switch (f.type){
		case Filesystem_state::FILE:    e.type = File::FILE;    break;
		case Filesystem_state::DIR:     e.type = File::DIR;     break;
		case Filesystem_state::SYMLINK: e.type = File::SYMLINK; break;
		default: ASSERT(0);
		}
		e.data = f.type == Filesystem_state::SYMLINK ? f.symlink_target.string() : "";
		ret[f.path] = e;
	}
	return ret;
}

// What a scenario expects, out of a list of paths relative to 'root'
static
Tree expectation(const Tree &sample, const vector<string> &paths)
{
	Tree ret;
	for (auto &p : paths){
		auto it = sample.find(fs::path(p));
		if (it == sample.end())
			throw runtime_error("There is no " + p + " in the sample tree");
		ret[it->first] = it->second;
	}
	return ret;
}

// The same, plus the dirs which are not archived themselves, but have to be created to
// hold the restored files
static
Tree restore_expectation(const Tree &sample, const vector<string> &paths)
{
	Tree ret = expectation(sample, paths);
	Tree originals = ret;
	for (auto &e : originals)
		for (auto p = e.first.parent_path(); !p.empty(); p = p.parent_path())
			ret.try_emplace(p, Expected{File::DIR, ""});
	return ret;
}

static
void compare_trees(const Tree &got, const Tree &want, string_view what, bool check_data)
{
	vector<string> problems;
	for (auto &w : want){
		auto it = got.find(w.first);
		if (it == got.end()){
			problems.push_back("  missing: " + w.first.string());
			continue;
		}
		if (it->second.type != w.second.type)
			problems.push_back("  wrong type: " + w.first.string());
		else if (check_data and it->second.data != w.second.data)
			problems.push_back("  wrong content: " + w.first.string());
	}
	for (auto &g : got)
		if (!want.contains(g.first))
			problems.push_back("  unexpected: " + g.first.string());
	if (!problems.empty()){
		string all;
		for (auto &p : problems)
			all += "\n" + p;
		throw runtime_error(cformat("{0} is wrong:{1}", what, all));
	}
}

struct Scenario{
	string name;
	string root_suffix;    // appended to the 'root' value in the config. to try a trailing '/'
	vector<string> include;// 'include' entries, as they are written in the config
	vector<string> exclude;// 'exclude' entries, as they are written in the config
	vector<string> expect; // paths, relative to the root. expected to be archived and restored
	bool expect_fail = false;    // either the filters leave nothing to archive, or the config
	                             // has to be refused
};

// writes the config, and prints it
static
fs::path write_conf(const fs::path &scratch, string_view body)
{
	auto conf = scratch / "task.conf";
	string conf_txt = "task test backup {\n" + string(body) + "}\n";
	ofstream out(conf);
	out << conf_txt;
	out.flush();
	if (!out.good())
		throw runtime_error("Can't write " + conf.string());
	print("{}", conf_txt);
	return conf;
}

static
void archive_with(const fs::path &scratch, string_view body)
{
	auto cfg_arg = "cfg-file=" + write_conf(scratch, body).string();
	run({"archive", cfg_arg.c_str()});
}

static
void check_scenario(const Scenario &sc)
{
	auto scratch = test_dir / "filters" / sc.name;
	fs::remove_all(scratch);
	fs::create_directories(scratch);
	auto in  = scratch / "in";
	auto arc = scratch / "arc";
	make_sample_tree(in);

	Tree sample = walk_of(in);
	Tree expected          = expectation(sample, sc.expect);
	Tree expected_restored = restore_expectation(sample, sc.expect);

	string body = "\tarchive " + arc.string() + "\n";
	body += "\troot " + in.string() + sc.root_suffix + "\n";
	if (!sc.include.empty()){
		body += "\tinclude {\n";
		for (auto &i : sc.include)
			body += "\t\t" + i + "\n";
		body += "\t}\n";
	}
	if (!sc.exclude.empty()){
		body += "\texclude {\n";
		for (auto &e : sc.exclude)
			body += "\t\t" + e + "\n";
		body += "\t}\n";
	}
	body += "\tpassword " + password + "\n";
	body += "\tcompression on\n";

	cprintln("┄┄┄ filters: {fb}{0}{fd} ┄┄┄", sc.name);

	auto conf = write_conf(scratch, body);

	auto cfg_arg = "cfg-file=" + conf.string();
	int rc = 0;
	bool threw = false;
	try{
		rc = run_raw({"archive", cfg_arg.c_str()});
	}
	catch(...){
		threw = true; // an invalid config throws all the way out of run()
	}
	if (sc.expect_fail != (rc != 0 or threw)){
		throw runtime_error("The scenario '" + sc.name + "' expected "
							+ (sc.expect_fail ? "a failure" : "a success")
							+ ", but run() returned " + to_string(rc));
	}

	{
		if (sc.expect_fail){
			if (fs::exists(arc)){
				Catalogue cat(arc, password, false);
				if (cat.num_states())
					throw runtime_error("The scenario '" + sc.name
										+ "' expected nothing to be archived, but there is a version");
			}
			println("  ok: nothing archived\n");
			return;
		}
		Catalogue cat(arc, password, false);
		if (cat.num_states() != 1){
			throw runtime_error("Expecting exactly one version in the archive, got "
								+ to_string(cat.num_states()));
		}
		compare_trees(state_tree(cat, 0), expected, "Archived paths", false);
	}

	extract(0, arc, scratch / "out", password);
	compare_trees(walk_of(scratch / "out"), expected_restored, "Restored files", true);

	println("  ok\n");
}

// a failing scenario must not stop the rest of them
static
void check(vector<string> &failed, const Scenario &sc)
{
	try{
		check_scenario(sc);
	}
	catch(std::exception &e){
		cprintln("  {fr}failed{fd}: {}", message(e));
		failed.push_back(sc.name);
	}
}

static
void filters_test(vector<string> &failed)
{
	// everything from 'root', except the 'exclude'd
	{
		Scenario sc;
		sc.name = "root-only";
		sc.exclude = {"excl.txt", "skip", "dir1/skip", "excl_link", "no-such-dir/no-such-file"};
		sc.expect = {
			"a.txt", "empty.txt", "big.bin",
			"dir1", "dir1/f1.txt", "dir1/nested", "dir1/nested/f2.txt",
			"dir2", "dir2/deep", "dir2/deep/f3.txt",
			"link", "dirlink", "badlink",
		};
		check(failed, sc);
	}
	// 'include' has priority over 'exclude'. So 'dir1' is out, but 'dir1/nested' is in.
	// Note that 'dir1' itself is not archived, but has to be recreated on restore.
	{
		Scenario sc;
		sc.name = "include-over-exclude";
		sc.exclude = {"dir1", "dir2"};
		sc.include = {"dir1/nested"};
		sc.expect = {"dir1/nested", "dir1/nested/f2.txt"};
		check(failed, sc);
	}
	// 'include' wins even over a number of 'exclude'd dirs on the way to it
	{
		Scenario sc;
		sc.name = "include-over-exclude-deep";
		sc.exclude = {"skip", "dir1"};
		sc.include = {"dir1/skip/ignored.txt"};
		sc.expect = {"dir1/skip/ignored.txt"};
		check(failed, sc);
	}
	// only what is in 'include', 'root' is still the base for the stored paths
	{
		Scenario sc;
		sc.name = "include-list";
		sc.include = {"a.txt", "link", "dir2"};
		sc.exclude = {"dir2/deep"};
		sc.expect = {"a.txt", "link", "dir2"};
		check(failed, sc);
	}
	// a single file. There are no directories in the archive, so 'target-dir' has to be created
	{
		Scenario sc;
		sc.name = "single-file";
		sc.include = {"empty.txt"};
		sc.expect = {"empty.txt"};
		check(failed, sc);
	}
	// a trailing '/' in the config paths must not break the matching
	{
		Scenario sc;
		sc.name = "trailing-slashes";
		sc.root_suffix = "/";
		sc.exclude = {"excl.txt/", "skip/", "dir1/skip/", "excl_link/"};
		sc.expect = {
			"a.txt", "empty.txt", "big.bin",
			"dir1", "dir1/f1.txt", "dir1/nested", "dir1/nested/f2.txt",
			"dir2", "dir2/deep", "dir2/deep/f3.txt",
			"link", "dirlink", "badlink",
		};
		check(failed, sc);
	}
	// nothing left to archive
	{
		Scenario sc;
		sc.name = "all-excluded";
		sc.exclude = {"a.txt", "empty.txt", "big.bin", "excl.txt", "skip", "dir1", "dir2",
		              "link", "dirlink", "excl_link", "badlink"};
		sc.expect_fail = true;
		check(failed, sc);
	}
	// '.' and '..' are not allowed in a path
	{
		Scenario sc;
		sc.name = "dot-not-allowed";
		sc.include = {"."};
		sc.exclude = {"skip"};
		sc.expect_fail = true;
		check(failed, sc);
	}
	{
		Scenario sc;
		sc.name = "dotdot-not-allowed";
		sc.include = {"dir1"};
		sc.exclude = {"skip/../skip"};
		sc.expect_fail = true;
		check(failed, sc);
	}
}

/*
The same tree archived a few times with different filters. Restoring any of the versions
has to give the tree as it was at the time that version was archived. Version 0 is the newest.
*/
static
void multi_version_test()
{
	auto scratch = test_dir / "filters/multi-version";
	fs::remove_all(scratch);
	fs::create_directories(scratch);
	auto in  = scratch / "in";
	auto arc = scratch / "arc";
	make_sample_tree(in);
	cprintln("┄┄┄ filters: {fb}{0}{fd} ┄┄┄", "multi-version");

	// everything from 'root', except the 'exclude'd
	Tree sample1 = walk_of(in);
	vector<string> expect1 = {
		"a.txt", "empty.txt", "big.bin",
		"dir1", "dir1/f1.txt", "dir1/skip", "dir1/skip/ignored.txt",
		"dir1/nested", "dir1/nested/f2.txt",
		"dir2", "dir2/deep", "dir2/deep/f3.txt",
		"link", "dirlink", "badlink",
	};
	archive_with(scratch,
		"\tarchive " + arc.string() + "\n"
		"\troot " + in.string() + "\n"
		"\texclude {\n\t\tskip\n\t\texcl.txt\n\t\texcl_link\n\t}\n"
		"\tpassword " + password + "\n"
		"\tcompression on\n");

	// a new file, a changed file, a removed file and a removed dir
	make_file(in / "new.txt", "new");
	make_file(in / "a.txt", "a changed");
	fs::remove(in / "empty.txt");
	fs::remove_all(in / "dir1/skip");

	// only the 'include'd. 'dir1/nested' is 'exclude'd
	Tree sample2 = walk_of(in);
	vector<string> expect2 = {"a.txt", "new.txt", "dir1", "dir1/f1.txt"};
	archive_with(scratch,
		"\tarchive " + arc.string() + "\n"
		"\troot " + in.string() + "\n"
		"\tinclude {\n"
		"\t\ta.txt\n"
		"\t\tnew.txt\n"
		"\t\tdir1\n"
		"\t}\n"
		"\texclude {\n\t\tdir1/nested\n\t}\n"
		"\tpassword " + password + "\n"
		"\tcompression on\n");

	// the archive as a whole is consistent: the references and the content of the files
	run({"test", ("archive=" + arc.string()).c_str(), ("password=" + password).c_str()});

	{
		Catalogue cat(arc, password, false);
		if (cat.num_states() != 2){
			throw runtime_error("Expecting 2 versions in the archive, got "
								+ to_string(cat.num_states()));
		}
		compare_trees(state_tree(cat, 1), expectation(sample1, expect1), "Archived paths, version 1", false);
		compare_trees(state_tree(cat, 0), expectation(sample2, expect2), "Archived paths, version 0", false);
	}

	extract(1, arc, scratch / "out-old", password);
	compare_trees(walk_of(scratch / "out-old"), restore_expectation(sample1, expect1),
	              "Restored files, version 1", true);
	extract(0, arc, scratch / "out-new", password);
	compare_trees(walk_of(scratch / "out-new"), restore_expectation(sample2, expect2),
	              "Restored files, version 0", true);

	println("  ok\n");
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- the long test -=-=-=-=-=-=-=-=-=-=-=-=-=-

Directory structure of the seed (~/temp/atest):
	src/  - files for the initial seed
	add/  - from this dir files in the working dir will be updated gradually, with each test step
	rmv   - list of the files, which will be removed with each test step one by one

all of it is copied to /tmp/archivarius-test, and the test works on that copy:
	src/ add/ rmv/ - the copies of the seed
	tmp/           - the working dir, to where the seed is copied and then modified
	arc/           - the archive
*/
static
void full_test()
{
	auto seed = seed_dir();
	fs::remove_all(test_dir);
	fs::create_directories(test_dir);

	// copy the seed to /tmp, so that the test runs mostly in ram
	run_command(format("cp --reflink=auto -a {}/. {}", shell_quote((seed / "src").string()), shell_quote((test_dir / "tmp").string())));
	run_command(format("cp --reflink=auto -a {} {}", shell_quote((seed / "add").string()), shell_quote((test_dir / "add").string())));
	run_command(format("cp --reflink=auto -a {} {}", shell_quote((seed / "rmv").string()), shell_quote((test_dir / "rmv").string())));

	auto atest_tmp = test_dir / "tmp";
	auto atest_add = test_dir / "add";
	auto atest_rmv = test_dir / "rmv";
	auto atest_arc = test_dir / "arc";

	vector<string> rmv_list;
	{
		ifstream i = ifstream(atest_rmv);
		if (!i.good())
			throw runtime_error("Can't read " + atest_rmv.string());
		string line;
		while (i.good()){
			getline(i, line);
			if (!line.empty())
				rmv_list.push_back(line);
		}
	}
	size_t total = max((size_t)distance(fs::directory_iterator(atest_add), fs::directory_iterator()), rmv_list.size());
	auto to_add = fs::directory_iterator(atest_add);
	auto to_remove = rmv_list.begin();
	vector<Fs_state> states;
	for (bool quit = false; !quit; ){
		println("{}%", 100*states.size()/total);
		states.push_back(state_for(atest_tmp));
		auto cfg = "cfg-file=" + test_conf("test.conf").string();
		run({"archive", cfg.c_str()});
		quit = true;
		if (to_add != fs::directory_iterator()){
			run_command(format("cp --reflink=auto -a {} {}/", shell_quote(to_add->path().string()), shell_quote(atest_tmp.string())));
			to_add++;
			quit = false;
		}
		if (to_remove != end(rmv_list)){
			println("removing {}", (atest_tmp / *to_remove).string());
			fs::remove_all(atest_tmp / *to_remove);
			to_remove++;
			quit = false;
		}
	}
	println("extract and check");
	auto last_state = states.back();
	for (size_t i = 0; i < states.size(); i++){
		println("{}%", i * 100 /states.size());
		auto j = states.size() - 1 - i;
		extract(j, atest_arc, atest_tmp, long_test_password);
		auto fs = state_for(atest_tmp);
		compare(fs, states[i]);
		clear_previous_line();
	}
	this_thread::sleep_for(2s);
	{
		auto cfg = "cfg-file=" + test_conf("test-1s.conf").string();
		run({"archive", cfg.c_str()});
	}
	{
		Catalogue cat(atest_arc, long_test_password, false);
		ASSERT(cat.num_states() == 1);
		if (cat.num_states() != 1)
			throw runtime_error("GC test failed");
		// lock test
		bool failed = false;
		try{
			Catalogue cat1(atest_arc, long_test_password, false);
		}
		catch(...){
			failed = true;
		}
		ASSERT(failed);
		if (!failed)
			throw runtime_error("lock test failed");
	}
	extract(0, atest_arc, atest_tmp, long_test_password);
	auto fs = state_for(atest_tmp);
	compare(fs, last_state);
	cprint("{fg}All green! All shiny!{fd}\n");
	fs::remove_all(test_dir); // nothing to leave behind on success
}

void test()
{
	vector<string> failed;
	filters_test(failed);
	try{
		multi_version_test();
	}
	catch(std::exception &e){
		cprintln("  {fr}failed{fd}: multi-version. {}", message(e));
		failed.push_back("multi-version");
	}
	if (!failed.empty()){
		string all;
		for (auto &f : failed)
			all += "\n  " + f;
		throw runtime_error("Failed scenarios:" + all);
	}
	full_test();
}
