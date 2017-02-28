#ifndef BASE_64_ENCODING_H_
#define BASE_64_ENCODING_H_

#include <string>
#include <vector>

std::string base64_encode(unsigned char const*, unsigned int len);
std::vector<unsigned char> base64_decode(std::string const& s);

template<typename vType>
std::vector<vType> base64_decode_double(std::string const& s)
{
	auto dec = base64_decode(s);
	std::vector<vType> ret(dec.size() / sizeof(vType));
	memcpy(ret.data(), dec.data(), ret.size() * sizeof(vType));
	return ret;
}

#endif