#pragma once

#include <FLAC++/decoder.h>
#include <iostream>
#include <fstream>
#include "stdio.h"
#include "inttypes.h"
//#include <bitset>

static FLAC__uint64 total_samples = 0;
static uint32_t sample_rate = 0;
static uint32_t channels = 0;
static uint32_t bps = 0;

class FLACDecoder : public FLAC::Decoder::Stream
{
  public:
	FLACDecoder(std::ifstream& stream,std::ofstream& output);
    ~FLACDecoder();

  private:
	  FILE * file;
	  std::ifstream *stream;
	  std::ofstream *ostream;
	  std::ofstream *output;
	  
    //
    // implement these callbacks:
    //
    virtual ::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], size_t* bytes) override;
    virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame*     frame,
                                                            const FLAC__int32* const buffer[]) override;
    virtual void error_callback(::FLAC__StreamDecoderErrorStatus status) override;
    virtual ::FLAC__StreamDecoderSeekStatus seek_callback(FLAC__uint64 absolute_byte_offset) override;
    virtual ::FLAC__StreamDecoderTellStatus tell_callback(FLAC__uint64* absolute_byte_offset) override;
    virtual ::FLAC__StreamDecoderLengthStatus length_callback(FLAC__uint64* stream_length) override;
    virtual void metadata_callback(const ::FLAC__StreamMetadata* metadata) override;
    virtual bool eof_callback() override;

	bool insertFLAC__uint32(std::ofstream& f, FLAC__uint32);
	bool insert_uint16(std::ofstream& f, uint16_t x);
	bool insert_int16(std::ofstream& f, int16_t x);
	bool insert_int24(std::ofstream& f, FLAC__uint32 x);
};
