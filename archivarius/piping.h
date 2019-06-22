#pragma once
#include "precomp.h"

namespace archi{


class Source{
public:
	Source(Source&) = delete;
	Source(Source&&) = default;
	Source(){};

	Source & operator =(Source&&) = default;

	virtual
	~Source();

	struct Pump_result{
		u64 pumped_size;
		bool eof;
	};
private:
	virtual
	Pump_result pump(u8 *to, u64 size) = 0; // should pump exactly the size, if not eof

	friend class Pipe_in;
	friend class Stream_in;
};

class Pipe_in: public Source{
public:
	Pipe_in &operator <<(Pipe_in &s){
		next_ = &s;
		return s;
	}
	void operator <<(Source &s){
		next_ = &s;
	}
protected:
	Pump_result pump_next(u8 *to, u64 size);
private:
	Source *next_;
};

class Sink{
public:
	Sink(Sink&) = delete;
	Sink(Sink&&) = default;
	Sink(){}

	Sink & operator =(Sink&&) = default;

	virtual
	~Sink();
private:
	virtual
	void pump(u8 *from, u64 size) = 0;
	virtual
	void finish() = 0;

	friend class Pipe_out;
	friend class Stream_out;
};

class Pipe_out: public Sink{
public:
	Pipe_out &operator >>(Pipe_out &s){
		next_ = &s;
		return s;
	}
	void operator >>(Sink &s){
		next_ = &s;
	}
protected:
	void pump_next(u8 *from, u64 size);
	void finish_next();
private:
	Sink *next_ = nullptr;
	virtual
	void finish() override;
};

void checked_fclose(FILE *f);
typedef std::unique_ptr<std::FILE, decltype(&checked_fclose)> File_ptr;

class File_source : public Source{
public:
	File_source();
	File_source(const std::filesystem::path &path);
private:
	virtual
	Pump_result pump(u8 *to, u64 size) override;

	File_ptr file_;
};


class File_sink : public Sink{
public:
	File_sink();
	File_sink(const std::filesystem::path &path);

	u64 bytes_written();
	/// true if sink is associated with an open file and rdy to accept data
	operator bool(){
		return static_cast<bool>(file_);
	}
	/// closes the file if open, and resetes bytes_written(). basically object returns into default constructed state
	/// exception safe. throws only if no current exception is pending
	void reset();
private:
	virtual
	void pump(u8 *from, u64 size) override;
	virtual
	void finish() override;

	File_ptr file_;
	u64 bytes_written_ = 0;
};

static_assert (std::is_nothrow_move_constructible<File_sink>::value);

inline
u64 File_sink::bytes_written()
{
	return bytes_written_;
}


}
