#include <fstream>
#include <iostream>

#include "flac_decoder.hpp"

int main(int argc, char* argv[])
{
	const char* outfile = "output.wav";

	if (argc < 2) {
        std::cout << "Usage: ./out <flac_file_path>" << std::endl;
        return 1;
    }

    std::cout << "Using input file: " << argv[1] << std::endl;
	std::cout << "Using output file: " <<outfile  << "\n"<<std::endl;
	
	std::ifstream file(argv[1], std::ios::binary);
	std::ofstream ofile(outfile, std::ios::binary);
	
	FLACDecoder decoder(file,ofile);
	
	std::cout << "\n[INIT] "<<FLAC__StreamDecoderInitStatusString[decoder.init()] << std::endl;

	decoder.process_until_end_of_stream() ?	std::cout << "[DONE] " : std::cout << "[ERROR] "; 
	std::cout<< decoder.get_state().as_cstring() << std::endl;
	decoder.finish();

	file.close();
	ofile.close();
	return 0;
}
