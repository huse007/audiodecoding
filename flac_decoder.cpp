#include "flac_decoder.hpp"

FLACDecoder::FLACDecoder(std::ifstream& stream, std::ofstream& output)
{
	this->stream = &stream;
	this->stream->seekg(0, stream.beg);
	this->output = &output;
}

FLACDecoder::~FLACDecoder()
{
}

// Little endian
bool FLACDecoder::insertFLAC__uint32(std::ofstream& f, FLAC__uint32 x) {
	if(!f.put(x).eof() && !f.put(x >> 8).eof() && !f.put(x >> 16).eof() && !f.put(x >> 24).eof())
		return true;
	return false;
	
}
bool FLACDecoder::insert_uint16(std::ofstream& f, uint16_t x) {
	if (!f.put(x).eof() && !f.put(x >> 8).eof())
		return true;
	return false;
}
// (16 bit)
bool FLACDecoder::insert_int16(std::ofstream& f, int16_t x) {
	return insert_uint16(f, (FLAC__uint16)x);
}
bool FLACDecoder::insert_int24(std::ofstream& f, FLAC__uint32 x) {
	if (!f.put(x).eof() && !f.put(x >> 8).eof() && !f.put(x >> 16).eof())
		return true;
	return false;

}

FLAC__StreamDecoderReadStatus FLACDecoder::read_callback(FLAC__byte buffer[], size_t* bytes)
{
	if (*bytes > 0) {
		*bytes = stream->read((char*)buffer, *bytes).gcount();
		if (stream->fail()) {
			if (stream->eof()) {
				return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
			}
			else {
				return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
			}
		}
		else if (*bytes == 0) {
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		}
		else {
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		}
	}
	else {
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	}
}

FLAC__StreamDecoderWriteStatus FLACDecoder::write_callback(const FLAC__Frame* frame, const FLAC__int32* const buffer[])
{
	const FLAC__uint32 total_size = (FLAC__uint32)(total_samples * channels * (bps / 8));
	
	if (total_samples == 0) {
		std::cout << "[ERROR] total samples cannot be zero" << std::endl;
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}
	
	if (channels != 2 || bps != 24) {
		std::cout << "[ERROR] must 2 channels and 24 bit streams" << std::endl;
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	// Write WAVE header to file
	if (frame->header.number.sample_number == 0) {
		if (
			!output->write("RIFF", 4) ||
			!insertFLAC__uint32(*output, total_size + 36) ||
			output->write("WAVEfmt ", 8).fail() ||
			!insertFLAC__uint32(*output, 16) ||
			!insert_uint16(*output, 1) ||
			!insert_uint16(*output, (FLAC__uint16)channels) ||
			!insertFLAC__uint32(*output, sample_rate) ||
			!insertFLAC__uint32(*output, sample_rate * channels * (bps / 8)) ||
			!insert_uint16(*output, (FLAC__uint16)(channels*(bps / 8))) ||
			!insert_uint16(*output, (FLAC__uint16)bps) ||
			output->write("data", 4).fail() ||
			!insertFLAC__uint32(*output, total_size)) {
			std::cout << "[FAILED TO WRITE HEADER]" << std::endl;
		}
	}
	
	/* Write decoded PCM samples 24 bit to file */
	for (size_t i = 0; i < frame->header.blocksize; i++) {
		if (!insert_int24(*output, buffer[0][i]) ||
			!insert_int24(*output, buffer[1][i])) {
			std::cout << "[ERROR WRITE]" << std::endl;
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}
	}
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

FLAC__StreamDecoderTellStatus FLACDecoder::tell_callback(FLAC__uint64* absolute_byte_offset)
{
	off_t pos;
	if ((pos = stream->tellg()) < 0) {
		std::cout << "[TELL] : error\n"<<std::endl;
		return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
	}
	else {
		*absolute_byte_offset = (FLAC__uint64)pos;
		return FLAC__STREAM_DECODER_TELL_STATUS_OK;
	}
}

FLAC__StreamDecoderLengthStatus FLACDecoder::length_callback(FLAC__uint64* stream_length)
{
    return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
}

bool FLACDecoder::eof_callback()
{
	return stream->eof();
}

FLAC__StreamDecoderSeekStatus FLACDecoder::seek_callback(FLAC__uint64 absolute_byte_offset)
{
	return FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED;
}

void FLACDecoder::error_callback(FLAC__StreamDecoderErrorStatus status)
{
	std::cout << "[ERROR] " << FLAC__StreamDecoderErrorStatusString[status] << std::endl;
}

void FLACDecoder::metadata_callback(const ::FLAC__StreamMetadata* metadata)
{
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		
		sample_rate = metadata->data.stream_info.sample_rate;
		total_samples = metadata->data.stream_info.total_samples;
		channels = metadata->data.stream_info.channels;
		bps = metadata->data.stream_info.bits_per_sample;
		
		fprintf(stderr, "[INFO] sample rate    : %u Hz\n", sample_rate);
		fprintf(stderr, "[INFO] channels       : %u\n", channels);
		fprintf(stderr, "[INFO] bits per sample: %u\n", bps);
		fprintf(stderr, "[INFO] total samples  : %" PRIu64 "\n\n", total_samples);
	}
}
