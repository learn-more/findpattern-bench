#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MATCHES 1024

int get_pattern_size(const char* signature) {
    // c = 2 * b + (b - 1) . 2 chars per byte + b - 1 spaces between
    return (strlen(signature) + 1) / 3;
}

unsigned char get_bits(unsigned char x) {
    // ascii numbers to byte
    if (x >= '0' && x <= '9') {
        return x - '0';
    }
    else { // ascii letters to hex byte
        // & 0xDF converts lowercase ascii to uppercase
        return ((x & 0xDF) - 'A') + 0xa;
    }
}

namespace
{
    // signature must use ? as mask for nibbles, have a space between each byte, must not prefix bytes with 0x or \x, and be ascii.
    // upper and lowercase supported.
    int find_pattern(const char* signature, const unsigned char* data, size_t data_size) {
        int matches_count = 0;
        int pattern_size = get_pattern_size(signature);
        for (int i = 0; i < data_size; i++) {
            int sig_idx = 0;
            for(;sig_idx < pattern_size && (i + sig_idx) < data_size; sig_idx++) {
                int sig_pat_idx = sig_idx * 3;
                unsigned char dat_byt = data[i + sig_idx];

                unsigned char sig_hi = signature[sig_pat_idx] == '?' ? (dat_byt & 0xF0) : get_bits(signature[sig_pat_idx]) << 4;
                unsigned char sig_lo = signature[sig_pat_idx + 1] == '?' ? (dat_byt & 0xF) :  get_bits(signature[sig_pat_idx + 1]);
                
                if (dat_byt != (sig_hi | sig_lo)) {
                    break;
                }
            }

            if (sig_idx >= pattern_size) {
                return i;
            }
        }
        return matches_count;
    }

	struct STEVEMK14EBR : public BenchBase
	{
		virtual void init(Tests test)
		{
			switch (test)
			{
			case Tests::First:
				mPattern = "45 43 45 55 33 9a fa ?? ?? ?? ?? 45 68 21";
				break;
			case Tests::Second:
				mPattern = "aa aa aa aa aa aa aa aa aa bb aa ?? ?? ?? ?? 45 68 21";
				break;
			default:
				break;
			}
		}

		virtual LPVOID runOne(PBYTE baseAddress, DWORD size)
		{
            
			return (char*)baseAddress + find_pattern(mPattern, baseAddress, size);
		}
		virtual const char* name() const
		{
			return "stevemk14ebr";
		}
		const char* mPattern = "";
	};


	REGISTER(STEVEMK14EBR);
} // namespace
