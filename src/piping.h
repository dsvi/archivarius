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
	virtual
	Pump_result pump(u8 *to, u64 size) = 0; // should pump exactly the size, if not eof
};

class Pipe_in: public Source{
public:
	void source(Source *next){
		next_ = next;
	}
protected:
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
	// returns new size
	virtual
	void pump(u8 *from, u64 size) = 0;
	virtual
	void finish() = 0;
};

class Pipe_out: public Sink{
public:
	void sink(Sink *sink){
		next_ = sink;
	}
	void finish() override;
protected:
	Sink *next_ = nullptr;
};



class File_source : public Source{
public:
	File_source();
	File_source(const std::filesystem::path &path);
	virtual
	Pump_result pump(u8 *to, u64 size) override;
private:
	typedef std::unique_ptr<std::FILE, decltype(&fclose)> File_ptr;
	File_ptr file_;
};


class File_sink : public Sink{
public:
	File_sink();
	File_sink(const std::filesystem::path &path);
	virtual
	void pump(u8 *to, u64 size) override;
	virtual
	void finish() override;
	u64 bytes_written();
	operator bool(){
		return static_cast<bool>(file_);
	}
private:
	typedef std::unique_ptr<std::FILE, decltype(&fclose)> File_ptr;
	File_ptr file_;
	u64 bytes_written_ = 0;
};

inline
u64 File_sink::bytes_written()
{
	return bytes_written_;
}



