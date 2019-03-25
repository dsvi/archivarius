#pragma once
#include "piping.h"
#include "buffer.h"
#include "encryption_params.h"

class Chapoly: public Encryption_params{};

/// reads all the input till eof at the first pump, and checks authenticity
/// then feeds input from internal buffer
/// so not suitable for files which can't possibly fit in memory
class Pipe_chapoly_in: public Pipe_in{
public:
	Pipe_chapoly_in(Chapoly &p);
private:
	virtual
	Pump_result pump(u8 *to, u64 size) override;
	Botan::ChaCha20Poly1305_Decryption decryptor_;
	Botan::secure_vector<uint8_t> buf_;
	size_t offset_ = 0;
};


/// accumulates all the output into internal buffer, ecrypts, signs and flushes it further only on finish()
/// so not suitable for files which can't possibly fit in memory
class Pipe_chapoly_out: public Pipe_out{
public:
	Pipe_chapoly_out(Chapoly &p);
private:
	virtual
	void pump(u8 *to, u64 size) override;
	virtual
	void finish() override;
	Botan::ChaCha20Poly1305_Encryption encryptor_;
	Botan::secure_vector<uint8_t> buf_;
};


