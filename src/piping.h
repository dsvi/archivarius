#pragma once
#include "precomp.h"

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
	void source(Source *next){
		next_ = next;
	}
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
	// TODO: remove
	void sink(Sink *sink){
		next_ = sink;
	}
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



class File_source : public Source{
public:
	File_source();
	File_source(const std::filesystem::path &path);
private:
	virtual
	Pump_result pump(u8 *to, u64 size) override;

	typedef std::unique_ptr<std::FILE, decltype(&fclose)> File_ptr;
	File_ptr file_;
};


class File_sink : public Sink{
public:
	File_sink();
	File_sink(const std::filesystem::path &path);

	u64 bytes_written();
	operator bool(){
		return static_cast<bool>(file_);
	}
private:
	virtual
	void pump(u8 *from, u64 size) override;
	virtual
	void finish() override;

	typedef std::unique_ptr<std::FILE, decltype(&fclose)> File_ptr;
	File_ptr file_;
	u64 bytes_written_ = 0;
};

inline
u64 File_sink::bytes_written()
{
	return bytes_written_;
}



